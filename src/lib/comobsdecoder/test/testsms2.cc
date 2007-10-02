/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: testsms2.cc,v 1.2.2.3 2007/09/27 09:02:24 paule Exp $                                                       

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
#include <fstream>
#include <iostream>
#include <db/dbdrivermgr.h>
#include <fileutil/readfile.h>
#include <fileutil/dir.h>
#include <fileutil/file.h>
#include "../sms2.h"
#include "ReadParamsFromFile.h"
#include "../smsmeldingparser.h"

using namespace std;
using namespace kvalobs;
using namespace kvalobs::decoder::comobsdecoder;
using namespace miutil;
using namespace dnmi::db;

void runtestOnFile(const ParamList &paramList,
					const list<kvTypes> &typelist,
				   	const dnmi::file::File &file);

int
main(int argn, char **argv)
{  
  dnmi::file::Dir  dir;
  dnmi::file::File file;
  ParamList        paramList;
  std::list<kvalobs::kvTypes> typesList;
  string           inFile;


  if(!ReadParamsFromFile("param.csv", paramList)){
    cerr << "Cant read params from the file <param.csv>" << endl;
  }
  
  if(!ReadTypesFromFile("types.csv", typesList)){
  	cerr << "Cant read params from the file <types.csv>" << endl;
  }

  if(argn>1){
    file=dnmi::file::File(argv[1]);
    
    if(!file.ok()){
      cerr << "Cant read file <" << argv[1] << endl;
      return 1;
    }else{
      cerr << "Reading file: " << file.name() << endl;

      runtestOnFile(paramList,file);
      return 0;
    }
  }

  if(!dir.open("testdata", "sms??-??-????????????.xml")){
      cerr << "Cant read data from the <testdata> directory!" << endl;
      return 1;
  }

  

  while(dir.hasNext()){
    dir.next(file);
    
    runtestOnFile(paramList, typesList, file);
  }

  
}

void 
runtestOnFile(const ParamList &paramList, 
			  const list<kvTypes> &typelist,
			  const dnmi::file::File &file)
{
  SmsMelding       *smsmsg;
  SmsMeldingParser smsparse;
  string           fcontent;
  miTime           obst;
  SmsData          *data;
  string           returnMsg;
  list<kvalobs::kvRejectdecode> rejected;
  bool             hasRejected;
  ofstream         of;
  miTime           nowTime;
  cerr << "testfile: " << file.name() << endl;

  string fname=file.file();
  string::size_type i;
  
  i=fname.find_last_of("-");
  
  if(i==string::npos){
    cerr << "NO TIMESTAMP: Invalid format on filename." << endl;
    return;
  }
  
  fname=fname.substr(i+1);
  i=fname.find(".xml");
  
  if(i==string::npos){
    cerr << "NO TIMESTAMP: Invalid format on filename." << endl;
    return;
  }
  
  fname.erase(i);
  
  nowTime.setTime(fname);
  cerr << "Timestamp: " << fname << " miTime: " << nowTime <<endl;
  
  
  if(!dnmi::file::ReadFile(file.name(), fcontent)){
    cerr << "cant read the file!" << endl;
    return;
  }
  
  of.open(string("testresult/"+file.file()).c_str());

  if(!of){
    cerr << "Cant open result file: " << "testresult/"+file.file() << endl;
    return;
  }

  smsmsg=smsparse.parse(fcontent);
  
  if(!smsmsg){
    cerr << "Cant parse the smsmsg!" << endl;
    return;
  }

  of << fcontent << endl;
  cerr << fcontent << endl;

  cerr << "SMS code  : " << smsmsg->getCode() << endl;
  cerr << "nationalid: " << smsmsg->getClimano() << endl;
  cerr << "wmono     : " << smsmsg->getSynopno() << endl;
  
  Sms2 sms(paramList);
  
  sms.setNowTime(nowTime);

  data=sms.decode(smsmsg->getClimano(), smsmsg->getCode(),
		  smsmsg->getMeldingList(), returnMsg, rejected, 
		  hasRejected);
  
  if(!data){
    cerr << "FAILED: cant decode smsmsg!" << endl;
  }else{
    TSmsDataElem *elem=data->data();
      
    if(!elem){
      cerr << "No data in smsmsg!" << endl;
    }else{
      CITSmsDataElem eit=elem->begin();
      
      cerr << endl << fcontent << endl;

      for(;eit!=elem->end(); eit++){
	of << *eit ;
	cerr << *eit ;
      }
    }
  }
  
  of.close();
}

