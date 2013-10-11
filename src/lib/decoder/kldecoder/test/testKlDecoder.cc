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
#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/assign.hpp>
#include <decoder/decoderbase/decodermgr.h>
#include <kvdb/dbdrivermgr.h>
#include <fileutil/readfile.h>
#include <fileutil/dir.h>
#include <fileutil/file.h>
#include <puTools/miTime.h>
#include <miutil/timeconvert.h>
#include <miconfparser/miconfparser.h>
#include <dbdrivers/dummysqldb.h>
#include <decoderbase/test/ReadParamsFromFile.h>
#include <decoderbase/test/ReadTypesFromFile.h>
#include <decoderbase/test/ReadDataFromFile.h>
#include "../KvDataContainer.h"
#include "../kldata.h"
#include "../kldecoder.h"
#include "../DataDecode.h"
#include <gtest/gtest.h>

using namespace std;
using namespace kvalobs;
using namespace miutil;
using namespace dnmi::file;
using namespace dnmi::db;
//using namespace miutil::conf;

using namespace kvalobs::decoder::kldecoder;
namespace dc = kvalobs::decoder;
namespace pt = boost::posix_time;
namespace ba = boost::assign;

namespace {
const char *schemaKvStation = "CREATE TABLE station (	"
		"stationid INTEGER NOT NULL,	"
		"lat FLOAT DEFAULT NULL,	"
		"lon FLOAT DEFAULT NULL,	"
		"height FLOAT DEFAULT NULL,	"
		"maxspeed FLOAT DEFAULT NULL,	"
		"name       TEXT DEFAULT NULL,	"
		"wmonr      INTEGER DEFAULT NULL,	"
		"nationalnr INTEGER DEFAULT NULL,	"
		"ICAOid     CHAR(4) DEFAULT NULL,	"
		"call_sign  CHAR(7) DEFAULT NULL,	"
		"stationstr TEXT DEFAULT NULL,       "
		"environmentid  INTEGER DEFAULT NULL,	"
		"static    BOOLEAN DEFAULT FALSE,  "
		"fromtime TIMESTAMP NOT NULL,	"
		"UNIQUE ( stationid, fromtime ));";

//stationid |  lat   |  lon   | height | maxspeed |         name         | wmonr | nationalnr | icaoid | call_sign | stationstr | environmentid | static |      fromtime
//-----------+--------+--------+--------+----------+----------------------+-------+------------+--------+-----------+------------+---------------+--------+---------------------
//     59680 | 62.181 | 6.0807 |     74 |        0 | ØRSTA-VOLDA LUFTHAVN |  1209 |      59680 | ENOV   |           |            |             8 | t      | 1971-06-01 00:00:00
const char *stations =
		"INSERT INTO station VALUES(59680, 62.181, 6.0807, 74, 0, 'ØRSTA-VOLDA LUFTHAVN', 1209, 59680, 'ENOV', NULL, NULL, 8, 't', '1971-06-01 00:00:00');";


}



class KlDecoderTest : public testing::Test
{

protected:
   string dbId;
   string testdb;
   DriverManager dbMgr;
   dc::DecoderMgr decoderMgr;
   string decoderBaseTestDir;
   string testdir;
   string dbdir;
   string decoderdir;


   ParamList        paramList;
   KvTypeList typesList;

   ///Called before each test case.
   virtual void SetUp() {
	   testdb= TESTDB;
      testdir = TESTDIR;
      dbdir = DBDIR;
      decoderdir = DECODERDIR;
      decoderBaseTestDir = DECODERBASE_TESTDIR;
      decoderMgr.setDecoderPath( decoderdir );

      if( dbId.empty() ) {
         ASSERT_TRUE( dbMgr.loadDriver( dbdir+"/sqlite3driver.so", dbId ) )<<
         "Failed to load Db driver. Reason: " << dbMgr.getErr();
      }

      if( paramList.empty() ) {
         ASSERT_TRUE( ReadParamsFromFile( decoderBaseTestDir+"/kvparams.csv", paramList ) ) <<
         "Cant read params from the file <kvparams.csv>";
      }

      if( typesList.empty() ) {
    	  ASSERT_TRUE( ReadTypesFromFile(decoderBaseTestDir+"/kvtypes.csv", typesList) ) <<
    	           "Cant read types from the file <kvtypes.csv>";
      }

     setUpDb();
   }


