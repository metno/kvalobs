#include "pgqueue.h"
#include <algorithm>
#include <stdexcept>
#include <string>

// ---------------------------------------------------------------------------
// RAII result guard
// ---------------------------------------------------------------------------
struct ResultGuard {
  PGresult *res;
  explicit ResultGuard(PGresult *r) : res(r) {}
  ~ResultGuard() {
    if (res)
      PQclear(res);
  }
  ResultGuard(const ResultGuard &) = delete;
  ResultGuard &operator=(const ResultGuard &) = delete;
};

// ===========================================================================
// PgCluster
// ===========================================================================

PgCluster::PgCluster(std::vector<std::string> conninfos, Seconds cache_ttl)
    : conninfos_(std::move(conninfos)), cache_ttl_(cache_ttl) {
  if (conninfos_.empty())
    throw std::runtime_error("PgCluster: no connection strings provided");

  std::lock_guard<std::mutex> lk(mu_);
  probe_locked();
}

bool PgCluster::cache_valid() const {
  // mu_ must be held by caller
  auto age = std::chrono::steady_clock::now() - probed_at_;
  return !nodes_.empty() && age < cache_ttl_;
}

void PgCluster::probe_locked() {
  // mu_ must be held by caller
  nodes_.clear();

  for (const auto &ci : conninfos_) {
    PGconn *conn = PQconnectdb(ci.c_str());
    if (!conn || PQstatus(conn) != CONNECTION_OK) {
      // Node unreachable — skip, don't throw.
      if (conn)
        PQfinish(conn);
      continue;
    }

    // pg_is_in_recovery() → 't' on replica, 'f' on primary.
    PGresult *res = PQexec(conn, "SELECT pg_is_in_recovery()");
    bool ok = PQresultStatus(res) == PGRES_TUPLES_OK && PQntuples(res) == 1;
    bool in_recovery = ok && std::string(PQgetvalue(res, 0, 0)) == "t";
    PQclear(res);
    PQfinish(conn);

    if (ok) {
      NodeInfo n;
      n.conninfo = ci;
      n.is_primary = !in_recovery;
      nodes_.push_back(std::move(n));
    }
  }

  probed_at_ = std::chrono::steady_clock::now();
}

void PgCluster::reprobe() {
  std::lock_guard<std::mutex> lk(mu_);
  probe_locked();
}

std::vector<NodeInfo> PgCluster::topology() {
  std::lock_guard<std::mutex> lk(mu_);
  if (!cache_valid())
    probe_locked();
  return nodes_;
}

std::string PgCluster::primary_conninfo() {
  std::lock_guard<std::mutex> lk(mu_);
  if (!cache_valid())
    probe_locked();

  for (const auto &n : nodes_)
    if (n.is_primary)
      return n.conninfo;

  throw std::runtime_error("PgCluster: no primary found among probed nodes");
}

std::string PgCluster::replica_conninfo() {
  std::lock_guard<std::mutex> lk(mu_);
  if (!cache_valid())
    probe_locked();

  // Collect replicas.
  std::vector<const NodeInfo *> replicas;
  for (const auto &n : nodes_)
    if (!n.is_primary)
      replicas.push_back(&n);

  if (!replicas.empty()) {
    // Round-robin across available replicas.
    size_t idx = replica_rr_++ % replicas.size();
    return replicas[idx]->conninfo;
  }

  // No replica found — fall back to primary so callers still work on
  // single-node setups.
  for (const auto &n : nodes_)
    if (n.is_primary)
      return n.conninfo;

  throw std::runtime_error("PgCluster: no usable node found");
}

// ===========================================================================
// PgMessaging — static helpers
// ===========================================================================

PGconn *PgMessaging::connect(const std::string &conninfo) {
  PGconn *conn = PQconnectdb(conninfo.c_str());
  if (!conn || PQstatus(conn) != CONNECTION_OK) {
    std::string err = conn ? PQerrorMessage(conn) : "out of memory";
    if (conn)
      PQfinish(conn);
    throw std::runtime_error("PgMessaging: connect failed: " + err);
  }
  return conn;
}

