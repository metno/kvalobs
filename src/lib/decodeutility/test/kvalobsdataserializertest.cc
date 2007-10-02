/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvalobsdataserializertest.cc,v 1.1.2.2 2007/09/27 09:02:28 paule Exp $                                                       

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
#include "kvalobsdataserializertest.h"
#include "kvalobsdataserializer.h"
#include "kvalobsdataparser.h"
#include <string>

CPPUNIT_TEST_SUITE_REGISTRATION( KvalobsDataSerializerTest );

using namespace std;
using namespace kvalobs;
using namespace kvalobs::serialize;

KvalobsDataSerializerTest::KvalobsDataSerializerTest()
{}

KvalobsDataSerializerTest::~KvalobsDataSerializerTest()
{}

void KvalobsDataSerializerTest::setUp()
{
  kvalobs::kvDataFactory f( 42, "2006-04-26 06:00:00", 302 );
  indata.insert( f.getData( 1.0, 110 ) );
  indata.insert( f.getData( 4, 112 ) );
  indata.insert( f.getData( 3, 18 ) );
  indata.insert( f.getData( 3, 34 ) );
  indata.insert( f.getData( 3, 34, "2006-04-25 18:00:00" ) );

  in = new KvalobsData;
  in->insert( indata.begin(), indata.end() );
}

void KvalobsDataSerializerTest::tearDown()
{
  indata.clear();
  delete in;
}

KvalobsDataSerializerTest::KvalobsDataPtr KvalobsDataSerializerTest::loop()
{
  string xml = KvalobsDataSerializer::serialize( * in );
  KvalobsDataPtr out( new KvalobsData );
  KvalobsDataParser::parse( xml, * out.get() );
  return out;
}

void KvalobsDataSerializerTest::testPreserveKvData()
{
  KvalobsDataPtr out = loop();
  list<kvData> outdata;
  out->getData( outdata );
  DSet outdata_set( outdata.begin(), outdata.end() );

  CPPUNIT_ASSERT( equal( indata.begin(), indata.end(), outdata_set.begin(), compare::exactly_equal_ex_tbtime() ) );
}

void KvalobsDataSerializerTest::testPreserveOverwrite()
{
  KvalobsDataPtr out = loop();
  CPPUNIT_ASSERT( not out->overwrite() );
  in->overwrite( true );
  out = loop();
  CPPUNIT_ASSERT( out->overwrite() );
}

void KvalobsDataSerializerTest::testPreserveInvalidate()
{
  in->invalidate( true, 42, 302, "2006-04-26 06:00:00" );
  in->invalidate( true, 42, 302, "2006-04-25 18:00:00" );
  in->invalidate( true, 42, 302, "2006-04-25 12:00:00" );

  KvalobsDataPtr out = loop();

  CPPUNIT_ASSERT( out->isInvalidate( 42, 302, "2006-04-26 06:00:00" ) );
  CPPUNIT_ASSERT( out->isInvalidate( 42, 302, "2006-04-25 18:00:00" ) );
  CPPUNIT_ASSERT( out->isInvalidate( 42, 302, "2006-04-25 12:00:00" ) );
  CPPUNIT_ASSERT( not out->isInvalidate( 42, 302, "2006-04-26 12:00:00" ) );
}
