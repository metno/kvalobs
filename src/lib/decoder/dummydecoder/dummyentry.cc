/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: dummyentry.cc,v 1.1.2.1 2007/09/27 09:02:28 paule Exp $                                                       

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
#include "dummyentry.h"
#include "dummydecoder.h"
#include <milog/milog.h>

using namespace kvalobs::decoder::dummydecoder;

kvalobs::decoder::DecoderBase*
decoderFactory(dnmi::db::Connection &con, const ParamList &params,
               const std::list<kvalobs::kvTypes> &typeList, int decoderId,
               const std::string &obsType, const std::string &obs) {
  DummyDecoder *dec;

  try {
    dec = new DummyDecoder(con, params, typeList, obsType, obs, decoderId);
  } catch (...) {
    return 0;
  }

  return dec;
}

void releaseDecoder(kvalobs::decoder::DecoderBase* decoder) {
  delete decoder;
}

void setKvConf(kvalobs::decoder::DecoderBase* decoder,
               miutil::conf::ConfSection *theKvConf) {
  decoder->setKvConf(theKvConf);
}

std::list<std::string> getObsTypes() {
  std::list<std::string> list;

  list.push_back("dummydecode");

  return list;
}