void PgMessaging::prepare(PGconn *conn, const char *name, const char *sql) {
  ResultGuard g(PQprepare(conn, name, sql, 0, nullptr));
  if (PQresultStatus(g.res) != PGRES_COMMAND_OK) {
    throw std::runtime_error(std::string("PgMessaging: prepare '") + name +
                             "' failed: " + PQresultErrorMessage(g.res));
  }
}

void PgMessaging::check_result(PGresult *res, ExecStatusType expected,
                               const std::string &ctx) {
  if (PQresultStatus(res) != expected) {
    std::string err = PQresultErrorMessage(res);
    PQclear(res);
    throw std::runtime_error(ctx + ": " + err);
  }
}

void PgMessaging::check_command(PGresult *res, const std::string &ctx) {
  check_result(res, PGRES_COMMAND_OK, ctx);
}

// ===========================================================================
// PgMessaging — construction / prepared statements
// ===========================================================================

PgMessaging::PgMessaging(PgCluster &cluster) {
  primary_ = connect(cluster.primary_conninfo());
  replica_ = connect(cluster.replica_conninfo());

  prepare_primary_stmts();
  prepare_replica_stmts();
}

PgMessaging::~PgMessaging() {
  if (primary_) {
    PQfinish(primary_);
    primary_ = nullptr;
  }
  if (replica_) {
    PQfinish(replica_);
    replica_ = nullptr;
  }
}

void PgMessaging::prepare_primary_stmts() {
  // Statements that write — executed on primary_.
  prepare(primary_, "create_topic",
          "INSERT INTO topic (topic) VALUES ($1) ON CONFLICT DO NOTHING");

  prepare(primary_, "publish",
          "INSERT INTO messages (topic, data) VALUES ($1, $2) RETURNING id");

  prepare(primary_, "publish_dedup", R"(
        WITH existing AS (
            SELECT id FROM messages
            WHERE topic = $1 AND crc = md5($2)::uuid
            LIMIT 1
        ),
        inserted AS (
            INSERT INTO messages (topic, data)
            SELECT $1, $2
            WHERE NOT EXISTS (SELECT 1 FROM existing)
            RETURNING id
        )
        SELECT id FROM inserted
        UNION ALL
        SELECT id FROM existing
        LIMIT 1
    )");

  prepare(primary_, "register_consumer", R"(
        INSERT INTO consumer_offsets (consumer_name, topic, last_id)
        VALUES ($1, $2, 0)
        ON CONFLICT (consumer_name, topic) DO NOTHING
    )");

  prepare(primary_, "commit_offset", R"(
        UPDATE consumer_offsets
        SET last_id = $3, updated_at = now()
        WHERE consumer_name = $1 AND topic = $2
    )");
}

void PgMessaging::prepare_replica_stmts() {
  // Statements that only read — executed on replica_.
  prepare(replica_, "get_offset", R"(
        SELECT last_id FROM consumer_offsets
        WHERE consumer_name = $1 AND topic = $2
    )");

  prepare(replica_, "poll", R"(
        SELECT id, topic, data, created_at
        FROM messages
        WHERE topic = $1
          AND id > $2
        ORDER BY id
        LIMIT $3
    )");

  prepare(replica_, "consumer_lag", R"(
        SELECT COALESCE(MAX(m.id), 0) - co.last_id AS lag
        FROM consumer_offsets co
        LEFT JOIN messages m ON m.topic = co.topic
        WHERE co.consumer_name = $1 AND co.topic = $2
        GROUP BY co.last_id
    )");
}

// ===========================================================================
// PgMessaging — Topics
// ===========================================================================

void PgMessaging::create_topic(const std::string &topic) {
  const char *p[] = {topic.c_str()};
  ResultGuard g(
      PQexecPrepared(primary_, "create_topic", 1, p, nullptr, nullptr, 0));
  check_command(g.res, "create_topic");
}

// ===========================================================================
// PgMessaging — Producer (primary)
// ===========================================================================

