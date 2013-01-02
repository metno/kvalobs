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
#include <float.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <kvdb/dbdrivermgr.h>
#include <fileutil/readfile.h>
#include <fileutil/dir.h>
#include <fileutil/file.h>
#include <puTools/miTime.h>
#include <miutil/timeconvert.h>
#include <dbdrivers/dummysqldb.h>
#include "../smsdata.h"
#include "../sms2.h"
#include "ReadParamsFromFile.h"
#include "../smsmeldingparser.h"
#include "FakeComobsDecoder.h"
#include <gtest/gtest.h>

using namespace std;
using namespace kvalobs;
using namespace kvalobs::decoder::comobsdecoder;
using namespace miutil;
using namespace dnmi::file;
using namespace dnmi::db;
using namespace kvalobs::decodeutil;




class Sms2DecodeTest : public testing::Test
{

protected:
   string dbId;
   DriverManager dbMgr;
   string testdir;
   string dbdir;
   ParamList        paramList;
   std::list<kvalobs::kvTypes> typesList;

   ///Called before each test case.
   virtual void SetUp() {
      testdir = TESTDIR;
      dbdir = DBDIR;

      if( dbId.empty() )
         ASSERT_TRUE( dbMgr.loadDriver( dbdir+"/dummydriver.so", dbId ) )<<
         "Failed to load Db driver.";

      if( paramList.empty() )
         ASSERT_TRUE( ReadParamsFromFile(testdir+"/param.csv", paramList ) ) <<
         "Cant read params from the file <param.csv>";
   }

   ///Called after each test case.
   virtual void TearDown() {

   }

   kvalobs::decodeutil::DecodedDataElem
   getDataSet( DecodedData *decodedData, const miutil::miTime &obstime )
   {
      for( TDecodedDataElem::iterator it=decodedData->data()->begin();
            it != decodedData->data()->end(); ++it ) {
         if( it->getDate() == obstime )
            return *it;
      }

      return decodedData->createDataElem();
   }

   float getData( const kvalobs::decodeutil::DecodedDataElem &data,
                  const miutil::miTime &obstime,
                  int paramid, int sensor=0, int level=0)
   {
      for( std::list<kvalobs::kvData>::const_iterator it=data.data().begin();
            it != data.data().end(); ++it ){
         if( to_miTime(it->obstime())==obstime && it->paramID()==paramid &&
               it->sensor() == sensor && it->level() == level )
            return it->original();
      }

      return FLT_MAX;
   }

   kvalobs::decodeutil::DecodedData*
   runtestOnFile( const ParamList &paramList,
                  std::list<kvalobs::kvTypes> typesList,
                  const dnmi::file::File &file);

};


TEST_F( Sms2DecodeTest, MultiObsTime )
{
   dnmi::file::File file;
   kvalobs::decodeutil::DecodedData *data;

   file=File(testdir+"/testdata/sms02-29-201012280654.xml");
   data = runtestOnFile( paramList, typesList, file );

   EXPECT_TRUE( data != 0 );
   EXPECT_TRUE( data->size() == 1 ) << "DataSize: " << data->size();

   /*
   for( TDecodedDataElem::iterator eit = data->data()->begin(); eit != data->data()->end(); ++eit ) {
      cerr << "Date: " << eit->getDate() << endl;
      for(std::list<kvalobs::kvData>::const_iterator dit=eit->data().begin(); dit!= eit->data().end(); dit++){
         cerr << *dit  << endl;
      }
      cerr << " ----------------------------- " << endl;
   }*/

   DecodedDataElem elem = getDataSet( data, "2010-12-28 06:00:00" );
   EXPECT_TRUE( elem.dataSize() == 9 ) << " DataSet: size = " << elem.dataSize();
   EXPECT_TRUE( elem.getDate() == miTime("2010-12-28 06:00:00") ) << "Date: " << elem.getDate();
   EXPECT_FLOAT_EQ( getData( elem, miTime("2010-12-28 06:00:00"), 34), 2 )    << "Val: " << getData( elem, miTime("2010-12-28 06:00:00"), 34);
   EXPECT_FLOAT_EQ( getData( elem, miTime("2010-12-28 06:00:00"), 35), 0 )    << "Val: " << getData( elem, miTime("2010-12-28 06:00:00"), 35);
   EXPECT_FLOAT_EQ( getData( elem, miTime("2010-12-28 06:00:00"), 110), 0.1 ) << "Val: " << getData( elem, miTime("2010-12-28 06:00:00"), 110);
   EXPECT_FLOAT_EQ( getData( elem, miTime("2010-12-28 06:00:00"), 112), 49 )  << "Val: " << getData( elem, miTime("2010-12-28 06:00:00"), 112);

   EXPECT_FLOAT_EQ( getData( elem, miTime("2010-12-27 12:00:00"), 34), 2 )    << "Val: " << getData( elem, miTime("2010-12-28 06:00:00"), 34);
   EXPECT_FLOAT_EQ( getData( elem, miTime("2010-12-27 12:00:00"), 35), 0 )    << "Val: " << getData( elem, miTime("2010-12-28 06:00:00"), 35);

   EXPECT_FLOAT_EQ( getData( elem, miTime("2010-12-27 18:00:00"), 34), 2 )    << "Val: " << getData( elem, miTime("2010-12-28 06:00:00"), 34);
   EXPECT_FLOAT_EQ( getData( elem, miTime("2010-12-27 18:00:00"), 35), 0 )    << "Val: " << getData( elem, miTime("2010-12-28 06:00:00"), 35);
}

