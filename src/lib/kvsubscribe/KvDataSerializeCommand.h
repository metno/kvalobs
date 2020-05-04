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
 * kvalobs.domain.checked queue. The class is bugg never use it.
 * Use KvDataSerializeCommandExt.
 */
class KvDataSerializeCommandPrivat;

class KvDataSerializeCommand : public ProducerCommand {
  KvDataSerializeCommand();
  KvDataSerializeCommand(const KvDataSerializeCommand &);
  KvDataSerializeCommand(const KvDataSerializeCommand &&);
  KvDataSerializeCommand& operator=(const KvDataSerializeCommand &);

  KvDataSerializeCommandPrivat *pPrivate_;
  
 public:
  
  /**
   * The producer parameter sets the producer tag in the kvxml. Set it to 
   * something that identify the program/system, ex 'kvinput' for kvDataInputd.
   */
  explicit KvDataSerializeCommand(const std::list<kvalobs::kvData> &dataList, const std::string &producer);
  explicit KvDataSerializeCommand(const std::list<kvalobs::kvData> &&dataList, const std::string &producer);
  explicit KvDataSerializeCommand(const kvalobs::kvData &dataElem, const std::string &producer);
  virtual ~KvDataSerializeCommand();

  virtual const char *getData(unsigned int *size) const override;
};




}  // namespace service
}  // namespace kvalobs

#endif  // SRC_LIB_KVSUBSCRIBE_KVDATASERIALIZECOMMAND_H_
