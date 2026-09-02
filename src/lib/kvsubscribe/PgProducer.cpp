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

#include "PgProducer.h"
#include "lib/pgqueue/pgqueue.h"
#include <iostream>
#include <stdexcept>
#include <sstream>

namespace kvalobs {
namespace subscribe {

PgProducer::PgProducer(const std::string &topic,
                       const std::vector<std::string> &connections,
                       PgCluster::Environment env,
                       const std::string &appName,
                       ErrorHandler onFailedDelivery,
                       SuccessHandler onSuccessfulDelivery)
    : Producer(topic, onFailedDelivery, onSuccessfulDelivery),
      messageId_(0) {
  
  if (connections.empty()) {
    throw std::logic_error("Empty PostgreSQL connection list");
  }

  try {
    // Create a PgCluster to handle primary/replica selection
    cluster_ = std::make_unique<PgCluster>(
        connections,
        env,  // environment
        appName  // app name for connection identification
    );

    // Create PgMessaging instance for publishing
    messaging_ = std::make_unique<PgMessaging>(*cluster_);

    // Create the topic if it doesn't exist
    try {
      messaging_->create_topic(topic);
    } catch (const std::exception &e) {
      // Topic may already exist, which is fine
      // Other errors will be caught on the first publish attempt
    }

  } catch (const std::exception &e) {
    throw std::runtime_error(
        std::string("Failed to initialize PgProducer: ") + e.what());
  }
}

PgProducer::PgProducer(
      const std::string &topic,
      PgCluster *cluster,
      ErrorHandler onFailedDelivery,
      SuccessHandler onSuccessfulDelivery)
    : Producer(topic, onFailedDelivery, onSuccessfulDelivery),
      cluster_(cluster),
      messageId_(0) {

  if (!cluster_) {
    throw std::logic_error("PgCluster pointer is null");
  }

  try {
    // Create PgMessaging instance for publishing
    messaging_ = std::make_unique<PgMessaging>(*cluster_);
  } catch (const std::exception &e) {
    throw std::runtime_error(
        std::string("Failed to initialize PgProducer with existing cluster: ") + e.what());
  }
}


PgProducer::~PgProducer() { 
  catchup();
}

MessageId PgProducer::send(const std::string &data) {
  return send(data.c_str(), data.size());
}

MessageId PgProducer::send(const char *data, unsigned length) {
  std::lock_guard<std::mutex> lock(mu_);

  MessageId id = messageId_++;
  std::string dataStr(data, length);

  try {
    // Publish to the PostgreSQL queue
    long long pgMsgId = messaging_->publish(topic_, dataStr);

    // Queue a successful delivery notification
    PendingDelivery delivery;
    delivery.id = id;
    delivery.data = dataStr;
    delivery.success = true;
    pendingDeliveries_.push(delivery);

    return id;

  } catch (const std::exception &e) {
    // Queue a failed delivery notification
    PendingDelivery delivery;
    delivery.id = id;
    delivery.data = dataStr;
    delivery.success = false;
    delivery.error = e.what();
    pendingDeliveries_.push(delivery);

    return id;  // Return the ID anyway; callback will report the error
  }
}

void PgProducer::catchup(unsigned timeout) {
  std::lock_guard<std::mutex> lock(mu_);

  // Process all pending deliveries
  while (!pendingDeliveries_.empty()) {
    PendingDelivery delivery = pendingDeliveries_.front();
    pendingDeliveries_.pop();

    if (delivery.success) {
      // Call the success handler
      if (onSuccessfulDelivery_) {
        onSuccessfulDelivery_(delivery.id, delivery.data);
      }
    } else {
      // Call the error handler
      if (onFailedDelivery_) {
        onFailedDelivery_(delivery.id, delivery.data, delivery.error);
      }
    }
  }
}

} // namespace subscribe
} // namespace kvalobs