int
main(int argc, char **argv) {
   ::testing::InitGoogleTest(&argc, argv);
   return RUN_ALL_TESTS();
}

kvalobs::decodeutil::DecodedData*
Sms2DecodeTest::
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

   string fname=file.file();
   string::size_type i;

   i=fname.find_last_of("-");

   if(i==string::npos){
      cerr << "NO TIMESTAMP: Invalid format on filename." << endl;
      return 0;
   }

   fname=fname.substr(i+1);
   i=fname.find(".xml");

   if(i==string::npos){
      cerr << "NO TIMESTAMP: Invalid format on filename." << endl;
      return 0;
   }

   fname.erase(i);

   nowTime.setTime(fname);

   if(!dnmi::file::ReadFile(file.name(), fcontent)){
      cerr << "cant read the file!" << endl;
      return 0;
   }

   for( int i=0; i<2; ++i ) {
      if( of.is_open() )
         of.close();

      string myFile( "testresult/"+file.file() );
      of.open( myFile.c_str());

      if(!of.is_open() ){
         if( i>0 ) {
            cerr << "Cant open result file: " << myFile << endl;
            return 0;
         }

         mkdir( "./testresult", 0775 );
      } else {
         break;
      }
   }

   smsmsg=smsparse.parse(fcontent);

   if(!smsmsg){
      of << "Cant parse the smsmsg!" << endl;
      return 0;
   }

   of << "Timestamp: " << fname << " miTime: " << nowTime <<endl;
   of << fcontent << endl;

   of << "SMS code  : " << smsmsg->getCode() << endl;
   of << "nationalid: " << smsmsg->getClimano() << endl;
   of << "wmono     : " << smsmsg->getSynopno() << endl;

   fakeComobsDecoder = new FakeComobsDecoder( *dummyCon, paramList, typesList, "", "", 1 );

   Sms2 sms(paramList, *fakeComobsDecoder );

   sms.setNowTime(nowTime);

   data=sms.decode(smsmsg->getClimano(), smsmsg->getCode(),
                   smsmsg->getMeldingList(), returnMsg, rejected,
                   hasRejected);

   if( hasRejected )
      of << rejected.begin()->comment() << endl;

   if( !data ){
      of << "FAILED: cant decode smsmsg!" << endl;
   }else{
      kvalobs::decodeutil::TDecodedDataElem *elem = data->data();

      if( ! elem  ) {
         of << "No data!\n";
      } else {
         cerr << endl << fcontent << endl;

         for( kvalobs::decodeutil::TDecodedDataElem::iterator eit = elem->begin(); eit != elem->end(); ++eit ) {
            for(std::list<kvalobs::kvData>::const_iterator dit=eit->data().begin(); dit!= eit->data().end(); dit++){
               of << *dit << endl ;
            //   cerr << *dit  << endl;
            }
         }
      }
   }

   of.close();

   return data;
}


