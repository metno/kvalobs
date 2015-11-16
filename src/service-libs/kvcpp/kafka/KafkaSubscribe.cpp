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


#include "KafkaSubscribe.h"
#include "../kvevents.h"
#include <kvsubscribe/NotificationSubscriber.h>
#include <kvsubscribe/DataSubscriber.h>
#include <kvsubscribe/Notification.h>
#include <decodeutility/kvalobsdata.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <stdexcept>
#include <list>


using namespace kvalobs::subscribe;
using namespace kvalobs;

namespace kvservice
{
namespace kafka
{

namespace
{
std::string uniqueString()
{
    static boost::uuids::random_generator generator;
    return boost::uuids::to_string(generator());
}

bool match(const KvDataSubscribeInfoHelper &info, const Notification & n)
{
    auto info_ = info.getDataSubscribeInfo();

    if ( info_->ids.length() == 0 )
        return true;
    for ( int i = 0; i < info_->ids.length(); ++ i )
        if ( info_->ids[i] == n.station() )
            return true;
    return false;

}

void broadcast(const Notification & n, dnmi::thread::CommandQue &queue)
{
    KvWhatListPtr newData(new KvWhatList);
    newData->push_back(KvWhat(n.station(), n.type(), n.obstime()));
    kvservice::DataNotifyEvent * event = new kvservice::DataNotifyEvent(newData);
    queue.postAndBrodcast(event);
}

void broadcast(const ::kvalobs::serialize::KvalobsData & d, dnmi::thread::CommandQue & queue)
{
    std::map<int, kvservice::KvObsData> elements;

    boost::posix_time::ptime fakeTbtime(boost::posix_time::microsec_clock::universal_time());

    std::list<kvData> data;
    d.getData(data, fakeTbtime);
    for (const kvData & d : data)
    {
        kvservice::KvObsData & obsData = elements[d.stationID()];
        if ( obsData.stationid() == 0 and d.stationID() != 0 )
            obsData = kvservice::KvObsData(d.stationID());
        obsData.dataList().push_back(d);
    }

    std::list<kvTextData> textData;
    d.getData(textData, fakeTbtime);
    for (const kvTextData & d : textData)
    {
        kvservice::KvObsData & obsData = elements[d.stationID()];
        if ( obsData.stationid() == 0 and d.stationID() != 0 )
            obsData = kvservice::KvObsData(d.stationID());
        obsData.textDataList().push_back(d);
    }

    kvservice::KvObsDataListPtr toSend(new kvservice::KvObsDataList);
    for ( auto element : elements )
        toSend->push_back(element.second);

    kvservice::DataEvent * event = new kvservice::DataEvent(toSend);
    queue.postAndBrodcast(event);
}

void broadcast(const ::kvalobs::serialize::KvalobsData & d, const KvDataSubscribeInfoHelper &info, dnmi::thread::CommandQue & queue)
{
    auto stations = info.getDataSubscribeInfo()->ids;
    if ( stations.length() == 0 )
    {
        broadcast(d, queue);
        return;
    }

    std::set<int> st;
    for ( int i = 0; i < stations.length(); ++ i )
    	st.insert(stations[i]);


    boost::posix_time::ptime fakeTbtime(boost::posix_time::microsec_clock::universal_time());
    std::list<kvData> data;
    {
        std::list<kvData> dataInMessage;
        d.getData(dataInMessage, fakeTbtime);
        std::copy_if(dataInMessage.begin(), dataInMessage.end(), std::back_inserter(data),
                [& st](const kvalobs::kvData & d) {
        	return st.find(d.stationID()) != st.end();
        });
    }

    std::list<kvTextData> textData;
    {
        std::list<kvTextData> textDataInMessage;
        d.getData(textDataInMessage, fakeTbtime);
        std::copy_if(textDataInMessage.begin(), textDataInMessage.end(), std::back_inserter(textData),
                [& st](const kvalobs::kvTextData & d) {
        	return st.find(d.stationID()) != st.end();
        });
    }

    broadcast(serialize::KvalobsData(data, textData), queue);
}

}

KafkaSubscribe::KafkaSubscribe(const std::string & brokers) :
        brokers_(brokers)
{
}

KafkaSubscribe::~KafkaSubscribe()
{
    joinAll();
}

KafkaSubscribe::SubscriberID KafkaSubscribe::subscribeDataNotify( const KvDataSubscribeInfoHelper &info, dnmi::thread::CommandQue &queue )
{
    ConsumerPtr runner(new NotificationSubscriber(
            [info, & queue](const Notification & n) {
                if (match(info, n))
                    broadcast(n, queue);
            },
            NotificationSubscriber::Latest,
            brokers_
    ));


    std::string ret = uniqueString();
    consumers_[ret] = std::make_pair(runner, std::thread([runner](){runner->run();}));
    return ret;
}

KafkaSubscribe::SubscriberID KafkaSubscribe::subscribeData( const KvDataSubscribeInfoHelper &info, dnmi::thread::CommandQue &queue )
{
    ConsumerPtr runner(new DataSubscriber(
            [info, & queue](const ::kvalobs::serialize::KvalobsData & d) {
                broadcast(d, info, queue);
            },
            NotificationSubscriber::Latest,
            brokers_
    ));


    std::string ret = uniqueString();
    consumers_[ret] = std::make_pair(runner, std::thread([runner](){runner->run();}));
    return ret;
}

KafkaSubscribe::SubscriberID KafkaSubscribe::subscribeKvHint( dnmi::thread::CommandQue &queue )
{
    throw std::runtime_error("Not implemented!");
}

void KafkaSubscribe::unsubscribe( const SubscriberID &subscriberid )
{
    getConsumer_(subscriberid).first->stop();
}

void KafkaSubscribe::unsubscribeAll()
{
    for ( auto & consumer : consumers_ )
        consumer.second.first->stop();
}

bool KafkaSubscribe::knowsAbout( const SubscriberID &subscriberid ) const
{
    return consumers_.find(subscriberid) != consumers_.end();
}

void KafkaSubscribe::join(const SubscriberID &subscriberid)
{
    unsubscribe(subscriberid);
    getConsumer_(subscriberid).second.join();
    consumers_.erase(subscriberid);
}

void KafkaSubscribe::joinAll()
{
    unsubscribeAll();
    for ( auto & consumer : consumers_ )
        consumer.second.second.join();
    consumers_.clear();
}

KafkaSubscribe::RunnableConsumer & KafkaSubscribe::getConsumer_(const SubscriberID &subscriberid)
{
    ConsumerCollection::iterator find = consumers_.find(subscriberid);
    if ( consumers_.end() == find )
        throw std::runtime_error("Attempting to access invalid subscriber");
    return find->second;

}


} /* namespace kafka */
} /* namespace kvservice */
