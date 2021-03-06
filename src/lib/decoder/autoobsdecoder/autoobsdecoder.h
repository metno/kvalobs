/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: autoobsdecoder.h,v 1.12.2.2 2007/09/27 09:02:23 paule Exp $                                                       

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
#ifndef __kvalobs_decoder_autoobsdecoder_h__
#define __kvalobs_decoder_autoobsdecoder_h__

#include <decoderbase/decoder.h>
#include <kvalobs/kvTypes.h>
#include <list>

namespace kvalobs {
namespace decoder {
namespace autoobs {
/**
 * \addtogroup decodeautoobs.
 *
 * @{
 */

/**
 * \brief implement the interface DecoderBase.
 */
class AutoObsDecoder : public DecoderBase {
  AutoObsDecoder();
  AutoObsDecoder(const AutoObsDecoder &);
  AutoObsDecoder& operator=(const AutoObsDecoder &);

  long getStationId(std::string &msg);
  long getTypeId(std::string &msg);

  std::string getMetaSaSdEmEi(int stationid, int typeid_,
                              const boost::posix_time::ptime &obstime);

  char checkObservationTime(int typeId, boost::posix_time::ptime tbt,
                            boost::posix_time::ptime obt);
  DecodeResult rejected(const kvalobs::kvRejectdecode &rejectEntry,
                        const std::string &logmsg,
                        const std::string &logid = "",
                        const std::string &idlogMsg = "");

  boost::posix_time::ptime firstObsTime;  //Used by checkObservationTime.
  char checkRet;     //Used by checkObservationTime.
  kvalobs::decoder::ObsPgmParamInfo obsPgm;
  bool warnings;
  std::string logid;

 public:
  AutoObsDecoder(dnmi::db::Connection &con, const ParamList &params,
                 const std::list<kvalobs::kvTypes> &typeList,
                 const std::string &obsType, const std::string &obs,
                 int decoderId = -1);

  virtual ~AutoObsDecoder();

  virtual std::string name() const;

  virtual DecodeResult execute(std::string &msg);
};

/** @} */
}
}
}

#endif
