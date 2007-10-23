/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: force.cc,v 1.9.2.7 2007/09/27 09:02:18 paule Exp $                                                       

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
/**
 * \file
 * This file is an artifical used to force symbols in the executible 
 * so they may be used from a shared object. If the libs where the symbols 
 * are defined is linked with the shared object there is a posibility that 
 * the program will core dump at exit if there is static variables in some 
 * classes. I am not sure way this is happening, is it a bug in the linker, 
 * a bug in the runtime linker or a bug in the implementation in the class
 * that implement the static variable.
 *
 * I supose we could have avoided the problem if we linked all library as
 * shared objects. ie: kvalobs, miutil etc. 
 */

#include <kvalobs/kvDataOperations.h>
#include <kvalobs/kvRejectdecode.h>
#include <kvalobs/kvStation.h>
#include <kvalobs/kvTypes.h>
#include <kvalobs/paramlist.h>
#include <kvalobs/kvTextData.h>
#include <decoderbase/decoder.h>
#include <decodeutility/kvDataFormatter.h>
#include <decodeutility/decodeutility.h>
//#include <decodeutility/KvDataSerializer.h>
#include <decodeutility/kvalobsdataparser.h>
#include <fileutil/mkdir.h>
#include <kvalobs/kvDataOperations.h>

//class kvDataFactory( 0, miutil::miTime(), 0 ); 


namespace {
  kvalobs::kvData         dummyKvData;
  kvalobs::kvRejectdecode dummyKvRejectdecode;
  kvalobs::kvTypes        dummyKvTypes;
  kvalobs::kvTextData     dummyKvTextData;
  kvalobs::kvStation      dummykvStation;

  //decodeutility::KvDataSerializer dummyKvDataSerializer;
  //decodeutility::KvDataSerializer dummyKvDataSerializer1(std::string());

  void dummyFunc1(){
    ParamList pl;
    findParamIdInList(pl, 0);
  }

  void dummyFunc2(){
    using namespace kvalobs::decoder;

    std::string dummy1;
    int         dummy2;

    DecoderBase::isTextParam(dummy1);
    DecoderBase::isTextParam(dummy2);
  }

  void dummyFunc3(){
    decodeutility::kvdataformatter::kvDataList dummy;
    miutil::miString    dummy1;

    dummy=decodeutility::kvdataformatter::getKvData(dummy1);
  }

  void dummyFunc4(){
    std::string dummy;
    std::string ret;

    ret=decodeutility::VV(dummy);
  }
  
  void dummyFunc5(){
  	dnmi::file::mkdir("","");
  }

  void dummyfunc6( kvalobs::serialize::KvalobsDataParser * parser ){
    kvalobs::serialize::KvalobsData d;
    parser->parse( "foo", d );
  }

  void dummyfunc7(){
    kvalobs::compare::eq_sensor( 3, 65 );
  }
   
};

