/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 Copyright (C) 2015 met.no

 Contact information:
 Norwegian Meteorological Institute
 Box 43 Blindern
 0313 OSLO
 NORWAY
 email: kvalobs-dev@met.no

 This file is part of KVALOBS

 KVALOBS is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License as
 published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 KVALOBS is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 General Public License for more details.

 You should have received a copy of the GNU General Public License along
 with KVALOBS; if not, write to the Free Software Foundation Inc.,
 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef SRC_SERVICE_LIBS_KVCPP_PG_PGSUBSCRIBE_H_
#define SRC_SERVICE_LIBS_KVCPP_PG_PGSUBSCRIBE_H_

#include "kvcpp/KvApp.h"
#include "kvsubscribe/pgconfig.h"
#include <boost/noncopyable.hpp>
#include <condition_variable>
#include <map>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace kvalobs {
namespace subscribe {
class Consumer;
}
} // namespace kvalobs

namespace kvservice {
namespace pg {

class PgSubscribe : public virtual details::KvalobsSubscribe,
                    virtual details::KvAppControl,
                    boost::noncopyable {
public:
  PgSubscribe(kvalobs::subscribe::PgConfig config);
  ~PgSubscribe();

  virtual SubscriberID subscribeData(const KvDataSubscribeInfoHelper &info,
                                     dnmi::thread::CommandQue &queue);

  /**
   * Subscribe on data with a kafka consumer group, group.id
   */
  virtual SubscriberID
  subscribeDataWithGroupId(const KvDataSubscribeInfoHelper &info,
                           dnmi::thread::CommandQue &queue,
                           const std::string &groupId);

  virtual SubscriberID
  subscribeDataNotify(const KvDataSubscribeInfoHelper &info,
                      dnmi::thread::CommandQue &que);

  virtual SubscriberID subscribeKvHint(dnmi::thread::CommandQue &queue);

  virtual void unsubscribe(const SubscriberID &subscriberid);

  virtual void unsubscribeAll();

  bool knowsAbout(const SubscriberID &subscriberid) const;

  /**
   * Stop and wait for thread to finish
   */
  void join(const SubscriberID &subscriberid);

  /**
   * Stop all and wait for threads to finish
   */
  void joinAll();

  virtual bool shutdown() const;
  virtual void doShutdown();
  virtual void run();

private:
  typedef std::shared_ptr<kvalobs::subscribe::Consumer> ConsumerPtr;
  typedef std::pair<ConsumerPtr, std::thread> RunnableConsumer;
  typedef std::map<std::string, RunnableConsumer> ConsumerCollection;

  // std::string domain_;
  // int consumerPollSize_;
  // std::vector<std::string> connections_;
  // std::string consumerGroup_;
  kvalobs::subscribe::PgConfig config_;
  ConsumerCollection consumers_;

  RunnableConsumer &getConsumer_(const SubscriberID &subscriberid);

  std::mutex mutex_;
  std::condition_variable blocker_;
  bool shutdown_;
};

} /* namespace pg */
} /* namespace kvservice */

#endif /* SRC_SERVICE_LIBS_KVCPP_PG_PGSUBSCRIBE_H_ */
