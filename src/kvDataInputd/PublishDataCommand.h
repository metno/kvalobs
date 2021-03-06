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
#ifndef SRC_KVDATAINPUTD_PUBLISHDATACOMMAND_H_
#define SRC_KVDATAINPUTD_PUBLISHDATACOMMAND_H_

#include <string>
#include <memory>
#include <set>
#include "lib/decodeutility/kvalobsdata.h"
#include "lib/kvsubscribe/ProducerCommand.h"
#include "lib/kvalobs/kvStationInfo.h"

/**
 * \addtogroup kvDatainputd
 * @{
 */

/**
 * \brief This is the message that is passed to the
 *kafka producers.
 */

class PublishDataCommand : public kvalobs::service::ProducerCommand {
  PublishDataCommand();
  PublishDataCommand(const PublishDataCommand &);
  PublishDataCommand& operator=(const PublishDataCommand &);

  std::string data;
  std::set<kvalobs::kvStationInfo> summary;
  std::string toLog;

 public:
  explicit PublishDataCommand(const kvalobs::serialize::KvalobsData &pubData);

  const char *getData(unsigned int *size) const;
  virtual void onSend(kvalobs::subscribe::KafkaProducer::MessageId msgId, const std::string &threadName);
  virtual void onSuccess(kvalobs::subscribe::KafkaProducer::MessageId msgId, const std::string &threadName, const std::string &data);
  virtual void onError(kvalobs::subscribe::KafkaProducer::MessageId msgId, const std::string &threadName, const std::string & data,
                       const std::string & errorMessage);
};

/** @} */

#endif  // SRC_KVDATAINPUTD_PUBLISHDATACOMMAND_H_
