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
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <boost/thread.hpp>
#include <kvdb/dbdrivermgr.h>
#include <fileutil/readfile.h>
#include <fileutil/dir.h>
#include <fileutil/file.h>
#include <dbdrivers/dummysqldb.h>
#include "../smsdata.h"
#include "../sms2.h"
#include "ReadParamsFromFile.h"
#include "../smsmeldingparser.h"
#include "FakeComobsDecoder.h"
#include "../../decoderbase/RedirectInfo.h"

using namespace std;
using namespace kvalobs;
using namespace kvalobs::decoder::comobsdecoder;
using namespace miutil;
using namespace dnmi::db;

void runtestOnFile(const ParamList &paramList,
                   std::list<kvalobs::kvTypes> typesList,
                   const dnmi::file::File &file);

namespace kvdatainput {
namespace decodecommand {
boost::thread_specific_ptr<kvalobs::decoder::RedirectInfo> ptrRedirect;
}
}


string dbId;
DriverManager dbMgr;
string testdir( TESTDIR );

int
main(int argn, char **argv)
{  
   dnmi::file::Dir  dir;
   dnmi::file::File file;
   ParamList        paramList;
   std::list<kvalobs::kvTypes> typesList;
   string           inFile;

   cerr << "TESTDIR: " << testdir << endl;
   if( dbMgr.loadDriver( "./src/lib/dbdrivers/.libs/dummydriver.so", dbId ) ) {
      cerr << "Db driver loaded: " << dbId << endl;
   } else {
      cerr << "Failed to laod db driver for: dummydriver. Reason: " << dbMgr.getErr() << endl;
      return 1;
   }

   if(!ReadParamsFromFile(testdir+"/param.csv", paramList)){
      cerr << "Cant read params from the file <param.csv>" << endl;
   }

   // if(!ReadTypesFromFile("types.csv", typesList)){
   // 	cerr << "Cant read params from the file <types.csv>" << endl;
   // }

   if(argn>1){
      file=dnmi::file::File(argv[1]);

      if(!file.ok()){
         cerr << "Cant read file <" << argv[1] << endl;
         return 1;
      }else{
         cerr << "Reading file: " << file.name() << endl;

         runtestOnFile(paramList, typesList, file);
         return 0;
      }
   }

   if(!dir.open( testdir+"/testdata", "sms\?\?-\?\?-????????????.xml")){
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
              const std::list<kvalobs::kvTypes> typesList,
              const dnmi::file::File &file)
{
   dnmi::db::Connection *dummyCon= dbMgr.connect( dbId, "" );
   FakeComobsDecoder *fakeComobsDecoder;

   SmsMelding       *smsmsg;
   SmsMeldingParser smsparse;
   string           fcontent;
   miTime           obst;
   kvalobs::decodeutil::DecodedData *data;
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



   for( int i=0; i<2; ++i ) {
      if( of.is_open() )
         of.close();

      string myFile( "testresult/"+file.file() );
      of.open( myFile.c_str());

      if(!of.is_open() ){
         if( i>0 ) {
            cerr << "Cant open result file: " << myFile << endl;
            return;
         }

         mkdir( "./testresult", 0775 );
      } else {
         cerr << "Open file: " << myFile << endl;
         break;
      }
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

   fakeComobsDecoder = new FakeComobsDecoder( *dummyCon, paramList, typesList, "", "", 1 );

   Sms2 sms(paramList, *fakeComobsDecoder );

   sms.setNowTime(nowTime);

   data=sms.decode(smsmsg->getClimano(), smsmsg->getCode(),
                   smsmsg->getMeldingList(), returnMsg, rejected,
                   hasRejected);

   if( hasRejected )
      of << rejected.begin()->comment() << endl;

   if( !data ){
      cerr << "FAILED: cant decode smsmsg!" << endl;
   }else{
      kvalobs::decodeutil::TDecodedDataElem *elem = data->data();

      if( ! elem  ) {
         cerr << "No data!\n";
      } else {
         cerr << endl << fcontent << endl;

      for( kvalobs::decodeutil::TDecodedDataElem::iterator eit = elem->begin(); eit != elem->end(); ++eit ) {
         for(std::list<kvalobs::kvData>::const_iterator dit=eit->data().begin(); dit!= eit->data().end(); dit++){
            of << *dit << endl ;
         cerr << *dit ;
      }
    }
      }
   }

   of.close();
}


