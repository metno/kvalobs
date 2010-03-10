/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id$                                                       

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
#include <boost/thread/thread.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/exception.hpp>
#include "Qc2App.h"
#include <milog/milog.h>
#include "InitLogger.h"
#include <ostream>
#include "StopWatch.h"
#include <miconfparser/miconfparser.h>
#include <fileutil/pidfileutil.h>
#include "Qc2Thread.h"
#include <puTools/miTime.h>
#include <kvalobs/kvPath.h>



//#include <qt.h>
//#include "algorithms/Qraph.h"

#include "CheckedDataCommandBase.h"
#include "CheckedDataHelper.h"


//For test
const char* options[][ 2 ] =
  {
    {"InitRef",
     "NameService=corbaname::monsoon.oslo.dnmi.no"
    },
    {0, 0}
  };

///The kvalobs Qc2 main program.

using namespace std;
using namespace boost;


int
main( int argc, char** argv )
{


  stopwatch SW;
  CORBA::ORB_ptr orb;
  PortableServer::POA_ptr poa;

  ostringstream ost;
  
  string dbdriver;
  string stewpot; // EGL
  miutil::conf::ConfSection *conf = KvApp::getConfiguration();
  string constr( KvApp::createConnectString() );
  char *pKv = getenv( "KVALOBS" );
  bool error;
  //string pidfile;

  milog::LogContext logContext("Qc2 ...");

  if ( conf )
  {
    miutil::conf::ValElementList val = conf->getValue( "database.dbdriver" );

    if ( val.size() == 1 )
      dbdriver = val[ 0 ].valAsString();
      std::cout <<  "FROM CONFIGURATION FILE: " << dbdriver << std::endl;

    miutil::conf::ValElementList valerie = conf->getValue( "Qc2.stewart" );

    if ( valerie.size() == 1 )
      stewpot = valerie[ 0 ].valAsString();
     
      std::cout <<  "FROM CONFIGURATION FILE: " << stewpot << std::endl;
  }

  //Use postgresql as a last guess.
  if ( dbdriver.empty() )
    dbdriver = "pgdriver.so";

  string htmlpath;
  std::cout << "HTMLPATH" << htmlpath << std::endl;
  string logpath_(htmlpath);

  std::cout << "HTMLPATH" << htmlpath << std::endl;
  InitLogger( argc, argv, "Qc2", htmlpath );
  std::cout << "HTMLPATH" << htmlpath << std::endl;


  LOGINFO( "Qc2: starting ...." );

  filesystem::path rundir( kvPath("localstatedir") + "/run" );
  if ( ! boost::filesystem::exists(rundir) ) {
    try {
      filesystem::create_directories(rundir);      
    }
    catch ( filesystem::filesystem_error & e) {
      LOGFATAL( e.what() );
      return 1;
    }
  }
  else if ( ! filesystem::is_directory(rundir) ) {
    LOGFATAL( rundir.native_file_string() << "exists but is not a directory" );
    return 1;
  }
  filesystem::path pidfile( dnmi::file::createPidFileName( rundir.native_file_string(), "kvQc2" ) );

  if ( dnmi::file::isRunningPidFile( pidfile.native_file_string(), error ) )
  {
    if ( error )
    {
      LOGFATAL( "An error occured while reading the pidfile:" << endl
                << pidfile.native_file_string() << " remove the file if it exist and"
                << endl << "kvQc2 is not running. " <<
                "If it is running and there is problems. Kill kvQc2 and"
                << endl << "restart it." << endl << endl );
      return 1;
    }
    else
    {
      LOGFATAL( "Is kvQc2 allready running?" << endl
                << "If not remove the pidfile: " << pidfile.native_file_string() );
      return 1;
    }
  }


  //if ( !pKv )
  //{
    //LOGFATAL( "The environment variable KVALOBS must be set!" );
    //return 1;
  //}
//
  //pidfile = string( pKv ) + "/var/run/Qc2.pid";
//
  //std::cout << pidfile << std::endl;
//
  //if ( dnmi::file::isRunningPidFile( pidfile, error ) )
  //{
    //if ( error )
    //{
      //LOGFATAL( "An error occured while reading the pidfile:" << endl
                //<< pidfile << " remove the file if it exist and"
                //<< endl << "Qc2 is not running. " <<
                //"If it is running and there is problems. Kill Qc2 and"
                //<< endl << "restart it." << endl << endl );
      //return 1;
    //}
    //else
    //{
      //LOGFATAL( "Is Qc2 allready running?" << endl
                //<< "If not remove the pidfile: " << pidfile );
      //return 1;
    //}
  //}
//

  Qc2App app( argc, argv, dbdriver, constr, options );
  
  if ( !app.isOk() )
  {
    LOGFATAL( "FATAL: can't  initialize " << argv[ 0 ] << "!\n" );
    return 1;
  }

  orb = app.getOrb();
  poa = app.getPoa();
 
  app.createPidFile( "kvQc2" );
  sleep(1);

  Qc2Work Qc2Work( app, htmlpath );    //commented out while I test program options !!!!
  boost::thread Qc2Thread( Qc2Work );

  
  try {
// This is where all the *InputImpl(app) and AdminImpl( App ) can be reinstalled if ti is needed
      app.createPidFile( "Qc2" );
      orb->run();
      orb->destroy();
  }
  catch ( CORBA::SystemException& ) {
    LOGFATAL( "Caught CORBA::SystemException." );
    app.deletePidFile();
    exit( 1 );
  }
  catch ( CORBA::Exception& ) {
    LOGFATAL( "Caught CORBA::Exception." );
    app.deletePidFile();
    exit( 1 );
  }
  catch ( omniORB::fatalException & fe ) {
    LOGFATAL( "Caught omniORB::fatalException:" << endl
              << "  file: " << fe.file() << endl
              << "  line: " << fe.line() << endl
              << "  mesg: " << fe.errmsg() );
    app.deletePidFile();
    exit( 1 );
  }
  catch ( ... ) {
    LOGFATAL( "Caught unknown exception." );
    app.deletePidFile();
    exit( 1 );
  }


  CERR( "Qc2: exit ....\n" );
  app.deletePidFile();

  std::cout << "Oh yesteryear" << std::endl;
  std::cerr << "Flow my tears" << std::endl;

  return 0;
}

