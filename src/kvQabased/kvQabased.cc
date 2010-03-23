/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvQabased.cc,v 1.1.2.3 2007/09/27 09:02:21 paule Exp $                                                       

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
#include "qabaseApp.h"
#include "qabaseInputImpl.h"
#include "QaWorkThread.h"
#include <milog/milog.h>
#include "InitLogger.h"
#include <miconfparser/miconfparser.h>
#include <fileutil/pidfileutil.h>
#include "AdminImpl.h"
#include <kvalobs/kvPath.h>

//For test
const char* options[][ 2 ] =
  {
    {"InitRef",
     "NameService=corbaname::monsoon.oslo.dnmi.no"
    },
    {0, 0}
  };

using namespace std;
using namespace boost;


int
main( int argc, char** argv )
{
  CORBA::ORB_ptr orb;
  PortableServer::POA_ptr poa;
  string dbdriver;
  miutil::conf::ConfSection *conf = KvApp::getConfiguration();
  //Read all connection information from $KVALOBS/etc/kvalobs.conf
  //if it exist. Otherwise use the environment variables.
  //ie: KVDB, KVDBUSER, PGHOST, PGPORT
  string constr( KvApp::createConnectString() );
  bool error;


  if ( conf )
  {
    miutil::conf::ValElementList val = conf->getValue( "database.dbdriver" );

    if ( val.size() == 1 )
      dbdriver = val[ 0 ].valAsString();
  }

  //Use postgresql as a last guess.
  if ( dbdriver.empty() )
    dbdriver = "pgdriver.so";

  string htmlpath;

  try {
    InitLogger( argc, argv, "kvQabased", htmlpath );
  }
  catch ( filesystem::filesystem_error & e) {
    LOGFATAL( e.what() );
    return 1;
  }
  
  
  LOGINFO( "KvQabased: starting ...." );
  filesystem::path rundir( kvPath("rundir")  );
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
  filesystem::path pidfile( dnmi::file::createPidFileName( rundir.native_file_string(), "kvQabased" ) );

  if ( dnmi::file::isRunningPidFile( pidfile.native_file_string(), error ) )
  {
    if ( error )
    {
      LOGFATAL( "An error occured while reading the pidfile:" << endl
                << pidfile.native_file_string() << " remove the file if it exist and"
                << endl << "kvQabased is not running. " <<
                "If it is running and there is problems. Kill kvQabased and"
                << endl << "restart it." << endl << endl );
      return 1;
    }
    else
    {
      LOGFATAL( "Is kvQabased allready running?" << endl
                << "If not remove the pidfile: " << pidfile.native_file_string() );
      return 1;
    }
  }




  QaBaseApp app( argc, argv, dbdriver, constr, options );

  if ( !app.isOk() )
  {
    LOGFATAL( "FATAL: can't  initialize " << argv[ 0 ] << "!\n" );
    return 1;
  }

  orb = app.getOrb();
  poa = app.getPoa();

  QaWork qaWork( app, htmlpath );
  thread qaWorkThread( qaWork );

  try
  {

    QaBaseInputImpl* qabaseImpl = new QaBaseInputImpl( app );
    AdminImpl *admImpl = new AdminImpl( app );

    PortableServer::ObjectId_var mgrImplIid = poa->activate_object( qabaseImpl );
    PortableServer::ObjectId_var admImplIid = poa->activate_object( admImpl );

    {
      // IDL interface: CKvalObs::CQabase::QabaseInput
      CORBA::Object_var ref = qabaseImpl->_this();

      if ( !app.putRefInNS( ref, "kvQabaseInput" ) )
      {
        LOGFATAL( "FATAL: can't register with CORBA nameserver!\n" );
        return 1;
      }

      CORBA::String_var sior( orb->object_to_string( ref ) );
      cout << "IDL object kvQabaseInput IOR = '" << ( char* ) sior << "'" << endl;

      // IDL interface: micutil::Admin
      ref = admImpl->_this();

      if ( !app.putRefInNS( ref, "kvQabaseAdmin" ) )
      {
        LOGFATAL( "FATAL: can't register with CORBA nameserver!\n" );
        return 1;
      }

      sior = orb->object_to_string( ref );
      cout << "IDL object micutil::Admin IOR = '" << ( char* ) sior << "'"
      << endl;

    }

    // Obtain a POAManager, and tell the POA to start accepting
    // requests on its objects.
    PortableServer::POAManager_var pman = app.getPoaMgr();
    pman->activate();

    app.createPidFile( "kvQabased" );

    orb->run();
    orb->destroy();
  }
  catch ( CORBA::SystemException& )
  {
    LOGFATAL( "Caught CORBA::SystemException." );
    app.deletePidFile();
    exit( 1 );
  }
  catch ( CORBA::Exception& )
  {
    LOGFATAL( "Caught CORBA::Exception." );
    app.deletePidFile();
    exit( 1 );
  }
  catch ( omniORB::fatalException & fe )
  {
    LOGFATAL( "Caught omniORB::fatalException:" << endl
              << "  file: " << fe.file() << endl
              << "  line: " << fe.line() << endl
              << "  mesg: " << fe.errmsg() );
    app.deletePidFile();
    exit( 1 );
  }
  catch ( ... )
  {
    LOGFATAL( "Caught unknown exception." );
    app.deletePidFile();
    exit( 1 );
  }

  CERR( "kvQabased: exit ....\n" );
  app.deletePidFile();
  return 0;
}