   void createConfSection( const string &programName,
		                                         const string &decoderName,
		                                         miutil::conf::ConfSection *&conf)
   {

	   conf = new miutil::conf::ConfSection();
	   miutil::conf::ConfSection *programConf = new miutil::conf::ConfSection();
	   miutil::conf::ConfSection *tmp = new miutil::conf::ConfSection();

	   ASSERT_TRUE( programConf->addSection( decoderName, tmp ) );
	   ASSERT_TRUE( conf->addSection( programName, programConf ) );

	   string sectionName = programName+"."+decoderName;
	   miutil::conf::ConfSection *hasSection = conf->getSection( sectionName );

	   ASSERT_TRUE( hasSection ) << "Can't create conf section '" << sectionName  << "'.";
   }


   void setUpDb() {
	   unlink( TESTDB );
	   dnmi::db::Connection *con = dbMgr.connect( dbId, testdb );
	   ASSERT_TRUE( con != 0 )<< "Cant open database connection: " << testdb << ".";
	   ASSERT_NO_THROW( con->exec( schemaKvStation ) ) << "DB: cant create table 'stations'.";
	   cerr << stations << endl;
	   ASSERT_NO_THROW( con->exec( stations ) ) << "DB: cant insert into table 'stations'.";


   }

   bool getData( const KvDataContainer::DataList &dataList, kvalobs::kvData &data,
		   int stationid, int typeId, int paramid, int sensor=0, int level=0) const
   {
	   KvDataContainer::DataList::const_iterator it = dataList.begin();

	   for( ; it != dataList.end(); ++it ) {
		   if( it->stationID() == stationid &&
			   it->typeID() == typeId &&
			   it->paramID() == paramid &&
			   it->sensor() == sensor  &&
			   it->level() == level ) {
			   data = *it;
			   return true;
		   }
	   }

	   return false;
   }

   bool getTextData( const KvDataContainer::TextDataList &dataList, kvalobs::kvTextData &data,
   		   int stationid, int typeId, int paramid ) const
      {
   	   KvDataContainer::TextDataList::const_iterator it = dataList.begin();

   	   for( ; it != dataList.end(); ++it ) {
   		   if( it->stationID() == stationid &&
   			   it->typeID() == typeId &&
   			   it->paramID() == paramid ) {
   			   data = *it;
   			   return true;
   		   }
   	   }

   	   return false;
      }

   ///Called after each test case.
   virtual void TearDown() {
	   //cerr << "TearDown:\n";

   }
};

