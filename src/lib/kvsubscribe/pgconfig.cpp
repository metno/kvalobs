#include "pgconfig.h"
#include "lib/miconfparser/miconfparser.h"
#include <string>
#include <vector>

namespace kvalobs {
namespace subscribe {

namespace {

std::string getValue(const std::string &key, miutil::conf::ConfSection *conf,
                     const std::string &defaultValue = "") {
  miutil::conf::ValElementList ret = conf->getValue(key);
  if (ret.empty()) {
    return defaultValue;
  }
  auto val = ret.valAsString();
  if (val.empty()) {
    return defaultValue;
  }
  return val;
}

int getIntValue(const std::string &key, miutil::conf::ConfSection *conf,
                int defaultValue) {
  miutil::conf::ValElementList ret = conf->getValue(key);
  if (ret.empty()) {
    return defaultValue;
  }
  return ret.valAsInt(defaultValue);
}

std::string pgDomain(miutil::conf::ConfSection *conf) {
  auto ret = getValue("pgqueue.domain", conf, "");
  if (ret.empty()) {
    throw std::runtime_error("pgqueue.domain is not set in the configuration");
  }
  return ret;
}

int pgConsumerPollSize(miutil::conf::ConfSection *conf,
                       const std::string &progname, int defaultValue) {
  if (!progname.empty()) {
    auto v =
        getIntValue("pgqueue." + progname + ".consumer_poll_size", conf, -1);
    if (v != -1) {
      return v;
    }
  }
  return getIntValue("pgqueue.consumer_poll_size", conf, defaultValue);
}

std::vector<std::string> pgConnections(miutil::conf::ConfSection *conf) {

  std::vector<std::string> result;
  if (auto val = getValue("pgqueue.database_a", conf); !val.empty()) {
    result.push_back(val);
  }
  if (auto val = getValue("pgqueue.database_b", conf); !val.empty()) {
    result.push_back(val);
  }

  if (result.empty()) {
    throw std::runtime_error("No pgqueue databases are configured");
  }

  return result;
}

bool pgEnabled(miutil::conf::ConfSection *conf) {
  return conf->getValue("pgqueue.enabled").valAsBool(true);
}
} // namespace

PgConfig PgConfig::config(const miutil::conf::ConfSection *conf,
                          const std::string &progname,
                          int defaultConsumerPollSize) {
  PgConfig cfg;
  cfg.pollSize =
      pgConsumerPollSize(const_cast<miutil::conf::ConfSection *>(conf),
                         progname, defaultConsumerPollSize);
  cfg.connections =
      pgConnections(const_cast<miutil::conf::ConfSection *>(conf));
  cfg.domain = pgDomain(const_cast<miutil::conf::ConfSection *>(conf));
  cfg.enabled = pgEnabled(const_cast<miutil::conf::ConfSection *>(conf));
  return cfg;
}
std::string PgConfig::topic(TopicType t) const {
  switch (t) {
  case raw:
    return "kvalobs." + domain + ".raw";
  case checked:
    return "kvalobs." + domain + ".checked";
  default:
    return "kvalobs." + domain + ".checked";
  }
}

} // namespace subscribe
} // namespace kvalobs