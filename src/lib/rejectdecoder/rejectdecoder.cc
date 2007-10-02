/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: rejectdecoder.cc,v 1.1.2.2 2007/09/27 09:02:32 paule Exp $                                                       

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
#include <puTools/miTime>
#include <kvalobs/kvDbGate.h>
#include "rejectdecoder.h"


using namespace std;
using namespace kvalobs; 

kvalobs::decoder::rejectdecoder::
RejectDecoder::RejectDecoder(
    dnmi::db::Connection   &con,
    const ParamList        &params,
    const std::list<kvalobs::kvTypes> &typeList,
    const miutil::miString &obsType,
    const miutil::miString &obs, 
    int                    decoderId)
    :DecoderBase(con, params, typeList, obsType, obs, decoderId)
{
}

kvalobs::decoder::rejectdecoder::
RejectDecoder::~RejectDecoder()
{
}

miutil::miString 
kvalobs::decoder::rejectdecoder::
RejectDecoder::name() const
{
    return "RejectDecoder";
}


bool 
kvalobs::decoder::rejectdecoder::
RejectDecoder::doDecode(const std::string &message, 
			kvalobs::kvRejectdecode &reject)
{
  string sep("@$$reject$$@\n");
  string elems[3];   
  string::size_type i1, i2;
  int    n=0;

  i1=0;
  
  while(i2!=string::npos){
    i2=message.find(sep, i1);
    
    if(i2==string::npos && n<2)
      return false;
      
    if(i2==string::npos)
      elems[n]=message.substr(i1);
    else
      elems[n]=message.substr(i1, i2-i1);
    
    i1=i2+sep.length();
    n++;
  }

  return reject.set(elems[0], miutil::miTime::nowTime(), elems[1], elems[2]);
}


kvalobs::decoder::DecoderBase::DecodeResult 
kvalobs::decoder::rejectdecoder::
RejectDecoder::execute(miutil::miString &msg)
{
  kvRejectdecode rejected;
  milog::LogContext lcontext(name());
  
  LOGINFO("Rejected:  " << miutil::miTime::nowTime());
 
  
  if(doDecode(obs, rejected)){
    kvDbGate gate(getConnection());
    LOGDEBUG6("The observation was successfully decoded!"<< endl);

    if(gate.insert(rejected)){
      LOGDEBUG6("The message was put into the database!" << endl);
    }else{
      LOGWARN("DBERROR: the message could not be inserted into the db!" 
	      << endl << "Reason: " << gate.getErrorStr() << endl);
    }
  }

  //We return allways ok.

  return Ok;
  
 }


