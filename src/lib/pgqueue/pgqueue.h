#pragma once

#include <chrono>
#include <cstdint>
#include <mutex>
#include <stdexcept>
#include <string>
#include <vector>

namespace pgqueue {

const std::string CONTENT_TYPE_BINARY = "application/octet-stream";
const std::string CONTENT_TYPE_JSON = "application/json";
const std::string CONTENT_TYPE_TEXT = "text/plain; charset=utf-8";
const std::string CONTENT_TYPE_XML = "application/xml";
const std::string CONTENT_TYPE_BUFR = "application/bufr";

class PGException : public std::runtime_error {
public:
  explicit PGException(const std::string &msg) : std::runtime_error(msg) {}
};

class PGConnectionException : public PGException {
public:
  explicit PGConnectionException(const std::string &msg) : PGException(msg) {}
};

class PGQueryException : public PGException {
protected:
  std::string sqlstate_;

public:
  explicit PGQueryException(const std::string &msg, const char *sqlstate)
      : PGException(msg), sqlstate_(sqlstate) {}
  explicit PGQueryException(const std::string &msg, const std::string &sqlstate)
      : PGException(msg), sqlstate_(sqlstate) {}

  std::string sqlstate() const { return sqlstate_; }
};

class PGConfigException : public PGException {
public:
  explicit PGConfigException(const std::string &msg) : PGException(msg) {}
};

// ---------------------------------------------------------------------------
// Config — settings read from a configuration file
//
// File format (a small YAML subset: `key: value` lines and one list):
//
//   topic: kvalobs.staging.raw
//   max_pool_size: 10
//   pool_interval_ms: 500
//   connections:
//     - user=kvproc dbname=pgqueue host=db1 port=5432
//     - user=kvproc dbname=pgqueue host=db2 port=5432
//
// Empty lines and text after '#' are ignored. Values may be quoted with
// single or double quotes.
// ---------------------------------------------------------------------------
struct Config {
  std::string topic;
  int max_pool_size = 10;
  int pool_interval_ms = 500;
  std::vector<std::string> connections;
};

// Read and validate a configuration file.
// Throws PGConfigException if the file cannot be read, has a syntax error, an
// unknown key, an invalid number, or no connections.
Config read_config(const std::string &path);

// ---------------------------------------------------------------------------
// Message row returned from a poll
// ---------------------------------------------------------------------------
struct Message {
  std::uint64_t id;
  std::string topic;
  std::string data;
  std::string content_type;
  std::chrono::sys_time<std::chrono::microseconds> created_at;

  friend std::ostream &operator<<(std::ostream &os, const Message &m);
};

std::ostream &operator<<(std::ostream &os, const Message &m);

// ---------------------------------------------------------------------------
// NodeInfo — result of probing one connection string
// ---------------------------------------------------------------------------
struct NodeInfo {
  std::string conninfo;
  bool is_primary = false; // true  → primary (pg_is_in_recovery = f)
                           // false → replica (pg_is_in_recovery = t)
};

// ---------------------------------------------------------------------------
// PgCluster
//
// Probes a list of connection strings to identify which node is the primary
// and which are replicas.  Results are cached and refreshed automatically
// after `cache_ttl` (default 60 s), or on demand via reprobe().
//
// Thread-safe: all public methods are guarded by a mutex so a single
// PgCluster instance can be shared across threads.
//
// Usage:
//   PgCluster cluster({"host=primary ...", "host=replica ..."});
//   std::string primary_ci = cluster.primary_conninfo();
//   std::string replica_ci = cluster.replica_conninfo();
// ---------------------------------------------------------------------------

class PgCluster {
public:
  friend class PgMessaging;
  using Seconds = std::chrono::seconds;

  explicit PgCluster(std::vector<std::string> conninfos,
                     bool only_primary = false,
                     const std::string &appName = "");

  // Returns conninfo for the current primary.
  // Throws if no primary is found.
  // PGConnectionException is thrown if no primary is found.
  std::string primary_conninfo();

  // Returns conninfo for a replica (round-robins if multiple).
  // Falls back to primary if no replica is found (e.g. single-node setup).
  // PGConnectionException is thrown if no usable postgres server is found.
  std::string replica_conninfo();

  // Force an immediate re-probe (e.g. after a failover is detected).
  void reprobe();

  // Returns a snapshot of what the last probe found.
  std::vector<NodeInfo> topology();

  bool connected() const;

  // Returns the error and warning messages from the last probe.
  std::string errors() const;
  bool hasErrors() const;

  bool only_primary() const { return only_primary_; }
  std::string app_name() const { return appName_; }

private:
  std::vector<std::string> conninfos_;
  std::vector<NodeInfo> nodes_; // cached result
  std::string appName_;
  mutable std::mutex mu_;
  std::string error_;
  bool only_primary_;

