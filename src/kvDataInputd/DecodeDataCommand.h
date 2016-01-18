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
#ifndef SRC_KVDATAINPUTD_DECODEDATACOMMAND_H_
#define SRC_KVDATAINPUTD_DECODEDATACOMMAND_H_

#include <memory>
#include <list>
#include "decodeutility/kvalobsdata.h"

#include "kvDataInputd/ProducerCommand.h"

/**
 * \addtogroup kvDatainputd
 * @{
 */

/**
 * \brief This is the message that is passed to the
 *kafka producers.
 */

class DecodedDataCommand : public ProducerCommand {
  DecodedDataCommand();
  DecodedDataCommand(const DecodedDataCommand &);
  DecodedDataCommand& operator=(const DecodedDataCommand &);

  std::shared_ptr<std::list<kvalobs::serialize::KvalobsData>> data;

 public:
  DecodedDataCommand(
      std::shared_ptr<std::list<kvalobs::serialize::KvalobsData>> rawData);

  virtual kvalobs::subscribe::KafkaProducer::MessageId send(kvalobs::subscribe::KafkaProducer &producer);
};

/** @} */

#endif  // SRC_KVDATAINPUTD_DECODEDATACOMMAND_H_
