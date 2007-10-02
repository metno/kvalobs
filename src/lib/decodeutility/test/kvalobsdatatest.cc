/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvalobsdatatest.cc,v 1.1.2.2 2007/09/27 09:02:28 paule Exp $                                                       

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
#include "kvalobsdatatest.h"
#include <kvalobs/kvDataOperations.h>
#include <kvalobs/kvTextDataOperations.h>

CPPUNIT_TEST_SUITE_REGISTRATION( KvalobsDataTest );

using namespace std;
using namespace kvalobs;
using namespace kvalobs::serialize;

KvalobsDataTest::KvalobsDataTest()
{}

KvalobsDataTest::~KvalobsDataTest()
{}

void KvalobsDataTest::setUp()
{
  data = new KvalobsData;
}

void KvalobsDataTest::tearDown()
{
  delete data;
}

void KvalobsDataTest::testContructor()
{
  CPPUNIT_ASSERT( not data->overwrite() );
  CPPUNIT_ASSERT( data->empty() );
  CPPUNIT_ASSERT_EQUAL( size_t( 0 ), data->size() );

  list<kvData> d;
  list<kvTextData> td;
  data->getData( d, td );
  CPPUNIT_ASSERT( d.empty() );
  CPPUNIT_ASSERT( td.empty() );

  list<KvalobsData::InvalidateSpec> inv;
  data->getInvalidate( inv );
  CPPUNIT_ASSERT( inv.empty() );
}

void KvalobsDataTest::testInsertKvData()
{
  kvalobs::kvDataFactory f( 42, "2006-04-26 06:00:00", 302 );
  kvData d = f.getData(  0.1, 110 );
  data->insert( d );
  list<kvData> out;
  data->getData( out );

  CPPUNIT_ASSERT( not data->empty() );
  CPPUNIT_ASSERT_EQUAL( size_t( 1 ), data->size() );
  CPPUNIT_ASSERT_EQUAL( size_t( 1 ), out.size() );
  CPPUNIT_ASSERT( compare::exactly_equal_ex_tbtime()( d, out.front() ) );
}

void KvalobsDataTest::testInsertKvTextData()
{
  kvTextDataFactory f( 42, "2006-04-26 06:00:00", 302 );
  kvTextData d = f.getData( "FOO", 1021 );
  data->insert( d );
  list<kvTextData> out;
  data->getData( out );

  CPPUNIT_ASSERT( not data->empty() );
  CPPUNIT_ASSERT_EQUAL( size_t( 1 ), data->size() );
  CPPUNIT_ASSERT_EQUAL( size_t( 1 ), out.size() );
  CPPUNIT_ASSERT( compare::kvTextData_exactly_equal_ex_tbtime()( d, out.front() ) );
}

void KvalobsDataTest::testInvalidate()
{
  int st = 4;
  int tp = 5;
  miutil::miTime ot = "2006-04-26 06:00:00";
  data->invalidate( true, st, tp, ot );

  CPPUNIT_ASSERT( data->isInvalidate( st, tp, ot ) );

  list<KvalobsData::InvalidateSpec> inv;
  data->getInvalidate( inv );

  CPPUNIT_ASSERT_EQUAL( size_t( 1 ), inv.size() );

  const KvalobsData::InvalidateSpec & i = inv.front();
  CPPUNIT_ASSERT_EQUAL( st, i.station );
  CPPUNIT_ASSERT_EQUAL( tp, i.typeID );
  CPPUNIT_ASSERT_EQUAL( ot, i.obstime );
}
