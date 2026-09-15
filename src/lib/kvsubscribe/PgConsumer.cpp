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
#include "PgConsumer.h"
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <thread>

namespace kvalobs {
namespace subscribe {

namespace {

bool consumeRaw(PgMessaging *queue, const std::string &topic,
                const std::string &groupId, ConsumerDataHandler *handler,
                int pollSize = 100) {
  const std::vector<Message> messages = queue->poll(groupId, topic, pollSize);
  if (messages.empty()) {
    return false;
  }

  for (const Message &message : messages) {
    handler->data(message.data.c_str(), message.data.size());
  }
  queue->commit_offset(groupId, topic, messages.back().id);
  std::cout << "Committed offset " << messages.back().id << '\n';
  return true;
}

bool consumeChecked(PgMessaging *queue, const std::string &topic,
                    const std::string &groupId, ConsumerDataHandler *handler,
                    int pollSize = 100) {
  bool lowPri;
  std::string topicLowpri(topic + ".lowpri");
  std::vector<Message> messages;
  messages.clear();
  lowPri = false;
  messages = queue->poll(groupId, topic, pollSize);
  if (messages.empty()) {
    lowPri = true;
    messages = queue->poll(groupId, topicLowpri, pollSize);
  }
  if (messages.empty()) {
    return false;
  }

  for (const Message &message : messages) {
    handler->data(message.data.c_str(), message.data.size());
  }

  if (lowPri) {
    queue->commit_offset(groupId, topicLowpri, messages.back().id);
  } else {
    queue->commit_offset(groupId, topic, messages.back().id);
  }
  return true;
}

bool consume(PgMessaging *queue, const std::string &topic,
             const std::string &groupId, ConsumerDataHandler *handler,
             int pollSize = 100) {
  if (topic.find(".checked") != std::string::npos)
    return consumeChecked(queue, topic, groupId, handler, pollSize);
  return consumeRaw(queue, topic, groupId, handler, pollSize);
}

std::vector<std::string> splitTopic(const std::string &topic) {
  std::vector<std::string> parts;
  std::stringstream stream(topic);
  std::string part;

  while (std::getline(stream, part, '.')) {
    parts.push_back(part);
  }

  return parts;
}

PgCluster::Environment decodeTopic(const std::string &topic) {
  auto parts = splitTopic(topic);

  if (parts.empty() || parts.size() < 3 || parts[0] != "kvalobs") {
    throw std::invalid_argument(
        "Invalid topic string: " + topic +
        ", expected at least 3 parts 'kvalobs.<env>.<checked|*>'");
  }

  auto type = parts[2];
  if (type != "checked" && type != "raw") {
    throw std::invalid_argument("Invalid topic type: " + type +
                                ", expected 'checked' or 'raw'");
  }

  auto domain = parts[1];
  if (domain.find("production") != std::string::npos)
    return PgCluster::production;

  if (domain.find("staging") != std::string::npos)
    return PgCluster::staging;

  return PgCluster::development;
}

} // namespace

PgConsumer::PgConsumer(const std::vector<std::string> &connections,
                       const std::string &topic, const std::string &groupId,
                       ConsumerDataHandler *handler, int pollSize)
    : Consumer(topic, groupId, handler), stopping_(false), pollSize_(pollSize) {
  init(connections, topic, groupId);
}

PgConsumer::PgConsumer(const std::vector<std::string> &connections,
                       const std::string &topic, const std::string &groupId,
                       int pollSize)
    : Consumer(topic, groupId), stopping_(false), pollSize_(pollSize) {
  init(connections, topic, groupId);
}

PgConsumer::~PgConsumer() { Consumer::remove(this); }

void PgConsumer::init(const std::vector<std::string> &connections,
                      const std::string &topic, const std::string &groupId) {
  backOffInSeconds_ = 1; // Initialize back-off duration in seconds
  PgCluster::Environment env = decodeTopic(topic);

  if (connections.empty())
    throw std::invalid_argument("No 'pgqueue' database connections provided");

  pgCluster_ = std::make_unique<PgCluster>(connections, env, groupId);

  if (!pgCluster_)
    throw std::runtime_error("Failed to create PgCluster instance");
}

std::string PgConsumer::getTopicLowpri() const {
  std::string topic = getTopic();
  if (topic.find(".checked") != std::string::npos)
    return topic + ".lowpri";
  return "";
}

void PgConsumer::runOnce(unsigned timeoutInMilliSeconds) {
  try {
    if (!pgMessaging_)
      pgMessaging_ = std::make_unique<PgMessaging>(*pgCluster_.get());

    if (!consume(pgMessaging_.get(), getTopic(), groupId_, getHandler(),
                 pollSize_)) {
      std::this_thread::sleep_for(
          std::chrono::milliseconds(timeoutInMilliSeconds));
      // Handle error if needed
    }
    backOffInSeconds_ = 0;
  } catch (const std::exception &e) {
    handleError(
        0,
        std::format(
            "Exception caught in PgConsumer::runOnce: {}. backOff_={} seconds",
            e.what(), backOffInSeconds_));
    std::this_thread::sleep_for(std::chrono::seconds(backOffInSeconds_));
    backOffInSeconds_ =
        std::min(backOffInSeconds_ * 2,
                 60); // Exponential back-off with a maximum of 60 seconds
    pgCluster_->reprobe();
    pgMessaging_.reset();
  }
}

bool PgConsumer::stopping() const { return stopping_; }

void PgConsumer::stop() { stopping_ = true; }

} // namespace subscribe
} // namespace kvalobs
