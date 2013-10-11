/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: dummydecoder.cc,v 1.1.2.2 2007/09/27 09:02:28 paule Exp $                                                       

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
#include <milog/milog.h>
#include <puTools/miTime.h>
#include "testdecoder.h"

namespace kvalobs{
namespace decoder{
namespace testdecoder{

TestDecoder::TestDecoder(
      dnmi::db::Connection   &con,
      const ParamList        &params,
      const std::list<kvalobs::kvTypes> &typeList,
      const std::string &obsType,
      const std::string &obs,
      int   decoderId)
:DecoderBase(con, params, typeList, obsType, obs, decoderId)
{
}

TestDecoder::~TestDecoder()
{
}

std::string
TestDecoder::name() const
{
   return "TestDecoder";
}

kvalobs::decoder::DecoderBase::DecodeResult 
TestDecoder::execute(std::string &msg)
{
   milog::LogContext lcontext(name());

   LOGINFO("New observation!  " << miutil::miTime::nowTime());


   return Ok;

}

}
}
}
