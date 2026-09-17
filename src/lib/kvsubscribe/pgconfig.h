#ifndef __KVALOBS_SUBSCRIBR_PGCONFIG_H__
#define __KVALOBS_SUBSCRIBR_PGCONFIG_H__

#include <string>
#include <vector>

namespace miutil {
namespace conf {
class ConfSection;
} // namespace conf
} // namespace miutil



namespace kvalobs {
namespace subscribe {

struct PgConfig {
  std::vector<std::string> connections;
  std::string domain;
  bool enabled;
  int pollSize;

  PgConfig() : enabled(false), pollSize(0) {}
  static PgConfig config(const miutil::conf::ConfSection *conf, const std::string &progname="", int defaultConsumerPollSize=10);
};

} // namespace subscribe
} // namespace kvalobs
#endif // __KVALOBS_SUBSCRIBR_PGCONFIG_H__