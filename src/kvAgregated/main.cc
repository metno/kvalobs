/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: main.cc,v 1.4.2.9 2007/09/27 09:02:15 paule Exp $                                                       

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
#include <kvcpp/corba/CorbaKvApp.h>
#include "AgregatorHandler.h"
#include "BackProduction.h"
#include "proxy/KvalobsProxy.h"
#include <kvalobs/kvStation.h>
#include <milog/milog.h>
#include <milog/FLogStream.h>
#include <puTools/miClock.h>
#include <kvdb/dbdrivermgr.h>
#include <set>
#include <fileutil/pidfileutil.h>
#include <kvalobs/kvPath.h>
#include <boost/thread/thread.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>

#include "minmax.h"
#include "rr.h"
#include "ra2rr_12.h"

using namespace std;
using namespace agregator;
using namespace miutil;
using namespace milog;
using namespace dnmi::db;
namespace opt = boost::program_options;

typedef kvservice::corba::CorbaKvApp KvApp;
// typedef kvservice::KvApp KvApp;

namespace
{
void version(std::ostream & out)
{
  out << "kvAgregated (kvalobs) " << VERSION << endl;
}

void help(std::ostream & out, const boost::program_options::options_description & options )
{
  version(out);
  out << "\nData agregation daemon for kvalobs.\n\n" << options << endl;
}

void runDaemon(kvservice::proxy::KvalobsProxy & proxy)
{
    proxy.start_thread();
    
    const set<int> &inter = proxy.getInteresting();
    for ( set<int>::const_iterator it = inter.begin(); it != inter.end(); it++ )
      cerr << *it << " ";
    cerr << endl;
    
    LOGINFO("Starting main loop");
    KvApp::kvApp->run();
}

void populate(std::vector<int> & out, const std::string & elementsSpec)
{
  using namespace boost;
  
  split_iterator<string::const_iterator> it = make_split_iterator(elementsSpec, first_finder(","));
  for ( ; it != split_iterator<string::const_iterator>(); ++ it )
    out.push_back(lexical_cast<int>(*it));
}

}