  void probe_locked(); // must be called with mu_ held
};

// ---------------------------------------------------------------------------
// PgMessaging
//
// Wraps two libpq connections obtained from a PgCluster:
//   - primary_  : used for all writes (publish, create_topic,
//                 register_consumer, commit_offset)
//   - replica_  : used for all reads  (poll, get_offset, consumer_lag)
//
// Prepared statements are registered separately on each connection.
// Not thread-safe — use one instance per thread.
//
// If the replica connection is the same as the primary (single-node or
// all replicas unavailable), both pointers point to different PGconn
// handles that happen to connect to the same server.  This is intentional
// and harmless.
// ---------------------------------------------------------------------------
class PgMessaging {
public:
  typedef enum class ConsumeFromMode {
    CONSUME_FROM_BEGINNING,
    CONSUME_FROM_END
  } ConsumeFromMode;
  // Takes a shared PgCluster reference — the cluster must outlive this object.
  // Throws PGConnectionException if connection to the cluster fails.
  // Throws PGQueryException if a query fails during initialization.
  explicit PgMessaging(PgCluster &cluster);
  ~PgMessaging();

  PgMessaging(const PgMessaging &) = delete;
  PgMessaging &operator=(const PgMessaging &) = delete;

  // --- Topics (write → primary) -------------------------------------------
  // Topics represent logical channels for messages.
  // Create a new topic. One need kv_write privileges to create a topic.
  //
  // Throws PGQueryException if the query fails.
  // Throws PGConnectionException if connection to the replica fails.
  void create_topic(const std::string &topic);

  // Throws PGQueryException if the query fails.
  // Throws PGConnectionException if connection to the replica fails.
  std::vector<std::string> list_topics() const;

  // Check if a topic exists. One need kv_read privileges to check a topic.
  // Throws PGQueryException if the query fails.
  // Throws PGConnectionException if connection to the replica fails.
  bool topic_exists(const std::string &topic) const;

  // Publish a message to a topic. One need kv_write privileges to publish a
  // message. The content_type parameter specifies the MIME type of the message
  // data. The topic must exist or an error will be thrown.
  // Throws PGQueryException if the query fails.
  // Throws PGConnectionException if connection to the primary fails.
  std::uint64_t
  publish(const std::string &topic, const std::string &data,
          const std::string &content_type /*= CONTENT_TYPE_BINARY*/);

  // Publish a message to a topic with deduplication, ie avoid duplicates.
  // The duplicates is determined based on a crc value computed from the message
  // content. In other words, if a message with the same crc value already
  // exists, it will not be published again. The id of the existing message will
  // be returned. The topic must exist or an error will be thrown.
  //
  // One need kv_write privileges to publish a message.
  // The content_type parameter specifies the MIME type of the message data.
  // Throws PGQueryException if the query fails.
  // Throws PGConnectionException if connection to the primary fails.
  std::uint64_t
  publish_dedup(const std::string &topic, const std::string &data,
                const std::string &content_type /*= CONTENT_TYPE_BINARY*/);

  // One need kv_read privileges to register a consumer and commit offsets.
  // The topic must exist or an error will be thrown.
  // Throws PGQueryException if the query fails.
  // Throws PGConnectionException if connection to the primary fails.
  std::uint64_t register_consumer(const std::string &consumer_name,
                                  const std::string &topic);

  // Commit the last processed message ID for a consumer on a topic.
  // One need kv_read privileges to commit offsets.
  // The topic must exist or an error will be thrown. In addition
  // the consumer must be registered for the topic.
  // Throws PGQueryException if the query fails.
  // Throws PGConnectionException if connection to the primary fails.
  void commit_offset(const std::string &consumer_name, const std::string &topic,
                     std::uint64_t last_id);

  // Set the consumer offset to the beginning or end of the topic.
  // One need kv_read privileges to set the consumer offset.
  // Throws PGQueryException if the query fails.
  // Throws PGConnectionException if connection to the primary fails.
  void set_consumer_offset(const std::string &consumer_name,
                           const std::string &topic, ConsumeFromMode mode);

  // Get the current offset for a consumer on a topic.
  // One need kv_read privileges to get the offset.
  // Throws PGQueryException if the query fails.
  // Throws PGConnectionException if connection to the replica fails.
  std::uint64_t get_offset(const std::string &consumer_name,
                           const std::string &topic);

  // Poll messages for a consumer on a topic.
  // One need kv_read privileges to poll messages.
  // The topic must exist or an error will be thrown. In addition
  // the consumer must be registered for the topic.
  // Throws PGQueryException if the query fails.
  // Throws PGConnectionException if connection to the replica fails.
  std::vector<Message> poll(const std::string &consumer_name,
                            const std::string &topic, int limit = 1000);

  // Get the lag (difference between the latest message ID and the consumer's
  // offset) for a consumer on a topic. Used for monitoring the consumer's
  // progress relative to the latest message. One need kv_read privileges to get
  // the consumer lag.
  // The topic must exist or an error will be thrown. In addition
  // the consumer must be registered for the topic.
  // Throws PGQueryException if the query fails.
  // Throws PGConnectionException if connection to the replica fails.
  std::uint64_t consumer_lag(const std::string &consumer_name,
                             const std::string &topic);

private:
  void *primaryCon_ = nullptr; // write connection
  void *replicaCon_ = nullptr; // read connection
  std::string appName_;
  bool onlyPrimary_ = false;
  void prepare_primary_stmts(void *conn);
  void prepare_replica_stmts(void *conn);
};

} // namespace pgqueue