TEST_F( KlDecoderTest, DataDecodeTest )
{
	string error;
	string filename;
	string obsType;
	string obsData;
	string header;
	KvTypeList types;
	KlDataArray klData;
	int useinfo7; //tolate/toearly flag
	conf::ConfSection *conf=0;
	list<string> strParams;
	list<string> expectedStrParams;
	string message;
	pt::ptime obstime;
	pt::ptime receivedTime;
	kvalobs::serialize::KvalobsData *data;

	types.push_back(kvTypes(311,"",60, 60,"I","h","For test"));

	filename = testdir+"/n59680-t311.dat";
	ASSERT_TRUE( ReadDataFromFile( filename, obsType, obsData ) )<< "Cant read testdata: " << filename << ".";
	ASSERT_TRUE( ! obsType.empty() && ! obsData.empty() ) << "Invalid datafile format: " << filename << ".";

	header="AA,DD,DX_1";
	vector<ParamDef> definedParams;

	bits::DataDecoder decoder( paramList, typesList );


	ASSERT_TRUE( decoder.splitParams( header, strParams, message) ) << "Cant split params '" << header << "'.";
	ba::push_back( expectedStrParams )("AA")("DD")("DX_1");
	ASSERT_TRUE( equal( strParams.begin(), strParams.end(), expectedStrParams.begin()  ) );


	header="AA(1),DD(1,0),DX_1(1,2)";
	ASSERT_TRUE( decoder.splitParams( header, strParams, message) ) << "Cant split params '" << header << "'.";
	expectedStrParams.clear();
	ba::push_back( expectedStrParams )("AA(1)")("DD(1,0)")("DX_1(1,2)");
	ASSERT_TRUE( equal( strParams.begin(), strParams.end(), expectedStrParams.begin()  ) );

	header="AA,DD,DX_1";
	ASSERT_TRUE( decoder.decodeHeader( header, definedParams, message ) );
	ASSERT_TRUE( definedParams.size() == 3 );
	ASSERT_TRUE( definedParams[0]== ParamDef( "AA", 1 ) );
	ASSERT_TRUE( definedParams[1]== ParamDef( "DD", 61 ) );
	ASSERT_TRUE( definedParams[2]== ParamDef( "DX_1",73) );

	header="AA(1),DD,DX_1(1,2)";
	ASSERT_TRUE( decoder.decodeHeader( header, definedParams, message ) )
		<< "Cant decode header: " << message;

	ASSERT_TRUE( definedParams[0]== ParamDef( "AA", 1, 1, 0 ) );
	ASSERT_TRUE( definedParams[1]== ParamDef( "DD", 61, 0, 0 ) );
	ASSERT_TRUE( definedParams[2]== ParamDef( "DX_1" ,73, 1, 2) );

	header="AA,DD,DX_1";
	ASSERT_TRUE( decoder.decodeHeader( header, definedParams, message ) );
	obsData = "201310051000,3,276,254";

	ASSERT_TRUE( decoder.decodeData( klData, definedParams.size(), obstime, receivedTime, 311, obsData, 2, message) );
    ASSERT_TRUE( obstime == pt::time_from_string_nothrow("201310051000") );
    ASSERT_TRUE( klData[0] == KlData("3") );
    ASSERT_TRUE( klData[1] == KlData("276")  );
    ASSERT_TRUE( klData[2] == KlData("254") );

    //Set useinfo and controlinfo flag from data.
    obsData = "201310051000,3(,xxxxxxxxxxxxx6xx),276,254";
    ASSERT_TRUE( decoder.decodeData( klData, definedParams.size(), obstime, receivedTime, 311, obsData, 2, message) );
    ASSERT_TRUE( obstime == pt::time_from_string_nothrow("201310051000") );
    ASSERT_TRUE( klData[0] == KlData("3", "0000000000000000","9999900000000600") );
    ASSERT_TRUE( klData[1] == KlData("276")  );
    ASSERT_TRUE( klData[2] == KlData("254") );

    //Set useinfo(7)=0 on time.
    receivedTime = pt::time_from_string_nothrow("2013-10-05 10:04:56");
    ASSERT_TRUE( ! receivedTime.is_special() );
    obsData = "201310051000,3,276,254";
    ASSERT_TRUE( decoder.decodeData( klData, definedParams.size(), obstime, receivedTime, 311, obsData, 2, message) );
    ASSERT_TRUE( klData[0] == KlData("3") );
    ASSERT_TRUE( klData[1] == KlData("276")  );
    ASSERT_TRUE( klData[2] == KlData("254") );

    //Set useinfo(7)=4 to late
    receivedTime = pt::time_from_string_nothrow("2013-10-05 11:01:56");
    ASSERT_TRUE( ! receivedTime.is_special() );
    obsData = "201310051000,3,276,254";
    ASSERT_TRUE( decoder.decodeData( klData, definedParams.size(), obstime, receivedTime, 311, obsData, 2, message) );
    ASSERT_TRUE( klData[0] == KlData("3",  "0000000000000000","9999900400000000" ) );
    ASSERT_TRUE( klData[1] == KlData("276","0000000000000000","9999900400000000" )  );
    ASSERT_TRUE( klData[2] == KlData("254","0000000000000000","9999900400000000" ) );

    //Set useinfo(7)=3 to early.
    receivedTime = pt::time_from_string_nothrow("2013-10-05 08:59:59");
    ASSERT_TRUE( ! receivedTime.is_special() );
    obsData = "201310051000,3,276,254";
    ASSERT_TRUE( decoder.decodeData( klData, definedParams.size(), obstime, receivedTime, 311, obsData, 2, message) );
    ASSERT_TRUE( klData[0] == KlData("3",  "0000000000000000","9999900300000000" ) );
    ASSERT_TRUE( klData[1] == KlData("276","0000000000000000","9999900300000000" )  );
    ASSERT_TRUE( klData[2] == KlData("254","0000000000000000","9999900300000000" ) );


    //decode data
    //receivedTime = pt::time_from_string_nothrow("2013-10-05 10:04:56");
    receivedTime = pt::time_from_string_nothrow("2013-10-05 08:59:59"); //To early
    ASSERT_TRUE( ! receivedTime.is_special() );
    obstime = pt::time_from_string_nothrow("2013-10-05 10:00:00");
    obsData = "AA,DD,DX_1,signature\n"
    		  "201310051000,3,276,254,bm";

    data =  decoder.decodeData( obsData, 59680, 311, receivedTime, "", "" );
    ASSERT_TRUE( data );
    KvDataContainer dataContainer( data );
    KvDataContainer::Data obsData1;

    ASSERT_TRUE(  dataContainer.getData( obsData1, receivedTime ) == 3 );

    KvDataContainer::DataByObstime obsData2;
    kvalobs::kvData kvData;
    ASSERT_TRUE( dataContainer.getData( obsData2, 59680, 311, receivedTime ) == 3 );

    ASSERT_TRUE( getData( obsData2[obstime], kvData, 59680, 311, 1) );
    ASSERT_FLOAT_EQ( kvData.original(), 3 /* AA */);
    ASSERT_TRUE( kvData.useinfo() == kvUseInfo("9999900300000000") );
    ASSERT_TRUE( kvData.controlinfo() == kvControlInfo() );

    ASSERT_TRUE( getData( obsData2[obstime], kvData, 59680, 311, 61) );
    ASSERT_FLOAT_EQ( kvData.original(), 276 /* DD */);
    ASSERT_TRUE( kvData.useinfo() == kvUseInfo("9999900300000000") );
    ASSERT_TRUE( kvData.controlinfo() == kvControlInfo() );

    ASSERT_TRUE( getData( obsData2[obstime], kvData, 59680, 311, 73) );
    ASSERT_FLOAT_EQ( kvData.original(), 254 /* DX_1 */ );
    ASSERT_TRUE( kvData.useinfo() == kvUseInfo("9999900300000000") );
    ASSERT_TRUE( kvData.controlinfo() == kvControlInfo() );

    KvDataContainer::TextDataByObstime textObsData;
    kvalobs::kvTextData textData;
    ASSERT_TRUE( dataContainer.getTextData( textObsData, 59680, 311, receivedTime ) == 1 );
    ASSERT_TRUE( getTextData( textObsData[obstime], textData, 59680, 311, 1000 /*signature*/) );
    ASSERT_TRUE( textData.original() == "bm" /* signature */);
}


