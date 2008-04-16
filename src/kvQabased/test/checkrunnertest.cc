/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: checkrunnertest.cc,v 1.1.2.5 2007/09/27 09:02:21 paule Exp $                                                       

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
#include "checkrunnertest.h"
#include "../checkrunner.h"
#include <string>
#include <fstream>
#include <iterator>
#include <kvdb/kvdb.h>
#include <vector>
#include <set>
#include <kvalobs/kvDataOperations.h>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/exception.hpp>


CPPUNIT_TEST_SUITE_REGISTRATION( CheckRunnerTest );


using namespace std;
using namespace kvalobs;
using namespace boost::filesystem;

CheckRunnerTest::CheckRunnerTest()
    : stationInfo( 42, "2006-05-26 06:00:00", 302 )
    , db( 0 )
{}


CheckRunnerTest::~CheckRunnerTest()
{}

namespace
{
  std::string getInitSqlStatement() {
    ifstream f( CHECKRUNNERTEST_INIT_SQL );
    if ( ! f )
      throw std::runtime_error( "Test file not found: " CHECKRUNNERTEST_INIT_SQL );
    std::string ret;
    getline( f, ret, char_traits<char>::to_char_type( char_traits<char>::eof() ) );
    return ret;
  }
}

void CheckRunnerTest::setUp()
{
  db = std::auto_ptr<KvalobsDatabase>( new KvalobsDatabase );

  // Setup db:
  static const string init_sql_statement = getInitSqlStatement();
  db->getConnection() ->exec( init_sql_statement );
}


void CheckRunnerTest::tearDown()
{
}


void CheckRunnerTest::runCheckRunner( const std::string & checkName )
{
  runCheckRunner(checkName, stationInfo);
}

void CheckRunnerTest::runCheckRunner( const std::string & checkName, const kvalobs::kvStationInfo & si )
{
  CheckRunner checkRunner( si, * db->getQaBaseConnection(), getLogPath( checkName ) );
  checkRunner();
}

string CheckRunnerTest::getLogPath( const string & name ) const
{
  static const path baseLogPath( "test/var/log/" );
  path ret = baseLogPath / name;
  create_directories( ret );
  return ret.native_directory_string();
}


void CheckRunnerTest::testNoChecks()
{
  kvDataFactory f( 42, "2006-05-26 06:00:00", 302 );
  kvalobs::kvData inData = f.getData( 94, 33 );
  db->getConnection() ->exec( "insert into data values " + inData.toSend() );

  // remove all checks from database:
  db->getConnection() ->exec( "delete from checks" );

  // We merely check that this does not crash or throw any exceptions:
  runCheckRunner( __func__ );
}


void CheckRunnerTest::testNormal()
{
  kvDataFactory f( 42, "2006-05-26 06:00:00", 302 );
  kvalobs::kvData inData = f.getData( 94, 33 );
  db->getConnection() ->exec( "insert into data values " + inData.toSend() );

  runCheckRunner( __func__ );

  vector<kvData> result;
  getData( back_inserter( result ) );

  CPPUNIT_ASSERT_EQUAL( size_t( 1 ), result.size() );
  const kvData & outData = result.front();
  CPPUNIT_ASSERT( kvalobs::rejected( outData ) );
  const kvalobs::kvControlInfo ci = outData.controlinfo();
  //   CPPUNIT_ASSERT_EQUAL( 1, ci.flag( kvalobs::flag::fqclevel ) ); // This flag is no longer in use
  CPPUNIT_ASSERT_EQUAL( 6, ci.flag( kvalobs::flag::fr ) );

  kvalobs::kvUseInfo ui;
  ui.setUseFlags( ci );
  CPPUNIT_ASSERT_EQUAL( ui, outData.useinfo() );
}

namespace
{
  kvalobs::compare::exactly_equal eq_;
}


void CheckRunnerTest::testSkipHqc()
{
  kvDataFactory f( 42, "2006-05-26 06:00:00", 302 );
  kvalobs::kvData inData = f.getData( 94, 33 );
  kvalobs::hqc::hqc_accept( inData );
  db->getConnection() ->exec( "insert into data values " + inData.toSend() );

  runCheckRunner( __func__ );

  vector<kvData> result;
  getData( back_inserter( result ) );

  CPPUNIT_ASSERT_EQUAL( size_t( 1 ), result.size() );
  const kvData & outData = result.front();

//  CPPUNIT_ASSERT( kvalobs::hqc::hqc_accepted( outData ) );
  CPPUNIT_ASSERT( eq_(inData, outData) );
}


