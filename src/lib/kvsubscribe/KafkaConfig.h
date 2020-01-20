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

#ifndef __KAFKACONFIG_H__
#define __KAFKACONFIG_H__


#include <iostream>
#include <string>



namespace kvalobs {
namespace subscribe {

class KafkaConfig {
  public:
    KafkaConfig():brokers("localhost"), requestRequiredAcks(-1), requestTimeoutMs(5000){}

    std::string brokers;
    std::string topic;

   /** Number of isr (in sync replika) akcs required before 
    * we accept the message ad delivered.
    * -1 - all
    *  0 - None
    *  >0 the number of isr that has acked.
    * Default: -1
    */
    int requestRequiredAcks;
 
  /**
   * Timeout before we give up waiting on Acks. Reqiure kafkaAcks_ != 0 
   * Default: 5000 ms
   */
   int requestTimeoutMs;

  friend std::ostream& operator<<(std::ostream &o, const KafkaConfig &c); 
};
}

}

#endif