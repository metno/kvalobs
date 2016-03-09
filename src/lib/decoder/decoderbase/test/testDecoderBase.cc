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
#include <string>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <decoder/decoderbase/decodermgr.h>
#include <kvdb/dbdrivermgr.h>
#include <fileutil/readfile.h>
#include <fileutil/dir.h>
#include <fileutil/file.h>
#include <puTools/miTime.h>
#include <miutil/timeconvert.h>
#include <dbdrivers/dummysqldb.h>
//#include "ReadParamsFromFile.h"
#include "ReadTypesFromFile.h"
#include <gtest/gtest.h>
#include "../RedirectInfo.h"

using namespace std;
using namespace kvalobs;
using namespace miutil;
using namespace dnmi::file;
using namespace dnmi::db;
namespace dc = kvalobs::decoder;
namespace pt = boost::posix_time;

namespace kvdatainput {
namespace decodecommand {
boost::thread_specific_ptr<kvalobs::decoder::RedirectInfo> ptrRedirect;
}
}

class DecoderBaseTest : public testing::Test {

 protected:
  string dbId;
  string testdb;
  dc::DecoderMgr decoderMgr;
  string testdir;
  string dbdir;
  string decoderdir;

  ParamList paramList;
  KvTypeList typesList;

  ///Called before each test case.
  virtual void SetUp() {
    testdb = TESTDB;
    testdir = TESTDIR;
    dbdir = DBDIR;
    decoderdir = DECODERDIR;
    decoderMgr.setDecoderPath(decoderdir);
    decoderMgr.updateDecoders();

    if (dbId.empty()) {
      ASSERT_TRUE(dnmi::db::DriverManager::loadDriver(dbdir+"/sqlite3driver.so", dbId))<<
      "Failed to load Db driver. Reason: " << dnmi::db::DriverManager::getErr();
    }

    if (paramList.empty()) {
      ASSERT_TRUE( readParamsFromFile(testdir+"/kvparams.csv", paramList ) )<<
      "Cant read params from the file <kvparams.csv>";
    }

    if (typesList.empty()) {
      ASSERT_TRUE( ReadTypesFromFile(testdir+"/kvtypes.csv", typesList) )<<
      "Cant read types from the file <kvtypes.csv>";
    }
  }

  ///Called after each test case.
  virtual void TearDown() {
    //cerr << "TearDown:\n";

  }

//   kvalobs::decodeutil::DecodedData*
//   runtestOnFile( const ParamList &paramList,
//                  std::list<kvalobs::kvTypes> typesList,
//                  const dnmi::file::File &file);

};

TEST_F( DecoderBaseTest, useinfo7 ) {
  string error;
  string obsType("test");
  string obsData("");
  KvTypeList types;
  int useinfo7;  //tolate/toearly flag

  types.push_back(kvTypes(1, "", 60, 60, "I", "h", "For test"));

  dnmi::db::Connection *con = dnmi::db::DriverManager::connect(dbId, testdb);

  ASSERT_TRUE( con != 0 )<< "Cant open database connection: " << testdb << ".";

  dc::DecoderBase *dec = decoderMgr.findDecoder(*con, paramList, types, obsType,
                                                obsData, error);
  ASSERT_TRUE( dec != 0 )<< "Cant create test decoder.";

  pt::ptime obstime(pt::time_from_string("2010-09-13 06:00:00"));
  pt::ptime oktime(pt::time_from_string("2010-09-13 06:10:53"));
  pt::ptime oktime1(pt::time_from_string("2010-09-13 05:51:53"));
  pt::ptime toearly(pt::time_from_string("2010-09-13 04:59:00"));
  pt::ptime tolate(pt::time_from_string("2010-09-13 07:01:00"));

  ASSERT_TRUE(dec->getUseinfo7Code(1, oktime, obstime, "") == 0);
  ASSERT_TRUE(dec->getUseinfo7Code(1, oktime1, obstime, "") == 0);
  ASSERT_TRUE(dec->getUseinfo7Code(1, toearly, obstime, "") == 3);
  ASSERT_TRUE(dec->getUseinfo7Code(1, tolate, obstime, "") == 4);

  decoderMgr.releaseDecoder(dec);
}