int main( int argc, char **argv )
{
  opt::options_description options("Available options");
  
  opt::options_description mode("Working mode");
  std:string backProduction;
  bool daemonMode;
  std::string stationsSpec;
  std::string parameterSpec;
  mode.add_options()
  ("back-production,b", opt::value<std::string>(& backProduction), 
      "Produce data according to the given specification. Format for specification is '2008-04-08T06:00:00,5', which means produce data valid for period 2008-04-08T06:00:00 - 2008-04-08T11:00:00. Daemon mode will not be entered if this option is given.")
  ("daemon-mode,d", opt::bool_switch(& daemonMode),
      "Enter daemon mode, even if overridden by the --back-production option.")
  ("stations,s", opt::value(& stationsSpec), "Only process stations from the given comma-separated list.")
  ("parameter,p", opt::value(& parameterSpec), "Only process parameters from the given comma-separated list.")
  ;
  options.add(mode);
  
  opt::options_description database("Database");
  bool repopulate;
  database.add_options()
  ("repopulate,r", opt::bool_switch(& repopulate), "Repopulate internal agragator database on startup");
  options.add(database);
  
  opt::options_description general("General");
  general.add_options()
  ("help", "Get help message")
  ("version", "Display version information");
  options.add(general);  
  
  opt::variables_map o;
  
  opt::parsed_options parsed = 
    opt::command_line_parser(argc, argv).options(options).allow_unregistered().run();  
  opt::store(parsed, o);
  opt::notify(o);

  if ( o.find("help") != o.end() ) {
    help(cout, options);
    return 0;
  }
  if ( o.find("version") != o.end() ) {
    version(cout);
    return 0;
  }
  
  std::vector<int> stations;
  try {
    populate(stations, stationsSpec);
  }
  catch ( std::exception & ) {
    cout << "Invalid format: " << stationsSpec << endl;
    cout << "Example format for station specification is 180,320,400" << endl;
    return 1;
  }
  
  std::vector<int> parameters;
  try {
    populate(parameters, parameterSpec);
  }
  catch ( std::exception & ) {
    cout << "Invalid format: " << stationsSpec << endl;
    cout << "Example format for parameter specification is 104,109" << endl;
    return 1;
  }

	
  //PID-file
  const std::string pidfile=kvPath("localstatedir")+"/run/kvAgregated.pid";
  if ( backProduction.empty() || daemonMode ) {
      bool pidfileError;
      if(dnmi::file::isRunningPidFile(pidfile, pidfileError)){
        if(pidfileError){
          LOGFATAL("An error occured while reading the pidfile:" << endl
    	       << pidfile << " remove the file if it exist and"
    	       << endl << "<agregate> is not running. " << 
    	       "If it is running and there is problems. Kill <agregate> and"
    	       << endl << "restart it." << endl << endl);
          return 1;
        }else{
          LOGFATAL("Is <agregate> allready running?" << endl
    	       << "If not remove the pidfile: " << pidfile);
          return 1;
        }
      }
  }
  dnmi::file::PidFileHelper pidFile;


  // Logging
  FLogStream fine( 9, 1024 * 1024 );
  fine.open(kvPath("localstatedir") + "/log/kvAgregated.log");
  fine.loglevel( INFO );
  FLogStream error( 9 );
  error.open(kvPath("localstatedir") + "/log/kvAgregated.warn.log");
  error.loglevel( WARN );
  LogManager::instance()->addStream( &fine );
  LogManager::instance()->addStream( &error );

  
  // Proxy database
  DriverManager manager;
  std::string proxyID;
  const string dbDriverPath = kvPath("pkglibdir") + "/db/";
  if ( !manager.loadDriver(dbDriverPath + "sqlitedriver.so", proxyID) ) {
    LOGFATAL( "Error when loading database driver: " << manager.getErr() );
    return 2;
  }
  Connection *dbConn = manager.connect(proxyID, 
                                       kvPath("localstatedir") + "/agregate/database.sqlite");
  if ( ! dbConn ) {
    LOGFATAL("Cant create a database connection to: " 
	     << endl << kvPath("localstatedir") + "/agregate/database.sqlite");
    return 1;
  }

  
  // Configuration file
  //string myconf=kvdir + "etc/" + "kvAgregated.conf";
  string myconf = "kvAgregated.conf";
  miutil::conf::ConfSection *confSec = KvApp::readConf(myconf);
  if(!confSec){
    myconf=kvPath("sysconfdir") + "/kvalobs.conf";
    confSec = KvApp::readConf(myconf);
  }
  if(!confSec){
    LOGFATAL("Cant open conf file: " << myconf);
    return 1;
  }

  // PID-file creation
  if ( backProduction.empty() || daemonMode )
    pidFile.createPidFile(pidfile);

  
  // Core objects
  KvApp app( argc, argv, confSec );
  kvservice::proxy::KvalobsProxy proxy( *dbConn, stations, false );
  AgregatorHandler handler( proxy );
  handler.setParameterFilter(parameters);
  if ( repopulate )
    proxy.db_repopulate();


  
  // Standard times
  set<miClock> six;
  six.insert( miClock( 6,0,0) );
  six.insert( miClock(18,0,0) );

  // Add handlers
  MinMax tan12 = min( TAN, TAN_12, 12, six );
  handler.addHandler( &tan12 );
  MinMax tax12 = max( TAX, TAX_12, 12, six );
  handler.addHandler( &tax12 );
  MinMax tgn12 = min( TGN, TGN_12, 12, six );
  handler.addHandler( &tgn12 );
  if ( backProduction.empty() || daemonMode ) {
    rr_1  rr1  = rr_1();
    handler.addHandler( &rr1  );
  }
  rr_12 rr12 = rr_12();
  handler.addHandler( &rr12 );
  rr_24 rr24 = rr_24();
  handler.addHandler( &rr24 );
  ra2rr_12_backward ra2rr_b;
  handler.addHandler( &ra2rr_b );
  ra2rr_12_forward  ra2rr_f;
  handler.addHandler( &ra2rr_f );
  
  // Backproduction instead of daemon mode?
  try {
    if ( ! backProduction.empty() ) {
      BackProduction back(proxy, backProduction);
      if ( ! daemonMode ) {
	back();
	return 0;
      }
      else {
	boost::thread t( back );
	runDaemon(proxy);
	t.join();
      }
    }
    else {
      miutil::miTime to( 
	  miutil::miDate::today(), 
	  miutil::miClock(miutil::miClock::oclock().hour(), 0, 0) );
      miutil::miTime from( to );
      from.addHour( -3 );
      
      BackProduction back(proxy, from, to);
      boost::thread t( back );
      runDaemon(proxy);
      t.join();
    }
  }
  catch (std::exception & e ) {
    LOGFATAL(e.what());
    return 1;
  }

  return 0;
}
