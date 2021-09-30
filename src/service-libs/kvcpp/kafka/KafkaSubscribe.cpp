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

#include <stdexcept>
#include <list>
#include <set>
#include <mutex>
#include "boost/uuid/uuid.hpp"
#include "boost/uuid/uuid_generators.hpp"
#include "boost/uuid/uuid_io.hpp"
#include "lib/kvsubscribe/DataSubscriber.h"
#include "lib/decodeutility/kvalobsdata.h"
#include "lib/milog/milog.h"
#include "service-libs/kvcpp/kvevents.h"
#include "service-libs/kvcpp/kafka/KafkaSubscribe.h"
#include "service-libs/kvcpp/test/testKafkaSubcriber.h"  // header file for the test interface
#include "creategroupid.h"

using namespace kvalobs::subscribe;
using namespace kvalobs;

namespace kvservice {
namespace kafka {

namespace {
std::string uniqueString() {
  static boost::uuids::random_generator generator;
  return boost::uuids::to_string(generator());
}

void broadcast(const ::kvalobs::serialize::KvalobsData & d,
               dnmi::thread::CommandQue & queue) {
  std::map<int, kvservice::KvObsData> elements;

  boost::posix_time::ptime fakeTbtime(
      boost::posix_time::microsec_clock::universal_time());

  std::list<kvData> data;
  d.getData(data, fakeTbtime);
  for (const kvData & d : data) {
    kvservice::KvObsData & obsData = elements[d.stationID()];
    if (obsData.stationid() == 0 and d.stationID() != 0)
      obsData = kvservice::KvObsData(d.stationID());
    obsData.dataList().push_back(d);
  }

  std::list<kvTextData> textData;
  d.getData(textData, fakeTbtime);
  for (const kvTextData & d : textData) {
    kvservice::KvObsData & obsData = elements[d.stationID()];
    if (obsData.stationid() == 0 and d.stationID() != 0)
      obsData = kvservice::KvObsData(d.stationID());
    obsData.textDataList().push_back(d);
  }

  kvservice::KvObsDataListPtr toSend(new kvservice::KvObsDataList(d.created()));

  for (auto element : elements)
    toSend->push_back(element.second);

  kvservice::DataEvent * event = new kvservice::DataEvent(toSend);
  queue.postAndBrodcast(event);
}

void broadcast(const ::kvalobs::serialize::KvalobsData & d,
               const KvDataSubscribeInfoHelper &info,
               dnmi::thread::CommandQue & queue) {
  auto stations = info.getDataSubscribeInfo()->ids;
  if (stations.length() == 0) {
    broadcast(d, queue);
    return;
  }

  std::set<int> st;
  for (int i = 0; i < stations.length(); ++i)
    st.insert(stations[i]);

  boost::posix_time::ptime fakeTbtime(
      boost::posix_time::microsec_clock::universal_time());
  std::list<kvData> data;
  {
    std::list<kvData> dataInMessage;
    d.getData(dataInMessage, fakeTbtime);
    std::copy_if(dataInMessage.begin(), dataInMessage.end(),
                 std::back_inserter(data), [& st](const kvalobs::kvData & d) {
                   return st.find(d.stationID()) != st.end();
                 });
  }

  std::list<kvTextData> textData;
  {
    std::list<kvTextData> textDataInMessage;
    d.getData(textDataInMessage, fakeTbtime);
    std::copy_if(textDataInMessage.begin(), textDataInMessage.end(),
                 std::back_inserter(textData),
                 [& st](const kvalobs::kvTextData & d) {
                   return st.find(d.stationID()) != st.end();
                 });
  }

  if ( data.empty() && textData.empty() )  // Do not publish empty messages.
    return;

  serialize::KvalobsData theData(data, textData);
  theData.created(d.created());

  broadcast(theData, queue);
}

struct cmpWhat {
  bool operator ()(const KvWhat & a, const KvWhat & b) {
    if (a.stationID() != b.stationID())
      return a.stationID() < b.stationID();
    if (a.typeID() != b.typeID())
      return a.typeID() < b.typeID();
    return a.obsTime() < b.obsTime();
  }
};

void broadcastNotification(
    const ::kvalobs::serialize::KvalobsData & kvalobsData,
    dnmi::thread::CommandQue &queue) {

  std::set<KvWhat, cmpWhat> elements;
  std::list<kvalobs::kvData> data;
  std::list<kvalobs::kvTextData> textData;
  kvalobsData.getData(data, textData);
  for (auto d : data)
    elements.insert(KvWhat(d.stationID(), d.typeID(), d.obstime()));
  for (auto d : textData)
    elements.insert(KvWhat(d.stationID(), d.typeID(), d.obstime()));

  KvWhatListPtr newData(new KvWhatList(elements.begin(), elements.end()));
  kvservice::DataNotifyEvent * event = new kvservice::DataNotifyEvent(newData);
  queue.postAndBrodcast(event);
}

}

KafkaSubscribe::KafkaSubscribe(const std::string & domain,
                               const std::string & brokers)
    : domain_(domain),
      brokers_(brokers),
      shutdown_(false) {
}

KafkaSubscribe::~KafkaSubscribe() {
  joinAll();
}

KafkaSubscribe::SubscriberID KafkaSubscribe::subscribeData(
    const KvDataSubscribeInfoHelper &info, dnmi::thread::CommandQue &queue) {
  auto groupId=KvApp::getConsumerGroupId();
  LOGDEBUG("KafkaSubscribe::subscribeData: kafka consumer group id: '" << groupId << "'.");
  return subscribeDataWithGroupId(info, queue, groupId);
}

KafkaSubscribe::SubscriberID KafkaSubscribe::subscribeDataWithGroupId(const KvDataSubscribeInfoHelper &info,
                                     dnmi::thread::CommandQue &queue, const std::string &groupId){
  ConsumerPtr runner(
      new DataSubscriber(
          [info, & queue](const ::kvalobs::serialize::KvalobsData & d) {
            broadcast(d, info, queue);
          },
          domain_, brokers_, groupId));

  std::string ret = uniqueString();
  consumers_[ret] = std::make_pair(runner,
                                   std::thread([runner]() {runner->run();}));
  return ret;

}


KafkaSubscribe::SubscriberID KafkaSubscribe::subscribeDataNotify(
    const KvDataSubscribeInfoHelper &info, dnmi::thread::CommandQue &queue) {
  ConsumerPtr runner(
      new DataSubscriber(
          [info, & queue](const ::kvalobs::serialize::KvalobsData & d) {
            broadcastNotification(d, queue);
          },
          domain_, brokers_));

  std::string ret = uniqueString();
  consumers_[ret] = std::make_pair(runner,
                                   std::thread([runner]() {runner->run();}));
  return ret;
}

KafkaSubscribe::SubscriberID KafkaSubscribe::subscribeKvHint(
    dnmi::thread::CommandQue &queue) {
  throw std::runtime_error("subscribeKvHint: Not implemented!");
}

void KafkaSubscribe::unsubscribe(const SubscriberID &subscriberid) {
  getConsumer_(subscriberid).first->stop();
}

void KafkaSubscribe::unsubscribeAll() {
  for (auto & consumer : consumers_)
    consumer.second.first->stop();
}

bool KafkaSubscribe::knowsAbout(const SubscriberID &subscriberid) const {
  return consumers_.find(subscriberid) != consumers_.end();
}

void KafkaSubscribe::join(const SubscriberID &subscriberid) {
  unsubscribe(subscriberid);
  getConsumer_(subscriberid).second.join();
  consumers_.erase(subscriberid);
}

void KafkaSubscribe::joinAll() {
  unsubscribeAll();
  for (auto & consumer : consumers_)
    consumer.second.second.join();
  consumers_.clear();
}

KafkaSubscribe::RunnableConsumer & KafkaSubscribe::getConsumer_(
    const SubscriberID &subscriberid) {
  ConsumerCollection::iterator find = consumers_.find(subscriberid);
  if (consumers_.end() == find)
    throw std::runtime_error("Attempting to access invalid subscriber");
  return find->second;

}

bool KafkaSubscribe::shutdown() const {
  return shutdown_;
}

void KafkaSubscribe::doShutdown() {
  shutdown_ = true;
  std::unique_lock<std::mutex> l(mutex_);
  blocker_.notify_all();
}

void KafkaSubscribe::run() {
  std::unique_lock<std::mutex> l(mutex_);
  blocker_.wait(l);
  joinAll();
}

namespace test {
/*
 * The namespace test is used to export function in the anonymous namespace
 * so they are available for unit testing. The test interface is defined in
 * kvcpp/test/testKafkaSubcriber.h.
 */
void broadcast(const ::kvalobs::serialize::KvalobsData & d,
               const KvDataSubscribeInfoHelper &info,
               dnmi::thread::CommandQue & queue) {
  kvservice::kafka::broadcast(d, info, queue);
}
}  // namespace test
}  // namespace kafka
}  // namespace kvservice