//TEST_F( Sms2DecodeTest, MultiObsTime )
//{
//   dnmi::file::File file;
//   kvalobs::decodeutil::DecodedData *data;
//
//   file=File(testdir+"/testdata/sms02-29-201012280654.xml");
//   data = runtestOnFile( paramList, typesList, file );
//
//   EXPECT_TRUE( data != 0 );
//   EXPECT_TRUE( data->size() == 1 ) << "DataSize: " << data->size();
//
//   /*
//   for( TDecodedDataElem::iterator eit = data->data()->begin(); eit != data->data()->end(); ++eit ) {
//      cerr << "Date: " << eit->getDate() << endl;
//      for(std::list<kvalobs::kvData>::const_iterator dit=eit->data().begin(); dit!= eit->data().end(); dit++){
//         cerr << *dit  << endl;
//      }
//      cerr << " ----------------------------- " << endl;
//   }*/
//
//   DecodedDataElem elem = getDataSet( data, miTime("2010-12-28 06:00:00") );
//   EXPECT_TRUE( elem.dataSize() == 9 ) << " DataSet: size = " << elem.dataSize();
//   EXPECT_TRUE( elem.getDate() == miTime("2010-12-28 06:00:00") ) << "Date: " << elem.getDate();
//   EXPECT_FLOAT_EQ( getData( elem, miTime("2010-12-28 06:00:00"), 34), 2 )    << "Val: " << getData( elem, miTime("2010-12-28 06:00:00"), 34);
//   EXPECT_FLOAT_EQ( getData( elem, miTime("2010-12-28 06:00:00"), 35), 0 )    << "Val: " << getData( elem, miTime("2010-12-28 06:00:00"), 35);
//   EXPECT_FLOAT_EQ( getData( elem, miTime("2010-12-28 06:00:00"), 110), 0.1 ) << "Val: " << getData( elem, miTime("2010-12-28 06:00:00"), 110);
//   EXPECT_FLOAT_EQ( getData( elem, miTime("2010-12-28 06:00:00"), 112), 49 )  << "Val: " << getData( elem, miTime("2010-12-28 06:00:00"), 112);
//
//   EXPECT_FLOAT_EQ( getData( elem, miTime("2010-12-27 12:00:00"), 34), 2 )    << "Val: " << getData( elem, miTime("2010-12-28 06:00:00"), 34);
//   EXPECT_FLOAT_EQ( getData( elem, miTime("2010-12-27 12:00:00"), 35), 0 )    << "Val: " << getData( elem, miTime("2010-12-28 06:00:00"), 35);
//
//   EXPECT_FLOAT_EQ( getData( elem, miTime("2010-12-27 18:00:00"), 34), 2 )    << "Val: " << getData( elem, miTime("2010-12-28 06:00:00"), 34);
//   EXPECT_FLOAT_EQ( getData( elem, miTime("2010-12-27 18:00:00"), 35), 0 )    << "Val: " << getData( elem, miTime("2010-12-28 06:00:00"), 35);
//}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

//kvalobs::decodeutil::DecodedData*
//DecoderBaseTest::
//runtestOnFile(const ParamList &paramList,
//              const std::list<kvalobs::kvTypes> typesList,
//              const dnmi::file::File &file)
//{
//   dnmi::db::Connection *dummyCon= dbMgr.connect( dbId, "" );
//   FakeComobsDecoder *fakeComobsDecoder;
//
//   SmsMelding       *smsmsg;
//   SmsMeldingParser smsparse;
//   string           fcontent;
//   miTime           obst;
//   kvalobs::decodeutil::DecodedData *data;
//   string           returnMsg;
//   list<kvalobs::kvRejectdecode> rejected;
//   bool             hasRejected;
//   ofstream         of;
//   miTime           nowTime;
//
//   string fname=file.file();
//   string::size_type i;
//
//   i=fname.find_last_of("-");
//
//   if(i==string::npos){
//      cerr << "NO TIMESTAMP: Invalid format on filename." << endl;
//      return 0;
//   }
//
//   fname=fname.substr(i+1);
//   i=fname.find(".xml");
//
//   if(i==string::npos){
//      cerr << "NO TIMESTAMP: Invalid format on filename." << endl;
//      return 0;
//   }
//
//   fname.erase(i);
//
//   nowTime.setTime(fname);
//
//   if(!dnmi::file::ReadFile(file.name(), fcontent)){
//      cerr << "cant read the file!" << endl;
//      return 0;
//   }
//
//   for( int i=0; i<2; ++i ) {
//      if( of.is_open() )
//         of.close();
//
//      string myFile( "testresult/"+file.file() );
//      of.open( myFile.c_str());
//
//      if(!of.is_open() ){
//         if( i>0 ) {
//            cerr << "Cant open result file: " << myFile << endl;
//            return 0;
//         }
//
//         mkdir( "./testresult", 0775 );
//      } else {
//         break;
//      }
//   }
//
//   smsmsg=smsparse.parse(fcontent);
//
//   if(!smsmsg){
//      of << "Cant parse the smsmsg!" << endl;
//      return 0;
//   }
//
//   of << "Timestamp: " << fname << " miTime: " << nowTime <<endl;
//   of << fcontent << endl;
//
//   of << "SMS code  : " << smsmsg->getCode() << endl;
//   of << "nationalid: " << smsmsg->getClimano() << endl;
//   of << "wmono     : " << smsmsg->getSynopno() << endl;
//
//   fakeComobsDecoder = new FakeComobsDecoder( *dummyCon, paramList, typesList, "", "", 1 );
//
//   Sms2 sms(paramList, *fakeComobsDecoder );
//
//   sms.setNowTime(nowTime);
//
//   data=sms.decode(smsmsg->getClimano(), smsmsg->getCode(),
//                   smsmsg->getMeldingList(), returnMsg, rejected,
//                   hasRejected);
//
//   if( hasRejected )
//      of << rejected.begin()->comment() << endl;
//
//   if( !data ){
//      of << "FAILED: cant decode smsmsg!" << endl;
//   }else{
//      kvalobs::decodeutil::TDecodedDataElem *elem = data->data();
//
//      if( ! elem  ) {
//         of << "No data!\n";
//      } else {
//         cerr << endl << fcontent << endl;
//
//         for( kvalobs::decodeutil::TDecodedDataElem::iterator eit = elem->begin(); eit != elem->end(); ++eit ) {
//            for(std::list<kvalobs::kvData>::const_iterator dit=eit->data().begin(); dit!= eit->data().end(); dit++){
//               of << *dit << endl ;
//            //   cerr << *dit  << endl;
//            }
//         }
//      }
//   }
//
//   of.close();
//
//   return data;
//}

