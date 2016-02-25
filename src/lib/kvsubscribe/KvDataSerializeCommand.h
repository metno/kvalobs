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
#ifndef SRC_LIB_KVSUBSCRIBE_KVDATASERIALIZECOMMAND_H_
#define SRC_LIB_KVSUBSCRIBE_KVDATASERIALIZECOMMAND_H_

#include <memory>
#include <list>
#include "decodeutility/kvalobsdata.h"
#include "kvalobs/kvData.h"
#include "kvsubscribe/ProducerCommand.h"

namespace kvalobs {
namespace service {

/**
 * \brief This is the message that can be passed to the
 * kvalobs.domain.checked queue.
 */

class KvDataSerializeCommand : public ProducerCommand {
  KvDataSerializeCommand();
  KvDataSerializeCommand(const KvDataSerializeCommand &);
  KvDataSerializeCommand& operator=(const KvDataSerializeCommand &);

  std::list<kvalobs::kvData> data;

 public:
  explicit KvDataSerializeCommand(const std::list<kvalobs::kvData> &dataList);
  explicit KvDataSerializeCommand(const std::list<kvalobs::kvData> &&dataList);
  explicit KvDataSerializeCommand(const kvalobs::kvData &dataElem);

  virtual const char *getData(unsigned int *size) const;

};

}  // namespace service
}  // namespace kvalobs

#endif  // SRC_LIB_KVSUBSCRIBE_KVDATASERIALIZECOMMAND_H_
