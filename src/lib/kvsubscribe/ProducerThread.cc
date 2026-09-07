/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 Copyright (C) 2007 met.no

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

#include "lib/kvsubscribe/ProducerThread.h"
#include "lib/kvsubscribe/KafkaProducer.h"
#include "lib/kvsubscribe/Producer.h"
#include "lib/kvsubscribe/messageid.h"
#include "lib/milog/milog.h"
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <sstream>

using miutil::concurrent::BlockingQueuePtr;
using miutil::concurrent::QueueIllegalState;
using std::map;
using std::shared_ptr;
using std::string;
using std::thread;
using std::unique_ptr;

namespace kvalobs {
namespace service {

namespace {

std::mutex mutex;
std::string getThreadId(const std::string &name) {
  if (!name.empty())
    return name;

  std::ostringstream sid;
  std::lock_guard<std::mutex> lock(mutex);
  static unsigned int id = 0;
  sid << "ProducerThread - " << id;
  ++id;
  return sid.str();
}

void nullErrorHandler(kvalobs::subscribe::MessageId, const std::string &, const std::string &){}
void nullSuccessHandler(kvalobs::subscribe::MessageId, const std::string &) {}


class Thread {
  typedef map<kvalobs::subscribe::MessageId,
              std::shared_ptr<ProducerCommand>>
      WaitingAck;
  shared_ptr<ProducerQue> que;
  shared_ptr<BlockingQueuePtr<string>> statusQue;
  WaitingAck waitingAck;
  std::string name;
  kvalobs::subscribe::Producer *producer;
  kvalobs::subscribe::Producer::ErrorHandler oldOnError = nullErrorHandler;
  kvalobs::subscribe::Producer::SuccessHandler oldOnSuccess = nullSuccessHandler;

  shared_ptr<ProducerCommand> getWaitingMessage(kvalobs::subscribe::MessageId id) {
    WaitingAck::iterator it = waitingAck.find(id);

    if (it == waitingAck.end())
      return shared_ptr<ProducerCommand>();

    shared_ptr<ProducerCommand> ret = it->second;
    waitingAck.erase(it);
    return ret;
  }

  
  void send(ProducerCommand *cmd) {
    if (cmd) {
      unsigned int size;
      const char *data = cmd->getData(&size);
      if (data && size > 0) {
        kvalobs::subscribe::Producer::QueueType queueType;
        if ( cmd->topicQueue() == ProducerCommand::TopicType::checked) {
          if(cmd->obstime() < (boost::posix_time::second_clock::universal_time()-
            boost::posix_time::hours(24))) {
            queueType = kvalobs::subscribe::Producer::QueueType::checked_lowpri;
          } else {
            queueType = kvalobs::subscribe::Producer::QueueType::checked;
          }
        } else {
          queueType = kvalobs::subscribe::Producer::QueueType::raw;
        }
        kvalobs::subscribe::MessageId msgId = producer->send(data, size,queueType);
        cmd->onSend(msgId, name);
        waitingAck[msgId] = shared_ptr<ProducerCommand>(cmd);
      } else {
        cmd->onSend(0, name);
        cmd->onSuccess(0, name, std::string());
      }
    }
  }

public:
  Thread(const std::string &myName, const string &brokers,
              const std::string &topic, shared_ptr<ProducerQue> myQue,
              shared_ptr<BlockingQueuePtr<std::string>> myStatusQue){
    producer= new  kvalobs::subscribe::KafkaProducer(
            topic, brokers,
            [this](kvalobs::subscribe::MessageId msgId, const std::string &data,
                   const std::string &errorMessage) {
              onError(msgId, data, errorMessage);
            },
            [this](kvalobs::subscribe::MessageId msgId, const std::string &data) {
              onSuccess(msgId, data);
            });
        que=myQue; 
        statusQue=myStatusQue;
        name=myName;
      }

    Thread(kvalobs::subscribe::Producer *myProducer, const std::string &myName, 
       shared_ptr<ProducerQue> myQue,
       shared_ptr<BlockingQueuePtr<std::string>> myStatusQue)
    {
      producer=myProducer;
      que=myQue; 
      statusQue=myStatusQue;
      name=myName;
      oldOnError=producer->setErrorHandler([this](kvalobs::subscribe::MessageId msgId, const std::string &data,
                   const std::string &errorMessage) {
              onError(msgId, data, errorMessage);
            });
      oldOnSuccess=producer->setSuccessHandler([this](kvalobs::subscribe::MessageId msgId, const std::string &data) {
              onSuccess(msgId, data);
            });
    
      }


