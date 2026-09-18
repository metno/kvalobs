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

#ifndef __PGCONSUMER_H__
#define __PGCONSUMER_H__

#include "Consumer.h"
#include "pgqueue/pgqueue.h"
#include <functional>
#include <list>
#include <memory>
#include <string>
#include <vector>

namespace kvalobs {
namespace subscribe {

/**
 * Base class for subscribing to data from kvalobs.
 *
 * Subclasses wil be fed data from the specified stream, and the abstract
 * methods data(...) and error(...) will be called as appropriate by this
 * class' event loop.
 */
class PgConsumer : public kvalobs::subscribe::Consumer {
public:
  // topic must be a valid kvalobs topic string.
  //  "kvalobs.<env>.<checked|raw>" where env must start with one of
  //  "production", "staging" or "development" env may be "productionN", where N
  //  is a number indicating a specific production environment. Same for
  //  "stagingN" and "developmentN" Example: "kvalobs.production.checked",
  //  "kvalobs.staging.checked" or "kvalobs.production.raw"
  // throws std::invalid_argument if no connections are provided or topic is
  // invalid throws std::runtime_error if PgCluster instance creation fails
  PgConsumer(const std::vector<std::string> &connections,
             const std::string &topic, const std::string &groupId,
             kvalobs::subscribe::ConsumerDataHandler *handler,
             int pollSize = 100);
  PgConsumer(const std::vector<std::string> &connections,
             const std::string &topic, const std::string &groupId,
             int pollSize = 100);

  virtual ~PgConsumer();

  std::string getTopicLowpri() const;
  /**
   * Process one message, waiting maximum for the given time if no messages are
   * available.
   *
   * Must call handleData(...) or handleError(...) as appropriate.
   *
   */
  virtual void runOnce(unsigned timeoutInMilliSeconds) override;

  /**
   * Has stop() been called?
   */
  virtual bool stopping() const override;

  /**
   * Stop this consumer.
   */
  virtual void stop() override;

protected:
  std::unique_ptr<PgCluster> pgCluster_;
  std::unique_ptr<PgMessaging> pgMessaging_;
  int backOffInSeconds_;
  std::string groupId_;
  bool stopping_;
  int pollSize_;

  void init(const std::vector<std::string> &connections,
            const std::string &topic, const std::string &groupId);
};

} // namespace subscribe
} // namespace kvalobs

#endif /* __CONSUMER_H__ */