void CheckRunnerTest::testSkipAggregated()
{
  kvDataFactory f( 42, "2006-05-26 06:00:00", -302 );
  kvalobs::kvData inData = f.getData( 94, 33 );
  db->getConnection() ->exec( "insert into data values " + inData.toSend() );

  {
    kvalobs::kvStationInfo si( stationInfo.stationID(), stationInfo.obstime(), - stationInfo.typeID() );
    CheckRunner checkRunner( si, * db->getQaBaseConnection(), getLogPath( __func__ ) );
    checkRunner();
  }

  vector<kvData> result;
  getData( back_inserter( result ) );

  CPPUNIT_ASSERT_EQUAL( size_t( 1 ), result.size() );
  const kvData & outData = result.front();

  CPPUNIT_ASSERT( eq_( inData, outData ) );
}


namespace
{
  void setQC1HasRun( kvData & d, flag::ControlFlag f )
  {
    kvalobs::kvControlInfo ci = d.controlinfo();
    ci.set( f, 1 );
    kvalobs::kvUseInfo ui = d.useinfo();
    ui.setUseFlags( ci );
    d.controlinfo( ci );
    d.useinfo( ui );
  }

  kvData getQC1ModifiedData( int val, int param, kvDataFactory & f, flag::ControlFlag fl = flag::fpos )
  {
    kvData ret = f.getData( val, param );
    setQC1HasRun( ret, fl );
    return ret;
  }
}


void CheckRunnerTest::testSkipPreChecked()
{
  kvDataFactory f( 42, "2006-05-26 06:00:00", 302 );
  kvalobs::kvData inData = getQC1ModifiedData( 94, 33, f );
  db->getConnection() ->exec( "insert into data values " + inData.toSend() );

  runCheckRunner( __func__ );

  vector<kvData> result;
  getData( back_inserter( result ) );

  CPPUNIT_ASSERT_EQUAL( size_t( 1 ), result.size() );
  const kvData & outData = result.front();

  CPPUNIT_ASSERT_EQUAL( inData.controlinfo(), outData.controlinfo() );
  CPPUNIT_ASSERT_EQUAL( inData.useinfo(), outData.useinfo() );
  CPPUNIT_ASSERT( eq_( inData, outData ) );
}

void CheckRunnerTest::testNoSkipedIfOneNotChecked()
{
  typedef set<kvData, compare::lt_kvData> Container;

  kvDataFactory f( 42, "2006-05-26 06:00:00", 302 );
  Container dl;

  dl.insert( getQC1ModifiedData( 94, 33, f ) );
  dl.insert( getQC1ModifiedData( 23, 110, f ) );
  dl.insert( getQC1ModifiedData( 1, 112, f ) );
  for ( Container::iterator it = dl.begin(); it != dl.end(); ++ it )
    db->getConnection() ->exec( "insert into data values " + it->toSend() );
  kvData unchecked = f.getData( 42, 42 );
  db->getConnection() ->exec( "insert into data values " + unchecked.toSend() );

  runCheckRunner( __func__ );

  Container result;
  getData( inserter( result, result.begin() ) );

  CPPUNIT_ASSERT_EQUAL( size_t( 4 ), result.size() );

  for ( Container::const_iterator inData = dl.begin(); inData != dl.end(); ++ inData )
  {
    Container::const_iterator outData = result.find( * inData );
    CPPUNIT_ASSERT( outData != result.end() );
    CPPUNIT_ASSERT( kvControlInfo() != outData->controlinfo() );
    //CPPUNIT_ASSERT_ASSERTION_FAIL(
    //  CPPUNIT_ASSERT_EQUAL( kvControlInfo(), outData->controlinfo() )
    //);
    // We assume the bogus checks in the test input was wrong:
    CPPUNIT_ASSERT_ASSERTION_FAIL(
      CPPUNIT_ASSERT_EQUAL( inData->controlinfo(), outData->controlinfo() )
    );
  }
  // See if previously unchecked data has been rechecked
  Container::const_iterator outData = result.find( unchecked );
  CPPUNIT_ASSERT( outData != result.end() );
  CPPUNIT_ASSERT_EQUAL_MESSAGE( "Data has wrong controlinfo/useinfo flags: " + outData->toSend(),
                                7, outData->useinfo().flag( 0 ) );
}


