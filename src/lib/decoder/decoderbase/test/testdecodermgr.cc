/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: testdecodermgr.cc,v 1.3.6.2 2007/09/27 09:02:27 paule Exp $                                                       

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
#include "../include/decoderbase/drivermgr.h"
#include "../include/decoderbase/decodermgr.h"


using namespace dnmi::db;
using namespace std;
using namespace kvalobs::decoder;
using namespace miutil;

int
main(int argn, char **argv)
{
    
    string driver("../../db/obj/pgdriver.so");
    string drvId;
    //string constr("dbname=kvtest");
    string constr("host=rime dbname=borge user=borgem password=borgem");
    Connection *con;
    DecoderBase   *decoder;
    DriverManager dbmngr;
    DecoderMgr    dcdmgr("../autoobsdecoder/obj/");

    if(!dbmngr.loadDriver(driver, drvId)){
	cerr << "Can't load driver <" << driver << ">\n";
	cerr << dbmngr.getErr() << endl;
	
	return 1;
    }else
	cerr << "Driver <" << drvId<< "> loaded!\n";

    
    con=dbmngr.connect(drvId, constr);

    if(!con){
	cerr << "Can't create connection to <" << drvId << ">\n";
	return 1;
    }

    cerr << "Connected to <" << drvId << ">\n";

    miString obsType("autoobs");
    miString obs("autoobs::obs");
    miString errMsg;

    decoder=dcdmgr.findDecoder(*con, 10224, obsType, obs, errMsg);
    
    if(decoder){
      cerr << "Found a decoder!\n";
    
      if(decoder->execute(errMsg))
	cerr << "Decoder: ret ok!\n";
      else
	cerr << "Decoder: ret error!\n";
      
      dcdmgr.releaseDecoder(decoder);
    }else
      cerr << "ERROR: Can't find a decoder! <" << errMsg << ">\n";

    delete con;
}
