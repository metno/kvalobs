#pragma once


#include <string>
#include <vector>
#include <chrono>
#include <mutex>
#include <stdexcept>

// ---------------------------------------------------------------------------
// Message row returned from a poll
// ---------------------------------------------------------------------------
struct Message {
    long long   id;
    std::string topic;
    std::string data;
    std::string created_at;
};

// ---------------------------------------------------------------------------
// NodeInfo — result of probing one connection string
// ---------------------------------------------------------------------------
struct NodeInfo {
    std::string conninfo;
    bool        is_primary = false;   // true  → primary (pg_is_in_recovery = f)
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
    typedef enum environment { production, staging, development } Environment;

    explicit PgCluster(std::vector<std::string> conninfos,
                       Environment env = development,
                       const std::string &appName="",
                       Seconds cache_ttl = Seconds{60});


    explicit PgCluster(std::vector<std::string> conninfos,
                       const std::string  env = "development",
                       const std::string &appName="",
                       Seconds cache_ttl = Seconds{60});
                   
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
    static  
    Environment env(const std::string& s);

    const std::string& env(Environment e) const;
    
private:
    std::vector<std::string>                       conninfos_;
    Seconds                                        cache_ttl_;
    std::vector<NodeInfo>                          nodes_;         // cached result
    std::chrono::steady_clock::time_point          probed_at_;
    size_t                                         replica_rr_{0}; // round-robin index
    Environment                                    env_;
    
    mutable std::mutex                             mu_;

    void probe_locked();   // must be called with mu_ held
    bool cache_valid() const;
    // Returns the messages table for the environment (kvproduction, kvstaging, kvdev)
    std::string msgTable() const; 

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
    // Takes a shared PgCluster reference — the cluster must outlive this object.
    explicit PgMessaging(PgCluster& cluster);
    ~PgMessaging();

    PgMessaging(const PgMessaging&)            = delete;
    PgMessaging& operator=(const PgMessaging&) = delete;

    // --- Topics (write → primary) -------------------------------------------
    void create_topic(const std::string& topic);
    std::vector<std::string> list_topics() const;
    bool topic_exists(const std::string& topic) const;

    // --- Producer (write → primary) -----------------------------------------
    long long publish(const std::string& topic, const std::string& data);
    long long publish_dedup(const std::string& topic, const std::string& data);

    // --- Consumer writes (write → primary) ----------------------------------
    void register_consumer(const std::string& consumer_name,
                           const std::string& topic);
    void commit_offset(const std::string& consumer_name,
                       const std::string& topic,
                       long long          last_id);

    // --- Consumer reads (read → replica) ------------------------------------
    long long            get_offset(const std::string& consumer_name,
                                    const std::string& topic);
    std::vector<Message> poll(const std::string& consumer_name,
                              const std::string& topic,
                              int                limit = 1000);
    long long            consumer_lag(const std::string& consumer_name,
                                      const std::string& topic);

private:
    void *primaryCon_=nullptr;    // write connection
    void *replicaCon_=nullptr;   // read connection
    std::string msgTbl_;
    std::string appName_;

    void           prepare_primary_stmts();
    void           prepare_replica_stmts();
};
