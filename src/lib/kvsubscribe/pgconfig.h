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
  typedef enum { raw, checked } TopicType;

  std::string domain;
  std::vector<std::string> connections;
  int pollSize;
  std::string consumerGroup;
  bool enabled;
  

  PgConfig() : enabled(false), pollSize(0) {}
  PgConfig(const std::string &domain,
           const std::vector<std::string> &connections,
           int pollSize,
           const std::string &consumerGroup,
           bool enabled)
      : domain(domain),
        connections(connections),
        pollSize(pollSize),
        consumerGroup(consumerGroup),
        enabled(enabled) {}

  std::string topic(TopicType t = checked) const;
  static PgConfig config(const miutil::conf::ConfSection *conf,
                         const std::string &progname = "",
                         int defaultConsumerPollSize = 10);


  // Returns the consumer group ID for the given program name, creating one if necessary (program name is not empty). 
  // If the program name is empty and no consumer group is configured, it will return an empty string. 
  static std::string getConsumerGroupId(const miutil::conf::ConfSection *conf,const std::string &progname);
};

} // namespace subscribe
} // namespace kvalobs
#endif // __KVALOBS_SUBSCRIBR_PGCONFIG_H__