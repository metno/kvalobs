#ifndef __KVQUEUEDB_H__
#define __KVQUEUEDB_H__

#include "src/lib/kvsubscribe/KvqueueConfig.h"
#include <list>
#include <string>

namespace kvalobs {
namespace subscribe {

class KvqueueDB {
public:
  struct Data {
    std::string data;
    MessageId id;
    std::string topic;

    Data(const char *data, unsigned length, MessageId id,
         const std::string &topic)
        : data(data, length), id(id), topic(topic) {}
    Data(const std::string &data, MessageId id, const std::string &topic)
        : data(data), id(id), topic(topic) {}
  };

  KvqueueDB();
  KvqueueDB(const KvqueueDB &) = delete;
  KvqueueDB(const KvqueueConfig &config);

  KvqueueDB &operator=(const KvqueueDB &) = delete;
  virtual ~KvqueueDB();
  virtual void send(const std::string &data) = 0;
  virtual void send(const char *data, unsigned length) = 0;
  virtual void catchup() = 0;

  virtual void init() = 0;
  virtual void close() = 0;

private:
  std::string topic;
  std::list<std::string> brokers;
};

} // namespace subscribe
} // namespace kvalobs

#endif