void CheckRunnerTest::testReCheckResetsFlags()
{
  flag::ControlFlag originalFlag = flag::fcc;

  kvDataFactory f( 42, "2006-05-26 06:00:00", 302 );
  kvalobs::kvData inData = getQC1ModifiedData( 94, 33, f, originalFlag );
  db->getConnection() ->exec( "insert into data values " + inData.toSend() );

  {
    CheckRunner checkRunner( stationInfo, * db->getQaBaseConnection(), getLogPath( __func__ ) );
    checkRunner( true );
  }

  vector<kvData> result;
  getData( back_inserter( result ) );

  CPPUNIT_ASSERT_EQUAL( size_t( 1 ), result.size() );
  const kvData & outData = result.front();

  CPPUNIT_ASSERT_EQUAL( 0, outData.controlinfo().flag( originalFlag ) );
}


void CheckRunnerTest::testOriginalValueIsInput()
{
  kvDataFactory f( 42, "2006-05-26 06:00:00", 302 );
  kvalobs::kvData inData = f.getData( 94, 33 );
  inData.corrected( 2 ); // This corrected value won't be rejected
  db->getConnection() ->exec( "insert into data values " + inData.toSend() );

  runCheckRunner( __func__ );

  vector<kvData> result;
  getData( back_inserter( result ) );

  CPPUNIT_ASSERT_EQUAL( size_t( 1 ), result.size() );
  const kvData & outData = result.front();
  CPPUNIT_ASSERT( kvalobs::rejected( outData ) );
}


void CheckRunnerTest::testCorrectedValueWhenRejected()
{
  kvDataFactory f( 42, "2006-05-26 06:00:00", 302 );
  kvalobs::kvData inData = f.getData( 94, 33 );
  db->getConnection() ->exec( "insert into data values " + inData.toSend() );

  runCheckRunner( __func__ );

  vector<kvData> result;
  getData( back_inserter( result ) );

  CPPUNIT_ASSERT_EQUAL( size_t( 1 ), result.size() );
  const kvData & outData = result.front();
  CPPUNIT_ASSERT( kvalobs::rejected( outData ) );
  CPPUNIT_ASSERT_EQUAL( float( -32766 ), outData.corrected() );
}

void CheckRunnerTest::testInsertModelValueSetsCorrectUseinfo()
{
  kvDataFactory f( 42, "2006-05-26 06:00:00", 302 );
  kvalobs::kvData inData = f.getMissing( 110 );
  db->getConnection() ->exec( "insert into data values " + inData.toSend() );

  runCheckRunner( __func__ );

  vector<kvData> result;
  getData( back_inserter( result ) );

  CPPUNIT_ASSERT_EQUAL( size_t( 1 ), result.size() );
  const kvData & outData = result.front();

  CPPUNIT_ASSERT( kvalobs::original_missing( outData ) );
  CPPUNIT_ASSERT( kvalobs::valid( outData ) );

  kvUseInfo real_ui = outData.useinfo();
  kvUseInfo correct_ui;
  correct_ui.setUseFlags( outData.controlinfo() );
  CPPUNIT_ASSERT_EQUAL( correct_ui, real_ui );
}

void CheckRunnerTest::testAllTypeidAreInputData()
{
  kvDataFactory f1( 42, "2006-05-26 06:00:00", 302 );
  kvalobs::kvData inData1 = f1.getData( 1.0, 111 );
  db->getConnection() ->exec( "insert into data values " + inData1.toSend() );

  kvDataFactory f2( 42, "2006-05-26 06:00:00", 322 );
  kvalobs::kvData inData2 = f2.getData( 2.0, 110 );
  db->getConnection() ->exec( "insert into data values " + inData2.toSend() );
  
  runCheckRunner( __func__ );
  
  typedef std::set<kvalobs::kvData, kvalobs::compare::lt_kvData> ResultContainer;
  ResultContainer result;
  getData( inserter( result, result.begin() ) );

  CPPUNIT_ASSERT_EQUAL( size_t( 2 ), result.size() );
  
  ResultContainer::const_iterator it = result.find( inData2 );
  CPPUNIT_ASSERT( it != result.end() );
  CPPUNIT_ASSERT_EQUAL( kvControlInfo( "1000300000000000" ), it->controlinfo() );
}