long long PgMessaging::publish(const std::string &topic,
                               const std::string &data) {
  const char *p[] = {topic.c_str(), data.c_str()};
  ResultGuard g(PQexecPrepared(primary_, "publish", 2, p, nullptr, nullptr, 0));
  check_result(g.res, PGRES_TUPLES_OK, "publish");
  if (PQntuples(g.res) == 0)
    throw std::runtime_error("publish: no id returned");
  return std::stoll(PQgetvalue(g.res, 0, 0));
}

long long PgMessaging::publish_dedup(const std::string &topic,
                                     const std::string &data) {
  const char *p[] = {topic.c_str(), data.c_str()};
  ResultGuard g(
      PQexecPrepared(primary_, "publish_dedup", 2, p, nullptr, nullptr, 0));
  check_result(g.res, PGRES_TUPLES_OK, "publish_dedup");
  if (PQntuples(g.res) == 0)
    throw std::runtime_error("publish_dedup: no id returned");
  return std::stoll(PQgetvalue(g.res, 0, 0));
}

// ===========================================================================
// PgMessaging — Consumer writes (primary)
// ===========================================================================

void PgMessaging::register_consumer(const std::string &consumer_name,
                                    const std::string &topic) {
  const char *p[] = {consumer_name.c_str(), topic.c_str()};
  ResultGuard g(
      PQexecPrepared(primary_, "register_consumer", 2, p, nullptr, nullptr, 0));
  check_command(g.res, "register_consumer");
}

void PgMessaging::commit_offset(const std::string &consumer_name,
                                const std::string &topic, long long last_id) {
  std::string id_str = std::to_string(last_id);
  const char *p[] = {consumer_name.c_str(), topic.c_str(), id_str.c_str()};
  ResultGuard g(
      PQexecPrepared(primary_, "commit_offset", 3, p, nullptr, nullptr, 0));
  check_command(g.res, "commit_offset");
}

// ===========================================================================
// PgMessaging — Consumer reads (replica)
// ===========================================================================

long long PgMessaging::get_offset(const std::string &consumer_name,
                                  const std::string &topic) {
  const char *p[] = {consumer_name.c_str(), topic.c_str()};
  ResultGuard g(
      PQexecPrepared(replica_, "get_offset", 2, p, nullptr, nullptr, 0));
  check_result(g.res, PGRES_TUPLES_OK, "get_offset");
  if (PQntuples(g.res) == 0)
    return 0;
  return std::stoll(PQgetvalue(g.res, 0, 0));
}

std::vector<Message> PgMessaging::poll(const std::string &consumer_name,
                                       const std::string &topic, int limit) {
  long long offset = get_offset(consumer_name, topic);
  std::string offset_str = std::to_string(offset);
  std::string limit_str = std::to_string(limit);

  const char *p[] = {topic.c_str(), offset_str.c_str(), limit_str.c_str()};
  ResultGuard g(PQexecPrepared(replica_, "poll", 3, p, nullptr, nullptr, 0));
  check_result(g.res, PGRES_TUPLES_OK, "poll");

  std::vector<Message> rows;
  int n = PQntuples(g.res);
  rows.reserve(n);
  for (int i = 0; i < n; ++i) {
    Message m;
    m.id = std::stoll(PQgetvalue(g.res, i, 0));
    m.topic = PQgetvalue(g.res, i, 1);
    m.data = PQgetvalue(g.res, i, 2);
    m.created_at = PQgetvalue(g.res, i, 3);
    rows.push_back(std::move(m));
  }
  return rows;
}

long long PgMessaging::consumer_lag(const std::string &consumer_name,
                                    const std::string &topic) {
  const char *p[] = {consumer_name.c_str(), topic.c_str()};
  ResultGuard g(
      PQexecPrepared(replica_, "consumer_lag", 2, p, nullptr, nullptr, 0));
  check_result(g.res, PGRES_TUPLES_OK, "consumer_lag");
  if (PQntuples(g.res) == 0)
    return 0;
  return std::stoll(PQgetvalue(g.res, 0, 0));
}
