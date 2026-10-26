#include "pgqueue.h"
#include <algorithm>
#include <format>
#include <iostream>
#include <libpq-fe.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>

namespace pgqueue {
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

PgCluster::PgCluster(std::vector<std::string> conninfos,
                     const std::string &appName)
    : conninfos_(std::move(conninfos)), appName_(appName) {
  if (conninfos_.empty())
    throw std::runtime_error("PgCluster: no connection strings provided");

  if (!appName.empty()) {
    // check if the conninfos strings contain the application_name parapeter. If
    // not add it.
    for (auto &con : conninfos_) {
      if (con.find("application_name") == std::string::npos) {
        con.append(std::format(R"( application_name={})", appName));
      }
    }
  }
  std::cerr << "Connection info: " << '\n';
  for (const auto &con : conninfos_) {
    std::cerr << "  - " << con << '\n';
  }
  std::lock_guard<std::mutex> lk(mu_);
  probe_locked();
}

void PgCluster::probe_locked() {
  // mu_ must be held by caller
  nodes_.clear();
  error_.clear();
  int errorCnt = 0;
  std::stringstream ss;

  for (const auto &ci : conninfos_) {
    std::cerr << "Attempting to connect to node: " << ci << '\n';
    PGconn *conn = PQconnectdb(ci.c_str());
    std::cerr << "Connecting to node: " << ci << '\n';
    if (!conn || PQstatus(conn) != CONNECTION_OK) {
      if (!conn) {
        ss << "Failed to connect to node: '" << ci << "'\n";
        errorCnt++;
        continue;
      }
      // Node unreachable — skip, don't throw.
      ss << "Failed to connect to node: " << PQerrorMessage(conn) << '\n';
      errorCnt++;
      PQfinish(conn);
      continue;
    }
    std::cerr << "Successfully connected to node: " << ci << '\n';
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

  if (errorCnt > 0) {
    error_ = ss.str();
  }
}

bool PgCluster::connected() const {
  std::lock_guard<std::mutex> lk(mu_);
  return !nodes_.empty();
}

void PgCluster::reprobe() {
  std::lock_guard<std::mutex> lk(mu_);
  probe_locked();
}

std::vector<NodeInfo> PgCluster::topology() {
  std::lock_guard<std::mutex> lk(mu_);
  if (nodes_.empty())
    probe_locked();
  return nodes_;
}

std::string PgCluster::primary_conninfo() {
  std::lock_guard<std::mutex> lk(mu_);
  if (nodes_.empty()) {
    probe_locked();
  }

  for (const auto &n : nodes_) {
    if (n.is_primary) {
      return n.conninfo;
    }
  }

  throw PGConnectionException("PgCluster: no primary found among probed nodes");
}

std::string PgCluster::replica_conninfo() {
  std::lock_guard<std::mutex> lk(mu_);
  if (nodes_.empty()) {
    probe_locked();
  }

  // Collect replicas.
  std::vector<const NodeInfo *> replicas;
  for (const auto &n : nodes_) {
    if (!n.is_primary) {
      replicas.push_back(&n);
    }
  }

  if (!replicas.empty()) {
    // More than one replica, select the first
    return replicas[0]->conninfo;
  }

  // No replica found — fall back to primary so callers still work on
  // single-node setups.
  for (const auto &n : nodes_) {
    if (n.is_primary) {
      return n.conninfo;
    }
  }

  throw PGConnectionException("PgCluster: no usable postgres server found");
}

std::string PgCluster::errors() const {
  std::lock_guard<std::mutex> lk(mu_);
  return error_;
}

bool PgCluster::hasErrors() const {
  std::lock_guard<std::mutex> lk(mu_);
  return !error_.empty();
}

// ===========================================================================
// PgMessaging — helpers
// ===========================================================================

namespace {
PGconn *connect(const std::string &conninfo) {
  PGconn *conn = PQconnectdb(conninfo.c_str());
  if (!conn || PQstatus(conn) != CONNECTION_OK) {
    std::string err = conn ? PQerrorMessage(conn) : "out of memory";
    if (conn) {
      PQfinish(conn);
    }
    throw PGConnectionException("PgMessaging: connect failed: " + err);
  }
  return conn;
}

void prepare(void *conn_, const char *name, const char *sql) {
  PGconn *conn = static_cast<PGconn *>(conn_);
  ResultGuard g(PQprepare(conn, name, sql, 0, nullptr));
  if (PQresultStatus(g.res) != PGRES_COMMAND_OK) {
    throw PGQueryException(std::string("PgMessaging: prepare '") + name +
                               "' failed: " + PQresultErrorMessage(g.res),
                           PQresultErrorField(g.res, PG_DIAG_SQLSTATE));
  }
}

void check_result_raw(PGresult *res, ExecStatusType expected,
                      const std::string &ctx) {
  if (PQresultStatus(res) != expected) {
    std::string expectedStatus = PQresStatus(expected);
    std::string gotStatus = PQresStatus(PQresultStatus(res));
    std::string err = PQresultErrorMessage(res);
    std::string sqlstate = PQresultErrorField(res, PG_DIAG_SQLSTATE);
    PQclear(res);
    if (sqlstate.find("08", 0) == 0) { // class 08 — connection exception
      throw PGConnectionException("PgMessaging: check_result_raw '" + ctx +
                                  "' failed: expected '" + expectedStatus +
                                  "', got '" + gotStatus + "', error: " + err);
    }
    throw PGQueryException("PgMessaging: check_result_raw '" + ctx +
                               "' failed: expected '" + expectedStatus +
                               "', got '" + gotStatus + "', error: " + err,
                           sqlstate);
  }
}

void check_result(ResultGuard &g, ExecStatusType expected,
                  const std::string &ctx) {
  try {
    check_result_raw(g.res, expected, ctx);
  } catch (const std::exception &e) {
    g.res = nullptr;
    throw;
  }
}

void check_command_raw(PGresult *res, const std::string &ctx) {
  check_result_raw(res, PGRES_COMMAND_OK, ctx);
}

void check_command(ResultGuard &g, const std::string &ctx) {
  try {
    check_command_raw(g.res, ctx);
  } catch (const std::exception &e) {
    g.res = nullptr;
    throw;
  }
}
} // namespace
// ===========================================================================
// PgMessaging — construction / prepared statements
// ===========================================================================

std::ostream &operator<<(std::ostream &os, const Message &m) {
  std::ostringstream oss;
  oss << "Message: id=" << m.id << ", topic=" << m.topic
      << ", content_type=" << m.content_type << ", created_at="
      << std::format("{:%FT%T}Z", floor<std::chrono::seconds>(m.created_at))
      << std::endl;

  if (m.data.empty()) {
    oss << "---- data is empty ----";
  } else if (m.content_type == CONTENT_TYPE_JSON ||
             m.content_type == CONTENT_TYPE_TEXT ||
             m.content_type == CONTENT_TYPE_XML) {
    oss << "---- data ----\n" << m.data << "\n---- end ----";
  } else {
    int n = std::min(static_cast<int>(m.data.length()), 5);
    oss << "---- data binary (first " << n << " bytes) ----\n"
        << m.data.substr(0, n) << "\n---- end ----";
  }
  os << oss.str();
  return os;
}

PgMessaging::PgMessaging(PgCluster &cluster) {
  primaryCon_ = connect(cluster.primary_conninfo());
  replicaCon_ = connect(cluster.replica_conninfo());
  prepare_primary_stmts();
  prepare_replica_stmts();
}

PgMessaging::~PgMessaging() {
  PGconn *primary_ = static_cast<PGconn *>(primaryCon_);
  PGconn *replica_ = static_cast<PGconn *>(replicaCon_);
  if (primary_) {
    PQfinish(primary_);
    primaryCon_ = nullptr;
  }
  if (replica_) {
    PQfinish(replica_);
    replicaCon_ = nullptr;
  }
}

void PgMessaging::prepare_primary_stmts() {
  // Statements that write — executed on primary_.
  prepare(primaryCon_, "create_topic",
          R"(INSERT INTO topic (topic) VALUES ($1) ON CONFLICT DO NOTHING
    )");

  prepare(primaryCon_, "publish",
          R"(
        INSERT INTO messages (topic, data, content_type)
        VALUES ($1, $2::bytea, $3)
        RETURNING id
    )");
  //  "INSERT INTO messages (topic, data) VALUES ($1, $2) RETURNING id");

  prepare(primaryCon_, "publish_dedup",
          R"(
        WITH existing AS (
            SELECT id FROM messages
            WHERE topic = $1 AND crc = md5($2)::uuid
            LIMIT 1
        ),
        inserted AS (
            INSERT INTO messages (topic, data, content_type)
            SELECT $1, $2::bytea, $3
            WHERE NOT EXISTS (SELECT 1 FROM existing)
            RETURNING id
        )
        SELECT id FROM inserted
        UNION ALL
        SELECT id FROM existing
        LIMIT 1
    )");

  // prepare(primaryCon_, "register_consumer", std::format(R"(
  //       INSERT INTO consumer_offsets (consumer_name, topic, last_id)
  //       VALUES ($1, $2, 0)
  //       ON CONFLICT (consumer_name, topic) DO NOTHING
  //   )").c_str());

  prepare(primaryCon_, "register_consumer",
          R"(
        INSERT INTO consumer_offsets (consumer_name, topic)
        VALUES ($1, $2)
        ON CONFLICT (consumer_name, topic) DO UPDATE SET consumer_name = EXCLUDED.consumer_name
        RETURNING last_id
    )");

  prepare(primaryCon_, "commit_offset",
          R"(
        UPDATE consumer_offsets
        SET last_id = $3, updated_at = now()
        WHERE consumer_name = $1 AND topic = $2
    )");

  prepare(primaryCon_, "set_offset_at_beginning",
          R"(
        UPDATE consumer_offsets
        SET last_id = (SELECT COALESCE(MIN(id)-1, 0) FROM messages WHERE topic = $2),
        updated_at = now()
        WHERE consumer_name = $1 AND topic = $2
    )");

  prepare(primaryCon_, "set_offset_at_end",
          R"(
        UPDATE consumer_offsets
        SET last_id = (SELECT COALESCE(MAX(id), 0) FROM messages WHERE topic = $2), 
        updated_at = now()
        WHERE consumer_name = $1 AND topic = $2
    )");
}

void PgMessaging::prepare_replica_stmts() {
  // Statements that only read — executed on replica_.
  prepare(replicaCon_, "get_offset",
          R"(
        SELECT last_id FROM consumer_offsets
        WHERE consumer_name = $1 AND topic = $2
    )");

  prepare(replicaCon_, "poll",
          R"(
        SELECT id, topic, data, content_type, created_at
        FROM messages
        WHERE topic = $1
          AND id > $2
        ORDER BY id
        LIMIT $3
    )");
  prepare(replicaCon_, "consumer_lag",
          R"(
        SELECT COALESCE(MAX(m.id), 0) - co.last_id AS lag
        FROM consumer_offsets co
        LEFT JOIN messages m ON m.topic = co.topic
        WHERE co.consumer_name = $1 AND co.topic = $2
        GROUP BY co.last_id
    )");
  prepare(replicaCon_, "topic_exists",
          R"(
        SELECT EXISTS (
            SELECT 1 FROM topic
            WHERE topic = $1
        )
    )");

  prepare(replicaCon_, "list_topics",
          R"(
        SELECT topic FROM topic
    )");
}

// ===========================================================================
// PgMessaging — Topics
// ===========================================================================

void PgMessaging::create_topic(const std::string &topic) {
  const char *p[] = {topic.c_str()};
  ResultGuard g(PQexecPrepared(static_cast<PGconn *>(primaryCon_),
                               "create_topic", 1, p, nullptr, nullptr, 0));
  check_command(g, "create_topic");
}

std::vector<std::string> PgMessaging::list_topics() const {
  ResultGuard g(PQexecPrepared(static_cast<PGconn *>(replicaCon_),
                               "list_topics", 0, nullptr, nullptr, nullptr, 0));
  check_result(g, PGRES_TUPLES_OK, "list_topics");
  std::vector<std::string> topics;
  for (int i = 0; i < PQntuples(g.res); ++i) {
    topics.push_back(PQgetvalue(g.res, i, 0));
  }
  return topics;
}

bool PgMessaging::topic_exists(const std::string &topic) const {
  const char *p[] = {topic.c_str()};
  ResultGuard g(PQexecPrepared(static_cast<PGconn *>(replicaCon_),
                               "topic_exists", 1, p, nullptr, nullptr, 0));
  check_result(g, PGRES_TUPLES_OK, "topic_exists");
  return PQntuples(g.res) > 0 && std::string(PQgetvalue(g.res, 0, 0)) == "t";
}

// ===========================================================================
// PgMessaging — Producer (primary)
// ===========================================================================

long long PgMessaging::publish(const std::string &topic,
                               const std::string &data,
                               const std::string &content_type) {
  const char *p[] = {topic.c_str(), data.c_str(), content_type.c_str()};
  ResultGuard g(PQexecPrepared(static_cast<PGconn *>(primaryCon_), "publish", 3,
                               p, nullptr, nullptr, 0));
  check_result(g, PGRES_TUPLES_OK, "publish");
  if (PQntuples(g.res) == 0)
    throw std::runtime_error("publish: no id returned");
  return std::stoll(PQgetvalue(g.res, 0, 0));
}

long long PgMessaging::publish_dedup(const std::string &topic,
                                     const std::string &data,
                                     const std::string &content_type) {
  const char *p[] = {topic.c_str(), data.c_str(), content_type.c_str()};
  ResultGuard g(PQexecPrepared(static_cast<PGconn *>(primaryCon_),
                               "publish_dedup", 3, p, nullptr, nullptr, 0));
  check_result(g, PGRES_TUPLES_OK, "publish_dedup");
  if (PQntuples(g.res) == 0)
    throw std::runtime_error("publish_dedup: no id returned");
  return std::stoll(PQgetvalue(g.res, 0, 0));
}

// ===========================================================================
// PgMessaging — Consumer writes (primary)
// ===========================================================================

long long PgMessaging::register_consumer(const std::string &consumer_name,
                                         const std::string &topic) {
  const char *p[] = {consumer_name.c_str(), topic.c_str()};
  ResultGuard g(PQexecPrepared(static_cast<PGconn *>(primaryCon_),
                               "register_consumer", 2, p, nullptr, nullptr, 0));
  check_result(g, PGRES_TUPLES_OK, "register_consumer");
  if (PQntuples(g.res) == 0) {
    return 0;
  }
  return std::stoll(PQgetvalue(g.res, 0, 0));
}

void PgMessaging::commit_offset(const std::string &consumer_name,
                                const std::string &topic, long long last_id) {
  std::string id_str = std::to_string(last_id);
  const char *p[] = {consumer_name.c_str(), topic.c_str(), id_str.c_str()};
  ResultGuard g(PQexecPrepared(static_cast<PGconn *>(primaryCon_),
                               "commit_offset", 3, p, nullptr, nullptr, 0));
  check_command(g, "commit_offset");
}

void PgMessaging::set_consumer_offset(const std::string &consumer_name,
                                      const std::string &topic,
                                      ConsumeFromMode mode) {
  const char *p[] = {consumer_name.c_str(), topic.c_str()};

  if (mode == CONSUME_FROM_BEGINNING) {
    ResultGuard g(PQexecPrepared(static_cast<PGconn *>(primaryCon_),
                                 "set_offset_at_beginning", 2, p, nullptr,
                                 nullptr, 0));
    check_command(g, "set_offset_at_beginning");
  } else if (mode == CONSUME_FROM_END) {
    ResultGuard g(PQexecPrepared(static_cast<PGconn *>(primaryCon_),
                                 "set_offset_at_end", 2, p, nullptr, nullptr,
                                 0));
    check_command(g, "set_offset_at_end");
  } else {
    throw std::invalid_argument("Invalid ConsumeFromMode");
  }
}

// ===========================================================================
// PgMessaging — Consumer reads (replica)
// ===========================================================================

long long PgMessaging::get_offset(const std::string &consumer_name,
                                  const std::string &topic) {
  const char *p[] = {consumer_name.c_str(), topic.c_str()};
  ResultGuard g(PQexecPrepared(static_cast<PGconn *>(replicaCon_), "get_offset",
                               2, p, nullptr, nullptr, 0));
  check_result(g, PGRES_TUPLES_OK, "get_offset");
  if (PQntuples(g.res) == 0)
    return 0;
  return std::stoll(PQgetvalue(g.res, 0, 0));
}

namespace {
// Helper functions for querying messages in binary format from PostgreSQL
std::tuple<std::int64_t, bool> parse_binary_int64(PGresult *res, int row,
                                                  int col) {
  if (PQgetisnull(res, row, col)) {
    return {0, false};
  }
  const char *p = PQgetvalue(res, row, col);
  auto len = PQgetlength(res, row, col);
  std::uint64_t v = 0;
  for (int i = 0; i < len; ++i) {
    v = (v << 8) | static_cast<unsigned char>(p[i]);
  }
  return std::make_tuple(static_cast<std::int64_t>(v), true);
}

// Postgres epoch (2000-01-01) minus Unix epoch (1970-01-01), in seconds.
constexpr std::int64_t pg_epoch_offset_s = 946'684'800;

std::chrono::sys_time<std::chrono::microseconds>
read_timestamptz(PGresult *res, int row, int col) {
  using namespace std::chrono;
  std::int64_t us_since_2000 = std::get<0>(parse_binary_int64(res, row, col));
  return sys_time<microseconds>{microseconds{us_since_2000} +
                                seconds{pg_epoch_offset_s}};
}

} // namespace

std::vector<Message> PgMessaging::poll(const std::string &consumer_name,
                                       const std::string &topic, int limit) {
  std::int64_t offset = get_offset(consumer_name, topic);
  std::string offset_str = std::to_string(offset);
  std::string limit_str = std::to_string(limit);

  const char *p[] = {topic.c_str(), offset_str.c_str(), limit_str.c_str()};
  ResultGuard g(PQexecPrepared(static_cast<PGconn *>(replicaCon_), "poll", 3, p,
                               nullptr, nullptr,
                               1)); // 1 request binary results
  check_result(g, PGRES_TUPLES_OK, "poll");

  std::vector<Message> rows;
  int n = PQntuples(g.res);
  rows.reserve(n);
  for (int i = 0; i < n; ++i) {
    Message m;
    // id, topic, data, content_type, created_at
    m.id = std::get<0>(parse_binary_int64(g.res, i, 0));
    m.topic = std::string(PQgetvalue(g.res, i, 1), PQgetlength(g.res, i, 1));
    m.data.assign(PQgetvalue(g.res, i, 2), PQgetlength(g.res, i, 2));
    m.content_type =
        std::string(PQgetvalue(g.res, i, 3), PQgetlength(g.res, i, 3));
    m.created_at = read_timestamptz(g.res, i, 4);
    rows.push_back(std::move(m));
  }
  return rows;
}

std::int64_t PgMessaging::consumer_lag(const std::string &consumer_name,
                                       const std::string &topic) {
  const char *p[] = {consumer_name.c_str(), topic.c_str()};
  ResultGuard g(PQexecPrepared(static_cast<PGconn *>(replicaCon_),
                               "consumer_lag", 2, p, nullptr, nullptr, 0));
  check_result(g, PGRES_TUPLES_OK, "consumer_lag");
  if (PQntuples(g.res) == 0)
    return 0;
  return static_cast<std::int64_t>(std::stoll(PQgetvalue(g.res, 0, 0)));
}
} // namespace pgqueue