void CheckRunnerTest::testPrefersCurrentTypeID()
{
  // Warning: 
  // This test may incorrectly pass if the underlying database happens to deliver 
  // inData1 to the checks before inData2.
  
  kvDataFactory f1( 42, "2006-05-26 06:00:00", 302 );
  kvalobs::kvData inData1 = f1.getData( 1.0, 110 );
  db->getConnection() ->exec( "insert into data values " + inData1.toSend() );

  kvDataFactory f2( 42, "2006-05-26 06:00:00", 300 );
  kvalobs::kvData inData2 = f2.getData( 2.0, 110 );
  db->getConnection() ->exec( "insert into data values " + inData2.toSend() );

  kvDataFactory f3( 42, "2006-05-26 06:00:00", 307 );
  kvalobs::kvData inData3 = f3.getData( 2.0, 110 );
  db->getConnection() ->exec( "insert into data values " + inData3.toSend() );

    
  runCheckRunner( __func__ );
  
  typedef std::set<kvalobs::kvData, kvalobs::compare::lt_kvData> ResultContainer;
  ResultContainer result;
  getData( inserter( result, result.begin() ) );

  CPPUNIT_ASSERT_EQUAL( size_t( 3 ), result.size() );
  
  ResultContainer::const_iterator it = result.find( inData1 );
  CPPUNIT_ASSERT( it != result.end() );
  CPPUNIT_ASSERT_EQUAL( kvControlInfo( "1000300000000000" ), it->controlinfo() );
  
  it = result.find( inData2 );
  CPPUNIT_ASSERT( it != result.end() );
  CPPUNIT_ASSERT_EQUAL( kvControlInfo(), it->controlinfo() );  

  it = result.find( inData3 );
  CPPUNIT_ASSERT( it != result.end() );
  CPPUNIT_ASSERT_EQUAL( kvControlInfo(), it->controlinfo() );  
}

namespace
{
  struct hasTypeId
  {
    const int wanted;
    hasTypeId(int wantedTypeId) : wanted(wantedTypeId) {}
    bool operator () (const kvalobs::kvData & d ) {
      return d.typeID() == wanted;
    }
  };  
}


void CheckRunnerTest::testHandlesSpecifiedTypeidInChecks()
{
  // This differs from testPrefersCurrentTypeID in that the checks table 
  // specifies that the checks for paramid 111 only applies to typeid 303.
  
  kvDataFactory fa( 9, "2006-05-26 06:00:00", 102 );
  const kvalobs::kvData bogusDataA = fa.getData( 0, 111 );
  db->getConnection() ->exec( "insert into data values " + bogusDataA.toSend() );

  kvDataFactory fb( 9, "2006-05-26 06:00:00", 303 );
  const kvalobs::kvData inData = fb.getData( 100000, 111 );
  db->getConnection() ->exec( "insert into data values " + inData.toSend() );

  kvDataFactory fc( 9, "2006-05-26 06:00:00", 304 );
  const kvalobs::kvData bogusDataB = fc.getData( 0, 111 );
  db->getConnection() ->exec( "insert into data values " + bogusDataB.toSend() );

  
  kvalobs::kvStationInfo si( 9, "2006-05-26 06:00:00", 303 );
  runCheckRunner( __func__, si );
  
  vector<kvData> result;
  getData( back_inserter( result ) );
  CPPUNIT_ASSERT_EQUAL( size_t( 3 ), result.size() );

  vector<kvData>::const_iterator find;
  
  find = std::find_if(result.begin(), result.end(), hasTypeId(303));
  CPPUNIT_ASSERT(find != result.end());
  CPPUNIT_ASSERT_EQUAL(4, find->controlinfo().flag(kvalobs::flag::fnum));
  
  find = std::find_if(result.begin(), result.end(), hasTypeId(102));
  CPPUNIT_ASSERT(find != result.end());
  CPPUNIT_ASSERT(eq_(bogusDataA, * find));
  
  find = std::find_if(result.begin(), result.end(), hasTypeId(304));
  CPPUNIT_ASSERT(find != result.end());
  CPPUNIT_ASSERT(eq_(bogusDataB, * find));
}

