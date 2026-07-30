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

#include "lib/kvsubscribe/KafkaProducerThread.h"
#include "lib/kvsubscribe/KafkaProducer.h"
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
  sid << "KafkaProducerThread-" << id;
  ++id;
  return sid.str();
}

class KafkaThread : kvalobs::subscribe::KafkaProducer {
  typedef map<kvalobs::subscribe::KafkaProducer::MessageId,
              std::shared_ptr<ProducerCommand>>
      WaitingAck;
  shared_ptr<ProducerQue> que;
  shared_ptr<BlockingQueuePtr<string>> statusQue;
  WaitingAck waitingAck;
  std::string name;

  shared_ptr<ProducerCommand> getWaitingMessage(KafkaProducer::MessageId id) {
    WaitingAck::iterator it = waitingAck.find(id);

    if (it == waitingAck.end())
      return shared_ptr<ProducerCommand>();

    shared_ptr<ProducerCommand> ret = it->second;
    waitingAck.erase(it);
    return ret;
  }

  using kvalobs::subscribe::KafkaProducer::send;

  void send(ProducerCommand *cmd) {
    if (cmd) {
      unsigned int size;
      const char *data = cmd->getData(&size);
      if (data && size > 0) {
        kvalobs::subscribe::KafkaProducer::MessageId msgId = send(data, size);
        cmd->onSend(msgId, name);
        waitingAck[msgId] = shared_ptr<ProducerCommand>(cmd);
      } else {
        cmd->onSend(0, name);
        cmd->onSuccess(0, name, std::string());
      }
    }
  }

public:
  KafkaThread(const std::string &name, const string &brokers,
              const std::string &topic, shared_ptr<ProducerQue> que,
              shared_ptr<BlockingQueuePtr<std::string>> statusQue)
      : KafkaProducer(
            topic, brokers,
            [this](KafkaProducer::MessageId msgId, const std::string &data,
                   const std::string &errorMessage) {
              onError(msgId, data, errorMessage);
            },
            [this](KafkaProducer::MessageId msgId, const std::string &data) {
              onSuccess(msgId, data);
            }),
        que(que), statusQue(statusQue), name(name) {}

  void onSuccess(KafkaProducer::MessageId msgId, const std::string &data) {
    shared_ptr<ProducerCommand> cmd = getWaitingMessage(msgId);
    if (cmd)
      cmd->onSuccess(msgId, name, data);
  }

  void onError(KafkaProducer::MessageId msgId, const std::string &data,
               const std::string &errorMessage) {
    shared_ptr<ProducerCommand> cmd = getWaitingMessage(msgId);
    if (cmd)
      cmd->onError(msgId, name, data, errorMessage);
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
      catchup(2000);
  }

  void run() {
    bool running = true;
    while (running) {
      try {
        catchup(0);
        send(que->timedGet(std::chrono::milliseconds(500), false));
      } catch (const miutil::concurrent::QueueSuspended &ex) {
        running = false;
      } catch (const std::exception &ex) {
      }
    }
    drainQue();
  }

  static void start(const std::string &name, const std::string &brokers,
                    const std::string &topic, ProducerQuePtr que,
                    shared_ptr<BlockingQueuePtr<std::string>> statusQue) {
    try {
      KafkaThread kafka(name, brokers, topic, que, statusQue);
      statusQue->add(new std::string("<STARTED>"));
      kafka.run();
      statusQue->add(new std::string("<EXIT>"));
    } catch (const std::exception &ex) {
      // Constructor failure
      statusQue->add(new std::string(ex.what()));
    }
  }
};
} // namespace

KafkaProducerThread::KafkaProducerThread(const std::string &name,
                                         unsigned int queueSize)
    : statusQue(new miutil::concurrent::BlockingQueuePtr<std::string>()),
      name(getThreadId(name)), queue(new ProducerQue(queueSize)) {}

KafkaProducerThread::~KafkaProducerThread() {
  if (kafkaThread.joinable())
    kafkaThread.detach();
}
void KafkaProducerThread::setName(const std::string &name_) { name = name_; }

void KafkaProducerThread::send(ProducerCommand *cmd) {
  try {
    queue->add(cmd);
  } catch (const std::exception &ex) {
  }
}

void KafkaProducerThread::start(const std::string &brokers,
                                const std::string &topic) {
  kafkaThread =
      thread(KafkaThread::start, name, brokers, topic, queue, statusQue);
  string *res = statusQue->get();
  if (*res == "<STARTED>") {
    LOGINFO("KafkaProducerThread: " << name << ": started.");
    return;
  } else {
    throw std::runtime_error(name + ": " + *res);
  }
}

void KafkaProducerThread::shutdown() {
  try {
    queue->suspend();
  } catch (...) {
  }
}

void KafkaProducerThread::join(
    const std::chrono::high_resolution_clock::duration &timeout) {
  try {
    string *res = statusQue->timedGet(timeout, true);
    if (*res == "<EXIT>") {
      kafkaThread.join();
      LOGINFO("KafkaProducerThread: " << name << ": stopped.");
    } else {
      LOGERROR(name << ": join: Unexpected return from thread <" + *res + ">.");
    }
  } catch (const miutil::concurrent::QueueTimeout &ex) {
    if (kafkaThread.joinable())
      kafkaThread.detach();
  }
}

} //  namespace service
} //  namespace kvalobs
