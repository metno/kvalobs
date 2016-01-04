/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: DataSrcImpl.h,v 1.4.2.2 2007/09/27 09:02:16 paule Exp $

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

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/thread.hpp>
#include <libmiamqpcpp/amqpcxx.h>
#include "DataSrcAmqp.h"

namespace pt = boost::posix_time;

DataSrcAmqp::DataSrcAmqp(const DataSrcApp &app)
    : app(app) {
}

void DataSrcAmqp::receiveData(amqp::Channel *channel) {
  while (!app.inShutdown()) {
  }
}

void DataSrcAmqp::runChannels(amqp::Connection *con) {
  amqp::Channel *channel;
  bool doReturn = false;

  while (!app.inShutdown()) {
    try {
      channel = con->createChannel();
      receiveData(channel);
    } catch (const amqp::ChannelError &ex) {
      //The channel has closed, reopen.
      continue;
    } catch (amqp::AMQPException &ex) {
      doReturn = true;
    } catch (...) {
      doReturn = true;
    }

    if (doReturn) {
      try {
        channel->close();
      } catch (...) {
      }

      delete channel;
      return;
    }
  }
}

void DataSrcAmqp::operator()() {
  amqp::ConnectionFactory factory;
  ;
  amqp::Connection *con;
  pt::ptime sleepTo;
  pt::ptime now;
  pt::ptime nextTryToConnect(pt::second_clock::universal_time());

  factory.setPasswd(app.getAmqpPasswd());
  factory.setDefaultUrl(app.getAmqpUrl());

  while (app.inShutdown()) {
    now = pt::second_clock::universal_time();

    if (now < nextTryToConnect) {
      boost::thread::sleep(now + pt::seconds(1));
      continue;
    }

    try {
      con = factory.newConnection();
    } catch (const amqp::AMQPException &ex) {
      nextTryToConnect = pt::second_clock::universal_time() + pt::seconds(5);
      continue;
    }

    con = runChannels(con);

    if (con) {
      con->close();
      con = 0;
    }
  }

}
;
