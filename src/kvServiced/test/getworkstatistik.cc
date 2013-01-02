/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: getdata.cc,v 1.1.2.2 2007/09/27 09:02:41 paule Exp $                                                       

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
#include <iostream>
#include <kvskel/kvService.hh>
#include <corbahelper/corbaApp.h>
#include <puTools/miTime.h>
#include <kvcpp/corba/CorbaKvApp.h>

using namespace CorbaHelper;
using namespace std;
using namespace CKvalObs::CService;
using namespace miutil;


std::string sconf=
"loglevel=INFO\n"
"tracelevel=DEBUG\n"
"\n"
"corba{\n"
"  #CORBA nameserver to be used.\n"
"   nameserver=\"localhost\"\n"
"\n"
"   #Which kvalobs servers shall we send observations to.\n"
"   path=(\"kv-conan\")\n"
"}\n";

int
main(int argn, char **argv)
{
   istringstream iconf( sconf );
   miutil::conf::ConfParser theParser;



   if( argn < 2 ) {
      cerr << "You must specify the configuration file." << endl;
      cerr << "[" << endl << sconf << endl << "]" << endl;
      cerr << "Cant read the configuration." << endl
           << theParser.getError() << endl;
      return 1;
   }
   miutil::conf::ConfSection *conf = theParser.parse( argv[1] );

   if( ! conf ) {
      cerr << "Cant parse the configurationfile: "<< argv[1] << endl;
      cerr << "Reason:" << theParser.getError() << endl;
      return 1;
   }

   kvservice::corba::CorbaKvApp app( argn, argv, conf );
	
	try {
	   kvalobs::kvWorkelement data;
	   kvservice::WorkstatistikIterator wit;
	   boost::posix_time::ptime to = boost::posix_time::second_clock::universal_time();
	   boost::posix_time::ptime from = to - boost::posix_time::hours(2);

	   cerr << "Workstatistik (obstime): " << from << " - " << to << endl;

	   if( ! app.getKvWorkstatistik( ObsTime, from, to, wit) ) {
	      cerr << "Failed to get workstatistik data!" << endl;
	      return 1;
	   }

	   while( wit.next( data ) ) {
	      cerr << data << endl;
	   }

	} catch (...) {
		cerr << "Exception while getData..." << endl;
		return 1;		
	}
	
	cerr << endl << endl << "Ok!" << endl;
}
