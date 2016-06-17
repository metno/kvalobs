/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: sms2.h,v 1.5.2.2 2007/09/27 09:02:24 paule Exp $                                                       

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

#ifndef __kvalobs_decoder_comobsdecoder_sms2_h__
#define __kvalobs_decoder_comobsdecoder_sms2_h__

#include <miutil/commastring.h>
#include "decodeddata.h"
#include "smsbase.h"

namespace kvalobs{
namespace decoder{
namespace comobsdecoder{

/**
* \addtogroup comobsdecode
*
* @{
*/

class Sms2 : public SmsBase{
   Sms2(const Sms2 &);
   Sms2& operator=(const Sms2 &);

protected:
   int decodeV1V2V3(const std::string &what,
                    std::string  &v1,
                    std::string  &v2,
                    std::string  &v3,
                    std::string  &intensitet,
                    std::ostream &err);

   void addV1V2V3( kvalobs::decodeutil::DecodedDataElem &data,
                   const miutil::miTime &obstime,
                   const std::string  &v1,
                   const std::string  &v2,
                   const std::string  &v3,
                   const std::string  &intensitet );
   void decodeAndAddV1V2V3( kvalobs::decodeutil::DecodedDataElem &data,
                            const miutil::miTime &obstime,
                            const std::string &what);


   std::string createTime(const std::string &sObsTime,
                             miutil::miTime &obsTime );


   miutil::miTime createObsTime(const std::string &YYYYMMDDhh,
                                std::string &sYYYYMMDDhh);


   bool onlyZeros(const std::string &val);


   bool doKvPrecipitation(kvalobs::decodeutil::DecodedDataElem &data,
                          const miutil::CommaString &cData,
                          std::ostream &ost);






public:
   Sms2(const ParamList &paramList,
        kvalobs::decoder::comobsdecoder::ComObsDecoder &dec);
   ~Sms2();

   kvalobs::decodeutil::DecodedData*
   decode(long stationid,
          int  smscode,
          const SmsMelding::MeldingList &obs,
          std::string &returnMessage,
          std::list<kvalobs::kvRejectdecode> &rejected,
          bool &hasRejected);

};

}
}
}

#endif
