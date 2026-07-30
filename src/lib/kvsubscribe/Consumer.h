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

#ifndef __CONSUMER_H__
#define __CONSUMER_H__

#include <functional>
#include <list>
#include <memory>
#include <string>
#include <vector>

namespace kvalobs {
namespace subscribe {

class ConsumerDataHandler {
public:
  virtual ~ConsumerDataHandler() {}
  /**
   * Process incoming data
   */
  virtual void data(const char *msg, unsigned length) = 0;

  /**
   * Handle errors on message arrival.
   */
  virtual void error(int code, const std::string &msg) = 0;
};

/**
 * Base class for subscribing to data from kvalobs.
 *
 * Subclasses wil be fed data from the specified stream, and the abstract
 * methods data(...) and error(...) will be called as appropriate by this
 * class' event loop.
 */
class Consumer {
public:
  Consumer(const std::string &topic, ConsumerDataHandler *handler = nullptr);
  Consumer(const std::string &topic);

  virtual ~Consumer();

  /**
   * Set the data handler for this consumer.
   *
   * @param handler The new data handler to use
   * @return The previous data handler
   */
  ConsumerDataHandler *setHandler(ConsumerDataHandler *handler);
  ConsumerDataHandler *getHandler() const;

  std::string getTopic() const;

  /**
   * Run until stop() has been called, processing events, calling data(...)
   * and error(...) as appropriate.
   *
   * It may make sense to run this in a std::thread
   */
  virtual void run();

  /**
   * Has stop() been called?
   */
  bool stopping() const;

  /**
   * Stop this consumer.
   */
  virtual void stop() = 0;

  /**
   * call stop() an all consumers
   */
  static void stopAll();

protected:
  /**
   * Process one message, waiting maximum for the given time if no messages are
   * available.
   *
   * Must call handleData(...) or handleError(...) as appropriate.
   *
   */
  virtual void runOnce(unsigned timeoutInMilliSeconds) = 0;

  void handleData(const char *msg, unsigned length);
  void handleError(int code, const std::string &msg);

private:
  bool stopping_;
  ConsumerDataHandler *handler_;
  std::string topic_;
  static std::list<Consumer *> allConsumers_;
};

} // namespace subscribe
} // namespace kvalobs

#endif /* __CONSUMER_H__ */