  void onSuccess(kvalobs::subscribe::MessageId msgId, const std::string &data) {
    shared_ptr<ProducerCommand> cmd = getWaitingMessage(msgId);
    if (cmd){
      cmd->onSuccess(msgId, name, data);
      if (oldOnSuccess){
        oldOnSuccess(msgId, data);
      }
    }
  }

  void onError(kvalobs::subscribe::MessageId msgId, const std::string &data,
               const std::string &errorMessage) {
    shared_ptr<ProducerCommand> cmd = getWaitingMessage(msgId);
    if (cmd){
      cmd->onError(msgId, name, data, errorMessage);
      if (oldOnError){
        oldOnError(msgId, data, errorMessage);
      }
    }
  }

  void drainQue() {
    try {
      while (!que->empty()) {
        send(que->getAfterSuspend());
      }
    } catch (const QueueIllegalState &ex) {
    } catch (...) {
    }

    while (!waitingAck.empty())
      producer->catchup(2000);
  }

  void run() {
    bool running = true;
    while (running) {
      try {
        producer->catchup(0);
        send(que->timedGet(std::chrono::milliseconds(500), false));
      } catch (const miutil::concurrent::QueueSuspended &ex) {
        running = false;
      } catch (const std::exception &ex) {
      }
    }
    drainQue();
  }

  static void startKafka(const std::string &name, const std::string &brokers,
                    const std::string &topic, ProducerQuePtr que,
                    shared_ptr<BlockingQueuePtr<std::string>> statusQue) {
    try {
      Thread myThread(name, brokers, topic, que, statusQue);
      statusQue->add(new std::string("<STARTED>"));
      myThread.run();
      statusQue->add(new std::string("<EXIT>"));
    } catch (const std::exception &ex) {
      // Constructor failure
      statusQue->add(new std::string(ex.what()));
    }
  }

  static void startProducer( 
      kvalobs::subscribe::Producer *producer,
      const std::string &name,
      ProducerQuePtr que,
      shared_ptr<BlockingQueuePtr<std::string>> statusQue) {
    try {
      Thread myThread(producer, name, que, statusQue);
      statusQue->add(new std::string("<STARTED>"));
      myThread.run();
      statusQue->add(new std::string("<EXIT>"));
    } catch (const std::exception &ex) {
      // Constructor failure
      statusQue->add(new std::string(ex.what()));
    }
  }
};



} // namespace

ProducerThread::ProducerThread(const std::string &name,
                               unsigned int queueSize)
    : statusQue(new miutil::concurrent::BlockingQueuePtr<std::string>()),
      name(getThreadId(name)), queue(new ProducerQue(queueSize)) {}

ProducerThread::~ProducerThread() {
  if (producerThread.joinable())
    producerThread.detach();
}
void ProducerThread::setName(const std::string &name_) { name = name_; }

size_t ProducerThread::getQueSize() const {
  return queue->size();
}

void ProducerThread::setMaxQueSize(unsigned int  maxSize) {
  queue->resize(maxSize);
}


void ProducerThread::send(ProducerCommand *cmd) {
  try {
    queue->add(cmd);
  } catch (const std::exception &ex) {
  }
}

void ProducerThread::start(const std::string &brokers,
                                const std::string &topic) {
  producerThread = thread(Thread::startKafka, name, brokers, topic, queue, statusQue);
  string *res = statusQue->get();
  if (*res == "<STARTED>") {
    LOGINFO("KafkaProducerThread: " << name << ": started.");
    return;
  } else {
    throw std::runtime_error(name + ": " + *res);
  }
}

void ProducerThread::start(kvalobs::subscribe::Producer *producer){
    producerThread = thread(Thread::startProducer, producer, name, queue, statusQue);
  string *res = statusQue->get();
  if (*res == "<STARTED>") {
    LOGINFO("KafkaProducerThread: " << name << ": started.");
    return;
  } else {
    throw std::runtime_error(name + ": " + *res);
  }
}

void ProducerThread::shutdown() {
  try {
    queue->suspend();
  } catch (...) {
  }
}

void ProducerThread::join(
    const std::chrono::high_resolution_clock::duration &timeout) {
  try {
    string *res = statusQue->timedGet(timeout, true);
    if (*res == "<EXIT>") {
      producerThread.join();
      LOGINFO("KafkaProducerThread: " << name << ": stopped.");
    } else {
      LOGERROR(name << ": join: Unexpected return from thread <" + *res + ">.");
    }
  } catch (const miutil::concurrent::QueueTimeout &ex) {
    if (producerThread.joinable())
      producerThread.detach();
  }
}

} //  namespace service
} //  namespace kvalobs