TEST_F( KlDecoderTest, decodeData )
{
	string error;
	string filename;
	string obsType;
	string obsData;
	string header;
	KvTypeList types;
	int useinfo7; //tolate/toearly flag
	conf::ConfSection *conf=0;

	filename = testdir+"/n59680-t311.dat";
	ASSERT_TRUE( ReadDataFromFile( filename, obsType, obsData ) )<< "Cant read testdata: " << filename << ".";
	ASSERT_TRUE( ! obsType.empty() && ! obsData.empty() ) << "Invalid datafile format: " << filename << ".";

	cerr << "testdb: '" << testdb << "'\n";
	dnmi::db::Connection *con = dbMgr.connect( dbId, testdb );
	ASSERT_TRUE( con  )<< "Cant open database connection: " << string(testdb) << ".";

	dc::DecoderBase *dec=decoderMgr.findDecoder( *con, paramList, typesList, obsType, obsData, error);
	ASSERT_TRUE( dec  ) << "Cant create test decoder. obsType: '" << obsType << "'.";


	createConfSection( "kvDataInputd", dec->name(), conf );

	kvalobs::decoder::kldecoder::KlDecoder *klDecoder = static_cast<kvalobs::decoder::kldecoder::KlDecoder*>(dec);
	ASSERT_TRUE( ! klDecoder->getSetUsinfo7() );

	decoderMgr.releaseDecoder( dec );
}


int
main(int argc, char **argv) {
   ::testing::InitGoogleTest(&argc, argv);
   return RUN_ALL_TESTS();
}

