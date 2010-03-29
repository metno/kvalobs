/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: test.cc,v 1.1.2.4 2007/09/27 09:02:21 paule Exp $                                                       

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


#include <milog/milog.h>
#include <gtest/gtest.h>

int main(int argc, char* argv[])
{
	milog::Logger::logger().logLevel( milog::FATAL );

	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}


//#include <cppunit/CompilerOutputter.h>
//#include <cppunit/extensions/TestFactoryRegistry.h>
//#include <cppunit/ui/text/TestRunner.h>
//#include <milog/milog.h>
//
//int main(int argc, char* argv[])
//{
//  CppUnit::TextUi::TestRunner runner;
//  runner.setOutputter( new CppUnit::CompilerOutputter( &runner.result(), std::cerr ) );
//
//  runner.addTest( CppUnit::TestFactoryRegistry::getRegistry().makeTest() );
//
//  milog::Logger::logger().logLevel( milog::FATAL );
//
//  return ! runner.run();
//}
