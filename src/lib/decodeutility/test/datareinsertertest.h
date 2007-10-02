/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: datareinsertertest.h,v 1.1.2.2 2007/09/27 09:02:27 paule Exp $                                                       

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
#ifndef DATAREINSERTERTEST_H
#define DATAREINSERTERTEST_H

#include <cppunit/extensions/HelperMacros.h>
#include "DataReinserter.h"

/**
 * @author Vegard Bnes
 *
 * @warning atm this test ONLY exists to verify that DataReinserter can compile properly.
*/
class DataReinserterTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE( DataReinserterTest );
    CPPUNIT_TEST( warning );
    CPPUNIT_TEST_SUITE_END();
  public:
    DataReinserterTest();

    ~DataReinserterTest();

    virtual void setUp();
    virtual void tearDown();

    void warning();

  private:

    struct DummyApp
    {
      CKvalObs::CDataSource::Result_var sendDataToKv( const char *, const char * ) { return CKvalObs::CDataSource::Result_var(); }
    };

    typedef kvalobs::DataReinserter<DummyApp> DataReinserter;
    DataReinserter * reinserter;
};

#endif
