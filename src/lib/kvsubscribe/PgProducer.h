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

#ifndef __KVSUBSCRIBE_PGPRODUCER_H__
#define __KVSUBSCRIBE_PGPRODUCER_H__

#include "messageid.h"
#include "Producer.h"
#include "pgqueue/pgqueue.h"
#include <cstdint>
#include <functional>
#include <memory>
#include <ostream>
#include <string>
#include <vector>
#include <queue>
#include <mutex>

class PgMessaging;

namespace kvalobs {
namespace subscribe {

class PgProducer : public Producer {
public:
  /**
   * Create a PgProducer that publishes to a PostgreSQL-backed queue.
   *
   * @param topic The topic (queue name) to publish to
   * @param connections Vector of PostgreSQL connection strings (host, port, dbname, user, etc.)
   * @param onFailedDelivery Callback invoked on delivery failure
   * @param onSuccessfulDelivery Callback invoked on successful delivery
   */
  explicit PgProducer(
      const std::string &topic,
      const std::vector<std::string> &connections,
      PgCluster::Environment env,
      const std::string &appName,
      ErrorHandler onFailedDelivery = [](MessageId, const std::string &,
                                         const std::string &) {},
      SuccessHandler onSuccessfulDelivery = [](MessageId,
                                               const std::string &) {});


   explicit PgProducer(
      const std::string &topic,
      PgCluster *cluster,
      ErrorHandler onFailedDelivery = [](MessageId, const std::string &,
                                         const std::string &) {},
      SuccessHandler onSuccessfulDelivery = [](MessageId,
                                               const std::string &) {});


  ~PgProducer();

  /**
   * Asynchronous sending of data. Remember to call catchup() at some point
   * to check results of send.
   *
   * On error, may either throw an exception right away, or deliver an
   * error report via the errorHandler callback after having called catchup,
   * or destroying this object.
   *
   * @throws exception if it fails right away
   *
   * @return a message id, that will be available in this object's constructor's
   *         onFailedDelivery and onSuccessfulDelivery functions
   */
  MessageId send(const std::string &data) override;

  MessageId send(const char *data, unsigned length) override;

  /**
   * Process all awaiting delivery reports.
   *
   * @param timeout Maximum time to wait for delivery report to become
   * available, in milliseconds (currently unused, for Kafka API compatibility)
   */
  void catchup(unsigned timeout = 0) override;

private:
  std::unique_ptr<PgCluster> cluster_;
  std::unique_ptr<PgMessaging> messaging_;
  MessageId messageId_;
  mutable std::mutex mu_;

  // Pending deliveries for deferred callback processing
  struct PendingDelivery {
    MessageId id;
    std::string data;
    bool success;
    std::string error;
  };
  std::queue<PendingDelivery> pendingDeliveries_;
};

} // namespace subscribe
} // namespace kvalobs

#endif /* KVSUBSCRIBE_PGPRODUCER_H_ */
