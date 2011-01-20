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

#include <kvalobs/kvData.h>
#include <list>
#include <kvalobs/kvStation.h>
#include <decoderbase/decoder.h>
#include <vector>
#include "kldata.h"
#include "paramdef.h"


namespace kvalobs{
namespace decoder{
namespace kldecoder{


/**
* \addtogroup kldecode
*
* @{
*/

typedef boost::mutex::scoped_lock    Lock;

/**
* \brief implements the interface  DecoderBase.
*
* <pre>
* Dataformat for message.
*
* obsType: kldata/nationalnr=<nummer>/type=<typeid>
* obs:
*   <pc1>[(<sensor>,<level>)],...,pcN[(<sensor>,<level>)]
*   YYYYMMDDhhmmss,<pc1_value[(<cinfo>,<uinfo>)]>,...,<pcN_value[(<cinfo>,<uinfo>)]>
*   YYYYMMDDhhmmss,<pc1_value[(<cinfo>,<uinfo>)]>,....,<pcN_value[(<cinfo>,<uinfo>)]>
*   ....
*   YYYYMMDDhhmmss,<pc1_value[(<cinfo>,<uinfo>)]>,....,<pcN_value[(<cinfo>,<uinfo>)]>
*
*  pc - paramcode, the name of the parameter. An underscore indicate that
*                  this is a code value. Suported pc that can have a code
*                  value is: HL and VV. The value vil be converted til meter.
*  If sensor or level is not specified. The default would apply. If both shall
*  take the default value, the paranteses can be left out.
*
*  cinfo - controlinfo
*  uinfo - useinfo
*  </pre>
*
*/
class KlDecoder : public DecoderBase{
   KlDecoder();
   KlDecoder(const KlDecoder &);
   KlDecoder& operator=(const KlDecoder &);

   long getStationId(miutil::miString &msg);
   long getTypeId(miutil::miString &msg)const;

   bool splitParams(const std::string &header,
                    std::list<std::string> &params,
                    miutil::miString &msg);

   bool splitData(const std::string &sdata,
                  std::list<std::string> &datalist,
                  miutil::miString &msg);

   bool decodeHeader(const std::string &header,
                     std::vector<ParamDef> &params,
                     miutil::miString &message);

   bool decodeData(KlDataArray &da,
                   KlDataArray::size_type daSize,
                   const std::string &sdata,
                   int line,
                   miutil::miString &msg);

   std::string toupper(const std::string &s);
   bool warnings;
   std::string logid;

public:
   KlDecoder(dnmi::db::Connection     &con,
             const ParamList        &params,
             const std::list<kvalobs::kvTypes> &typeList,
             const miutil::miString &obsType,
             const miutil::miString &obs,
             int                    decoderId=-1);

   virtual ~KlDecoder();

   virtual miutil::miString name()const;

   virtual DecodeResult execute(miutil::miString &msg);
};

/** @} */
}
}
}

#endif
