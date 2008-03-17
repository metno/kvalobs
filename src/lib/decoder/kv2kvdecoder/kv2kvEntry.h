/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kv2kvEntry.h,v 1.1.2.2 2007/09/27 09:02:29 paule Exp $                                                       

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
#ifndef __kv2kvEntry_h__
#define __kv2kvEntry_h__

#ifdef __cplusplus
#include <decoderbase/decoder.h>
#include <kvalobs/kvTypes.h>
#include <list>

/**
 * \defgroup kldecode Decoder for data from the klima department (CLOBS).
 * \ingroup kvdecoder
 *
 * @{ 
 */

extern "C" {
  kvalobs::decoder::DecoderBase* 
  decoderFactory(dnmi::db::Connection &con_,
		 const ParamList &params,
		 const std::list<kvalobs::kvTypes> &typeList,
		 int   decoderId_,
		 const miutil::miString &observationType_,
		 const miutil::miString &observation_);

  void releaseDecoder(kvalobs::decoder::DecoderBase* decoder);

  std::list<miutil::miString> getObsTypes();
}  

/** @} */

#endif // __cplusplus

#endif // __kv2kvEntry_h__