void CheckRunnerTest::testDoesNotCheckWhenChecksSpecifyAnotherTypeid()
{
  kvDataFactory fa( 9, "2006-05-26 06:00:00", 302 );
  const kvalobs::kvData inData = fa.getData( 21, 111 );
  db->getConnection()->exec( "insert into data values " + inData.toSend() );
    
  kvalobs::kvStationInfo si( 9, "2006-05-26 06:00:00", 302 );
  runCheckRunner( __func__, si );
 
  vector<kvData> result;
  getData( back_inserter( result ) );
  CPPUNIT_ASSERT_EQUAL( size_t( 1 ), result.size() );
  
  const kvData & outData = result.front();
  
  CPPUNIT_ASSERT_EQUAL(inData.controlinfo(), outData.controlinfo());
  CPPUNIT_ASSERT(eq_(inData, outData));
}

void CheckRunnerTest::testChecksHighLevels()
{
  kvDataFactory f(42, "2006-05-26 06:00:00", 302, 0, 25 );
  kvData d = f.getData(5, 110);
  db->getConnection()->exec( "insert into data values " + d.toSend() );

  runCheckRunner( __func__ );

  vector<kvData> result;
  getData( back_inserter( result ) );

//  for ( vector<kvData>::const_iterator it = result.begin(); it != result.end(); ++ it )
//    cout << * it << endl;
  
  CPPUNIT_ASSERT_EQUAL( size_t( 1 ), result.size() );
  CPPUNIT_ASSERT( result.front().controlinfo() != kvControlInfo() );
}

void CheckRunnerTest::testChecksNonstandardSensor()
{
  kvDataFactory f(42, "2006-05-26 06:00:00", 302, 1, 0 );
  kvData d = f.getData(5, 110);
  db->getConnection()->exec( "insert into data values " + d.toSend() );

  runCheckRunner( __func__ );

  vector<kvData> result;
  getData( back_inserter( result ) );

//  for ( vector<kvData>::const_iterator it = result.begin(); it != result.end(); ++ it )
//    cout << * it << endl;
  
  CPPUNIT_ASSERT_EQUAL( size_t( 1 ), result.size() );
  CPPUNIT_ASSERT( result.front().controlinfo() != kvControlInfo() );
}

void CheckRunnerTest::testOnlyUsesSpecificLevel()
{
  kvDataFactory f0(42, "2006-05-26 06:00:00", 302);
  kvData d0a = f0.getData(1, 112);
  db->getConnection()->exec( "insert into data values " + d0a.toSend() );
  // don't send this to the database:
  kvData d0b = f0.getMissing(42);
  db->getConnection()->exec( "insert into data values " + d0b.toSend() );
  
  kvDataFactory f25(42, "2006-05-26 06:00:00", 302, 0, 25 );
  kvData d25a = f25.getData(2, 112);
  db->getConnection()->exec( "insert into data values " + d25a.toSend() );
  kvData d25b = f25.getData(2, 42);
  db->getConnection()->exec( "insert into data values " + d25b.toSend() );

  runCheckRunner( __func__ );

  vector<kvData> result;
  getData( back_inserter( result ) );
  
  for ( vector<kvData>::const_iterator it = result.begin(); it != result.end(); ++ it )
    cout << * it << endl;
  
  vector<kvData>::const_iterator L0 = find_if(result.begin(), result.end(), 
      std::bind2nd( kvalobs::compare::same_kvData(), d0a ) );
  CPPUNIT_ASSERT( L0 != result.end() );
  CPPUNIT_ASSERT_EQUAL( 8, L0->controlinfo().flag(2) );
  // 9 probably means level 0 data was checked against level 25 data
  // 0 means check was skipped, or that test results were not stored
  
  
  vector<kvData>::const_iterator L25 = find_if(result.begin(), result.end(), 
      std::bind2nd( kvalobs::compare::same_kvData(), d25a ) );
  CPPUNIT_ASSERT( L25 != result.end() );
  CPPUNIT_ASSERT_EQUAL( 2, L25->controlinfo().flag(2) );
}
