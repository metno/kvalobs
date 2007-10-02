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
#include <kvservice/kvcpp2/corba/CorbaKvApp.h>
#include "AgregatorHandler.h"
#include "proxy/KvalobsProxy.h"
#include <kvalobs/kvStation.h>
#include <milog/milog.h>
#include <milog/FLogStream.h>
#include <puTools/miClock.h>
#include <db/dbdrivermgr.h>
#include <set>
#include <fileutil/pidfileutil.h>

#include <boost/thread/thread.hpp>

#include "minmax.h"
#include "rr.h"
#include "ra2rr_12.h"

using namespace std;
using namespace agregator;
using namespace miutil;
using namespace milog;
using namespace dnmi::db;

typedef kvservice::corba::CorbaKvApp KvApp;
// typedef kvservice::KvApp KvApp;

/**
 * Agregates backwards in time.
 * This is run in a thread from the main method.
 */
struct backprod 
{
  kvservice::proxy::KvalobsProxy &proxy;
  const unsigned int hoursback;

  /**
   * Constructor - sets up specifications for what to run. Does not start any 
   * agregation. operator() does that.
   *
   * @param proxy_ engine for processing data.
   * @hoursback_ agregate data for hoursback_ hours.
   */
  backprod( kvservice::proxy::KvalobsProxy & proxy_, unsigned int hoursback_ = 1 ) 
    : proxy(proxy_), hoursback(hoursback_) 
  {
  }

  /**
   * Start agregation.
   */
  void operator()() {
    miTime now( miDate::today(), miClock(miTime::nowTime().hour(),0,0) );
    now.addHour();
    miTime oldest = now;
    oldest.addHour( - (hoursback +1) );
    for ( miTime to = now; to > oldest; to.addHour( -1 ) ) {
      miTime from = to;
      from.addHour( -1 );      
      proxy.processData( from, to );
      if ( proxy.stopping() )
        return;
      LOGINFO( "Done processing data back to " << from );
      sleep( 60 );
    }
  }
};

int main( int argc, char **argv )
{
  // KVDIR
  string kvdir = getenv("KVALOBS");
  if ( kvdir.empty() ) {
    LOGFATAL( "Could not find KVALOBS environment variable" );
    return 1;
  }
  if(kvdir[kvdir.length()-1]!='/')
    kvdir+="/";

  //PID-file
  std::string pidfile=kvdir+"var/run/kvAgregated.pid";
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
  dnmi::file::PidFileHelper pidFile;


  // Logging
  FLogStream fine( 9, 1024 * 1024 );
  fine.open(kvdir + "var/agregate/agregator.log");
  fine.loglevel( DEBUG );
  FLogStream error( 9 );
  error.open(kvdir + "var/agregate/agregator.warn.log");
  error.loglevel( WARN );
  LogManager::instance()->addStream( &fine );
  LogManager::instance()->addStream( &error );

  // Proxy database
  DriverManager manager;
  std::string proxyID;
  const string dbDriverPath = kvdir + "lib/db/";
  if ( !manager.loadDriver(dbDriverPath + "sqlitedriver.so", proxyID) ) {
    LOGFATAL( "Error when loading database driver: " << manager.getErr() );
    return 2;
  }
  Connection *dbConn = manager.connect(proxyID, kvdir + "var/agregate/database.sqlite");

  if ( ! dbConn ) {
    LOGFATAL("Cant create a database connection to: " 
	     << endl << kvdir + "var/agregate/database.sqlite");
    return 1;
  }

  // Configuration file
  //string myconf=kvdir + "etc/" + "kvAgregated.conf";
  string myconf = "kvAgregated.conf";
  miutil::conf::ConfSection *confSec = KvApp::readConf(myconf);
  if(!confSec){
    myconf=kvdir + "etc/kvalobs.conf";
    confSec = KvApp::readConf(myconf);
  }
  if(!confSec){
    LOGFATAL("Cant open conf file: " << myconf);
    return 1;
  }

  // PID-file creation
  pidFile.createPidFile(pidfile);


  // Repopulate database?
  bool repopulate = false;
  int backprod_length = 1;
  int ageOfProxyData = 0;
  if ( argc > 1 ) {
    for ( int a = 1; a < argc; a++ ) {
      if ( ! strcmp( argv[a], "-r" ) )
	repopulate = true;
      if ( ! memcmp( argv[a], "-b", 2 ) ) {
	if ( &argv[a][2] )
	  backprod_length = atoi( &argv[a][2] );
	else
	  backprod_length = atoi( argv[ ++a ] );
      }
      if ( ! memcmp( argv[a], "-o", 2 ) ) {
	if ( &argv[a][2] )
	  ageOfProxyData = atoi( &argv[a][2] );
	else
	  ageOfProxyData = atoi( argv[ ++a ] );
      }
    }
  }

  // Core objects
  KvApp app( argc, argv, confSec );
  kvservice::proxy::KvalobsProxy proxy( *dbConn, false );
  AgregatorHandler handler( proxy );
  if ( repopulate )
    proxy.db_repopulate();

  if ( ageOfProxyData ) {
    miTime newOldest = miTime::nowTime();
    newOldest.addHour( - ageOfProxyData );
    proxy.setOldestInProxy( newOldest );
  }

  // Standard times
  set<miClock> six;
  six.insert( miClock( 6,0,0) );
  six.insert( miClock(18,0,0) );

  // Add handlers
  MinMax tan12 = min( TAN, TAN_12, 12, six );
  MinMax tax12 = max( TAX, TAX_12, 12, six );
  MinMax tgn12 = min( TGN, TGN_12, 12, six );
  rr_1  rr1  = rr_1();
  rr_12 rr12 = rr_12();
  rr_24 rr24 = rr_24();
  ra2rr_12_backward ra2rr_b;
  ra2rr_12_forward  ra2rr_f;
  handler.addHandler( &tan12 );
  handler.addHandler( &tax12 );
  handler.addHandler( &tgn12 );
  handler.addHandler( &rr1  );
  handler.addHandler( &rr12 );
  handler.addHandler( &rr24 );
  handler.addHandler( &ra2rr_b );
  handler.addHandler( &ra2rr_f );

  backprod back( proxy, backprod_length );
  boost::thread t( back );
  //back();

  // Start
  proxy.start_thread();

  const set<int> &inter = proxy.getInteresting();
  for ( set<int>::const_iterator it = inter.begin(); it != inter.end(); it++ )
    cerr << *it << " ";
  cerr << endl;

  LOGINFO("Starting main loop");
  app.run();

  t.join();

  return 0;
}
