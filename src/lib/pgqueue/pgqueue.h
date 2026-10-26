#pragma once

#include <chrono>
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
// Message row returned from a poll
// ---------------------------------------------------------------------------
struct Message {
  std::int64_t id;
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
                     const std::string &appName = "");

  // Returns conninfo for the current primary.
  // Throws if no primary is found.
  std::string primary_conninfo();

  // Returns conninfo for a replica (round-robins if multiple).
  // Falls back to primary if no replica is found (e.g. single-node setup).
  std::string replica_conninfo();

  // Force an immediate re-probe (e.g. after a failover is detected).
  void reprobe();

  // Returns a snapshot of what the last probe found.
  std::vector<NodeInfo> topology();

  // Returns the environment enum for a given string (production, staging, dev)
  // Throws std::invalid_argument if the string is invalid.

  bool connected() const;

  std::string errors() const;
  bool hasErrors() const;

private:
  std::vector<std::string> conninfos_;
  std::vector<NodeInfo> nodes_; // cached result
  std::string appName_;
  mutable std::mutex mu_;
  std::string error_;

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
  typedef enum { CONSUME_FROM_BEGINNING, CONSUME_FROM_END } ConsumeFromMode;
  // Takes a shared PgCluster reference — the cluster must outlive this object.
  explicit PgMessaging(PgCluster &cluster);
  ~PgMessaging();

  PgMessaging(const PgMessaging &) = delete;
  PgMessaging &operator=(const PgMessaging &) = delete;

  // --- Topics (write → primary) -------------------------------------------
  // Topics represent logical channels for messages.
  // Create a new topic. One need kv_write privileges to create a topic.
  void create_topic(const std::string &topic);
  std::vector<std::string> list_topics() const;

  // Check if a topic exists. One need kv_read privileges to check a topic.
  bool topic_exists(const std::string &topic) const;

  // --- Producer (write → primary) -----------------------------------------
  // Publish a message to a topic. One need kv_write privileges to publish a
  // message. The content_type parameter specifies the MIME type of the message
  // data.
  long long publish(const std::string &topic, const std::string &data,
                    const std::string &content_type = CONTENT_TYPE_BINARY);
  long long
  publish_dedup(const std::string &topic, const std::string &data,
                const std::string &content_type = CONTENT_TYPE_BINARY);

  // --- Consumer writes (write → primary) ----------------------------------
  // One need kv_read privileges to register a consumer and commit offsets.
  long long register_consumer(const std::string &consumer_name,
                              const std::string &topic);

  // Commit the last processed message ID for a consumer on a topic.
  // One need kv_write privileges to commit offsets.
  void commit_offset(const std::string &consumer_name, const std::string &topic,
                     long long last_id);

  // Set the consumer offset to the beginning or end of the topic.
  // One need kv_read privileges to set the consumer offset.
  void set_consumer_offset(const std::string &consumer_name,
                           const std::string &topic, ConsumeFromMode mode);

  // --- Consumer reads (read → replica) ------------------------------------
  // Get the current offset for a consumer on a topic.
  // One need kv_read privileges to get the offset.
  long long get_offset(const std::string &consumer_name,
                       const std::string &topic);

  // Poll messages for a consumer on a topic.
  // One need kv_read privileges to poll messages.
  std::vector<Message> poll(const std::string &consumer_name,
                            const std::string &topic, int limit = 1000);

  // Get the lag (difference between the latest message ID and the consumer's
  // offset) for a consumer on a topic. Used for monitoring the consumer's
  // progress relative to the latest message. One need kv_read privileges to get
  // the consumer lag.
  std::int64_t consumer_lag(const std::string &consumer_name,
                            const std::string &topic);

private:
  void *primaryCon_ = nullptr; // write connection
  void *replicaCon_ = nullptr; // read connection
  std::string appName_;

  void prepare_primary_stmts();
  void prepare_replica_stmts();
};

} // namespace pgqueue