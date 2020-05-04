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
#include <mutex>
#include "lib/decodeutility/kvalobsdataserializer.h"
#include "lib/kvsubscribe/KvDataSerializeCommand.h"

namespace kvalobs {
namespace service {

class KvDataSerializeCommandPrivat {
public:
  KvDataSerializeCommandPrivat(const std::list<kvalobs::kvData> &data_,const std::string &prod)
    :data(data_),producer(prod){
  }

  KvDataSerializeCommandPrivat(const std::list<kvalobs::kvData> &&data_,const std::string &prod)
    :data(data_),producer(prod){
  }

  KvDataSerializeCommandPrivat(const std::string &prod)
    :producer(prod){
  }
  std::list<kvalobs::kvData> data;
  std::string producer;
  std::string xml;

};

  
  
KvDataSerializeCommand::
KvDataSerializeCommand(const std::list<kvalobs::kvData> &dataList, const std::string &producer)
  :pPrivate_(new KvDataSerializeCommandPrivat(dataList, producer))
{
}

KvDataSerializeCommand::
KvDataSerializeCommand(const std::list<kvalobs::kvData> &&dataList, const std::string &producer)
  : pPrivate_(new KvDataSerializeCommandPrivat(dataList,producer))
{

}
KvDataSerializeCommand::
KvDataSerializeCommand(const kvalobs::kvData &dataElem, const std::string &producer)
  : pPrivate_(new KvDataSerializeCommandPrivat(producer)){
    pPrivate_->data.push_back(dataElem);
}

KvDataSerializeCommand::
~KvDataSerializeCommand(){
  delete pPrivate_;
}

const char *KvDataSerializeCommand::getData(unsigned int *size) const
{ 
  pPrivate_->xml = kvalobs::serialize::KvalobsDataSerializer::serialize(pPrivate_->data, pPrivate_->producer);
  *size = pPrivate_->xml.size();
  return pPrivate_->xml.data();
}





}  // namespace service
}  // namespace kvalobs
