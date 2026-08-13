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

#ifndef SRC_LIB_KVSUBSCRIBE_DATASUBSCRIBER_H_
#define SRC_LIB_KVSUBSCRIBE_DATASUBSCRIBER_H_

#include "Consumer.h"
#include <functional>

namespace kvalobs {
namespace serialize {
class KvalobsData;
}

namespace subscribe {

/**
 * Subscriber for new data. Kvalobs' qabase application will
 * send out a list of all new data whenever it has finished processed it, even
 * if it has decided that no checks should be run. This class may be used to
 * pick up such notifications.
 *
 * Valid messages are handled through the provided handling function, while
 * errors are merely logged.
 *
 * This class inherits from both KafkaConsumer (for Kafka transport) and
 * DataHandler (for data processing logic), bridging the transport layer
 * with the data handling layer.
 */

class DataSubscriber : public Consumer {
public:
  /**
   * New data handling function
   */
  typedef std::function<void(const ::kvalobs::serialize::KvalobsData &)>
      Handler;

  DataSubscriber(Handler handler, Consumer *consumer);
  
  // Consumer interface implementation
  virtual void run() override;
  virtual bool stopping() const override;
  virtual void stop() override;

  /**
   * The identifying string for this message stream
   */
  static std::string topic(const std::string &domain);
  static void
  setDebugWriter(std::function<void(const std::string &message,
                                    const serialize::KvalobsData &d)>
                     func);

  static void resetDebugWrite();

  static std::function<void(const std::string &message,
                                    const serialize::KvalobsData &d)>
      getDebugWriter() { return debugWriter; }
  
protected:
  // Consumer interface implementation (protected methods)
  virtual void runOnce(unsigned timeoutInMilliSeconds) override;

private:
  static std::function<void(const std::string &message,
                            const serialize::KvalobsData &d)>
      debugWriter;
  Handler handler_;
  Consumer *consumer_;
};

} /* namespace subscribe */
} /* namespace kvalobs */

#endif /* SRC_LIB_KVSUBSCRIBE_DATASUBSCRIBER_H_ */
