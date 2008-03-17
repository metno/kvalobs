/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kv2kvEntry.cc,v 1.2.2.1 2007/09/27 09:02:29 paule Exp $                                                       

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
#include <list>
#include "kv2kvEntry.h"
#include "kv2kvDecoder.h"
#include <milog/milog.h>

using namespace kvalobs::decoder::kv2kvDecoder;

kvalobs::decoder::DecoderBase* 
decoderFactory(dnmi::db::Connection &con,
	       const ParamList      &params,
	       const std::list<kvalobs::kvTypes> &typeList,
	       int   decoderId,
	       const miutil::miString &obsType,
	       const miutil::miString &obs)
{
  kv2kvDecoder *dec;

  try{
    dec = new kv2kvDecoder(con, params, typeList, obsType, obs, decoderId);
  }
  catch(...){
    //LOGERROR("Error when creating kv2kvDecoder");
    return 0;
  }
  
  return dec;
}

void 
releaseDecoder(kvalobs::decoder::DecoderBase* decoder)
{
  delete decoder;
}


std::list<miutil::miString> 
getObsTypes()
{
  std::list<miutil::miString> list;

  list.push_back("kv2kvDecoder");

  return list;
}

