/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: ra2rr_12Test.h,v 1.1.2.2 2007/09/27 09:02:16 paule Exp $                                                       

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
#ifndef RA2RR_12TEST_H_
#define RA2RR_12TEST_H_

#include "AbstractAgregatorTest.h"


class ra2rr_12Test : public AbstractAgregatorTest
{
    CPPUNIT_TEST_SUB_SUITE( ra2rr_12Test, AbstractAgregatorTest );
    CPPUNIT_TEST( testExpressedInterest );
    CPPUNIT_TEST( testNotEnoughData );
    CPPUNIT_TEST( testCompleteDataObservationInMiddle );
    CPPUNIT_TEST( testNonStandardDataSet );
    CPPUNIT_TEST( testDataMarkedAsMissing );
    CPPUNIT_TEST( testExpressedInterest );
    CPPUNIT_TEST( test12hZero );
    CPPUNIT_TEST( test12hNegative );
    CPPUNIT_TEST( test12hPositive24hNegative );
    CPPUNIT_TEST( test12hPositive24hZero );
    CPPUNIT_TEST( test12hPositive24hPositivePrev12hNegative );
    CPPUNIT_TEST( test12hPositive24hPositivePrev12hZero );
    CPPUNIT_TEST( test12hPositive24hPositivePrev12hPositive );
	CPPUNIT_TEST_SUITE_END();
    
public:
    ra2rr_12Test();
    virtual ~ra2rr_12Test();

    virtual void setUp();
    virtual void tearDown();

	void testExpressedInterest();
	
	void testNotEnoughData();
	
	void testCompleteDataObservationInMiddle();
	
    void testNonStandardDataSet();
    
	void testDataMarkedAsMissing();

    void test12hZero();
    
    void test12hNegative();

	void test12hPositive24hNegative();
	
	void test12hPositive24hZero();

	void test12hPositive24hPositivePrev12hNegative();

	void test12hPositive24hPositivePrev12hZero();

	void test12hPositive24hPositivePrev12hPositive();

private:
    enum { RR_12 = 109, RA = 104 };
};

#endif /*RA2RR_12TEST_H_*/
