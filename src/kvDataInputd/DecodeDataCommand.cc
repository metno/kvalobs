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
#include <string>
#include "lib/decodeutility/kvalobsdataserializer.h"
#include "kvDataInputd/DecodeDataCommand.h"

DecodedDataCommand::DecodedDataCommand(
    std::shared_ptr<std::list<kvalobs::serialize::KvalobsData>> data)
    : data(data) {
}

kvalobs::subscribe::KafkaProducer::MessageId DecodedDataCommand::send(kvalobs::subscribe::KafkaProducer &producer) {
  std::string xml;
  for (auto &&kvdata : *data) {
    xml = kvalobs::serialize::KvalobsDataSerializer::serialize(kvdata);
    // TODO(borgem): Send data to the decoded data kafka queue.
  }
  return 0;
}
