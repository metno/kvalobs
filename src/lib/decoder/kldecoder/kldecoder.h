/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: kldecoder.h,v 1.4.2.5 2007/09/27 09:02:29 paule Exp $                                                       

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
#ifndef __kvalobs_decoder_kldecoder_h__
#define __kvalobs_decoder_kldecoder_h__

#include <boost/date_time/posix_time/ptime.hpp>
#include <kvalobs/kvData.h>
#include <list>
#include <kvalobs/kvStation.h>
#include <decoderbase/decoder.h>
#include "decodeutility/KvDataContainer.h"
#include <vector>
#include "DataDecode.h"

namespace kvalobs {
namespace decoder {
namespace kldecoder {

/**
 * \addtogroup kldecode
 *
 * @{
 */

typedef boost::mutex::scoped_lock Lock;

/**
 * \brief implements the interface  DecoderBase.
 *
 * Variables that can be defined for the driver in the section
 * 'kvDataInputd.KlDataDecoder' for the driver in the configuration file
 * kvalobs.conf.
 *
 *  - set_useinfo7, valid values true or false.
 *    This value set how a missing received_time in obsType shall be
 *    interpreted. If false useinfo(7) is NOT set. If true useinfo(7)
 *    is set with a received_time set to current date and time.
 *
 * <pre>
 * Dataformat for message.
 *
 * obsType: kldata/nationalnr=<nummer>/type=<typeid>/add=<true¦false>/received_time=<ISO time>
 * obs:
 *   <pc1>[(<sensor>,<level>)],...,pcN[(<sensor>,<level>)]
 *   YYYYMMDDhhmmss,<pc1_value[(<cinfo>,<uinfo>)]>,...,<pcN_value[(<cinfo>,<uinfo>)]>
 *   YYYYMMDDhhmmss,<pc1_value[(<cinfo>,<uinfo>)]>,....,<pcN_value[(<cinfo>,<uinfo>)]>
 *   ....
 *   YYYYMMDDhhmmss,<pc1_value[(<cinfo>,<uinfo>)]>,....,<pcN_value[(<cinfo>,<uinfo>)]>
 *
 *  pc - paramcode, the name of the parameter. An underscore indicate that
 *                  this is a code value. Suported pc that can have a code
 *                  value is: HL and VV. The value will be converted to meter.
 *  If sensor or level is not specified. The default would apply. If both shall
 *  take the default value, the paranteces can be left out.
 *
 *  cinfo - controlinfo
 *  uinfo - useinfo
 *  </pre>
 *
 */
class KlDecoder : public DecoderBase {
  KlDecoder();
  KlDecoder(const KlDecoder &);
  KlDecoder& operator=(const KlDecoder &);

 protected:

  void decodeObsType(const std::string &obstype);

  kvalobs::decoder::DecoderBase::DecodeResult
  rejected(const std::string &msg, const std::string &logid,
           std::string &msgToSender, bool includeObs = true);

  bits::DataDecoder datadecoder;
  std::string logid;
  int typeID;
  int stationID;
  std::string stationidIn;
  std::string redirectedFrom;
  bool onlyInsertOrUpdate;
  boost::posix_time::ptime receivedTime;

 public:
  KlDecoder(dnmi::db::Connection &con, const ParamList &params,
            const std::list<kvalobs::kvTypes> &typeList,
            const std::string &obsType, const std::string &obs, int decoderId =
                -1);

  virtual ~KlDecoder();

  bool getSetUsinfo7();
  bool getOnlyInsertOrUpdate() const;
  long getStationId(std::string &msg) const;
  long getTypeId(std::string &msg) const;
  bool do302(int stationid, int typeId, 
             decodeutility::KvDataContainer::DataByObstime &dataIn, 
             decodeutility::KvDataContainer::TextDataByObstime &textDataIn, 
             std::map<boost::posix_time::ptime, int> &observations,
             const std::string &logid, std::string &msgToSender);

  DecodeResult insertDataInDb(kvalobs::serialize::KvalobsData *theData,
                              int stationid, int typeId,
                              const std::string &logid,
                              std::string &msgToSender);

  virtual std::string name() const;

  DecodeResult doExecute(std::string &msg);
  virtual DecodeResult execute(std::string &msg);
};

/** @} */
}
}
}

#endif
