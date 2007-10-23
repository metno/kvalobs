/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: checkrunnertest.h,v 1.1.2.3 2007/09/27 09:02:21 paule Exp $                                                       

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
#ifndef CHECKRUNNERTEST_H
#define CHECKRUNNERTEST_H

#include <cppunit/extensions/HelperMacros.h>
#include <memory>
#include "database/kvalobsdatabase.h"
#include <kvalobs/kvStationInfo.h>

/**
 * @author Vegard Bnes
 */
class CheckRunnerTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE( CheckRunnerTest );
    CPPUNIT_TEST( testNoChecks );
    CPPUNIT_TEST( testNormal );
    CPPUNIT_TEST( testSkipHqc );
    CPPUNIT_TEST( testSkipAggregated );
    CPPUNIT_TEST( testSkipPreChecked );
    CPPUNIT_TEST( testNoSkipedIfOneNotChecked );
    CPPUNIT_TEST( testReCheckResetsFlags );
    CPPUNIT_TEST( testOriginalValueIsInput );
    CPPUNIT_TEST( testCorrectedValueWhenRejected );
    CPPUNIT_TEST( testInsertModelValueSetsCorrectUseinfo );
    CPPUNIT_TEST( testAllTypeidAreInputData );
//     CPPUNIT_TEST( testPrefersCurrentTypeID );
    CPPUNIT_TEST_SUITE_END();

  public:
    CheckRunnerTest();

    ~CheckRunnerTest();

    virtual void setUp();
    virtual void tearDown();

    void testNoChecks();
    void testNormal();
    void testSkipHqc();
    void testSkipAggregated();
    void testSkipPreChecked();
    void testNoSkipedIfOneNotChecked();
    void testReCheckResetsFlags();
    void testOriginalValueIsInput();
    void testCorrectedValueWhenRejected();
    void testInsertModelValueSetsCorrectUseinfo();
    void testAllTypeidAreInputData();
    void testPrefersCurrentTypeID();

  private:
    void runCheckRunner( const std::string & checkName );
    template <typename InsertIterator>  void getData( InsertIterator out );
    std::string getLogPath( const std::string & name ) const;

  private:
    const kvalobs::kvStationInfo stationInfo;
    KvalobsDatabase * db;
};



template <typename InsertIterator>
void CheckRunnerTest::getData( InsertIterator out )
{
  typedef std::auto_ptr<dnmi::db::Result> Result;
  Result res( db->getConnection() ->execQuery( "select * from data" ) );
  while ( res->hasNext() )
    out = res->next();
}


#endif
