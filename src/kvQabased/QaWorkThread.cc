/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: QaWorkThread.cc,v 1.1.2.3 2007/09/27 09:02:21 paule Exp $                                                       

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
#include "QaWorkThread.h"
#include "qabaseApp.h"
#include "QaWorkCommand.h"
#include "checkrunner.h"
#include <sstream>
#include <milog/milog.h>
#include <kvalobs/kvDbGate.h>
#include <puTools/miTime>
#include <boost/regex.hpp>
#include <boost/filesystem/exception.hpp>
#include <memory>
#include <stdexcept>

using namespace kvalobs;
using namespace std;
using namespace miutil;

QaWork::QaWork( QaBaseApp &app_, const std::string& logpath )
    : app( app_ ), logpath_( logpath )
{}

namespace
{
  /**
   * Handles db connection and dsconnection after timeout
   */
  class ConnectionHandler
  {
      QaBaseApp & app_;
      dnmi::db::Connection * con;
      int idleTime;
      static const int max_idle_time = 60;
    public:

      ConnectionHandler( QaBaseApp & app )
          : app_( app ), con( 0 ), idleTime( 0 )
      {}

      ~ConnectionHandler()
      {
        LOGDEBUG( "Closing the database connection before termination!" );
        if ( con )
          app_.releaseDbConnection( con );
      }

      /**
       * \returns a connection to db, either freshly generated or an old one
       */
      dnmi::db::Connection * getConnection()
      {
        if ( ! con )
        {
          while ( ! app_.shutdown() )
          {
            con = app_.getNewDbConnection();
            if ( con )
            {
              LOGDEBUG( "Created a new connection to the database!" );
              break;
            }
            LOGINFO( "Can't create a connection to the database, retry in 5 seconds .." );
            sleep( 5 );
          }
        }
        idleTime = 0;
        return con;
      }

      /**
       * Signal that the db connection is not needed. If this signal is used
       * \c max_idle_time times in a row, the db connection will be released.
       */
      void notNeeded()
      {
        if ( con and ++ idleTime >= max_idle_time )
        {
          LOGDEBUG( "Closing the database connection!" );
          app_.releaseDbConnection( con );
          con = 0;
          idleTime = 0;
        }
      }
  };

  void doUpdateWorkQue( kvDbGate & gate, const kvStationInfo & si, const string & colName )
  {
    LOGDEBUG( "UPDATE: workque!" );
    ostringstream ost;
    ost << "UPDATE workque SET " << colName << "='" << miTime::nowTime().isoTime()
    << "' WHERE stationid=" << si.stationID()
    << "  AND obstime='" << si.obstime().isoTime()
    << "' AND typeid=" << si.typeID();

    if ( !gate.exec( ost.str() ) )
      LOGERROR( "QaWorkThread: (" << colName << ") Cant update table workque." <<
                "-- Stationid: " << si.stationID() << endl <<
                "--   obstime: " << si.obstime() << endl <<
                "--    typeid: " << si.typeID() <<
                "-- query: " << ost.str() << endl <<
                "-- reason: " << gate.getErrorStr() );
  }

  string getLogPath( const QaWorkCommand & work )
  {
    string work_logpath;
    if ( work.getKey( "logpath", work_logpath ) )
      return work_logpath;
    return "";
  }

  bool updateWorkQue_( const QaWorkCommand & work )
  {
    string work_update_workque;
    if ( work.getKey( "update_workque", work_update_workque ) )
      if ( work_update_workque == "false" )
        return false;
    return true;
  }
}

void
QaWork::operator() ()
{
  LOGINFO( "QaWork: starting work thread!\n" );

  ConnectionHandler connectionHandler( app );

  while ( !app.shutdown() )
  {
    auto_ptr<const dnmi::thread::CommandBase> cmd( app.getInQue().get( 1 ) );
    if ( ! cmd.get() )
    {
      connectionHandler.notNeeded();
      continue;
    }
    if ( app.shutdown() )
      continue;

    LOGINFO( "QaWork: command received....\n" );

    const QaWorkCommand * work = dynamic_cast<const QaWorkCommand*>( cmd.get() );

    // The list will have one and only one element when it is received from kvManager.
    if ( work and not work->getStationInfo().empty() )
    {
      dnmi::db::Connection * con = connectionHandler.getConnection();
      if ( ! con )
      {
        LOGERROR( "Could not get connection to database" );
        continue;
      }
      if ( not app.shutdown() )
        process( * con, * work );
    }
    else
      LOGERROR( "QaWork: Unexpected command ....\n" );
  }
  LOGINFO( "QaWork: Thread terminating!" );
}

void QaWork::process( dnmi::db::Connection & con, const QaWorkCommand & work )
{
  kvDbGate gate( & con );
  const kvStationInfo & si = work.getStationInfo().front();

  if ( updateWorkQue_( work ) )
    doUpdateWorkQue( gate, si, "qa_start" );
  else
    LOGDEBUG( "NO UPDATE: workque!" );

  kvalobs::kvStationInfoList retList;
  doWork( si, retList, con, getLogPath( work ) );

  if ( !retList.empty() )
  {
    if ( updateWorkQue_( work ) )
      doUpdateWorkQue( gate, si, "qa_stop" );

    //Return the result to kvManager.
    CKvalObs::CManager::CheckedInput_var callback = work.getCallback();
    app.sendToManager( retList, callback );
  }
}


/**
 * The retList must contain the result that is to be returned to
 * the kvManager. The result may contain more parameters and there
 * may be results for additional stations. But the station that came
 * in and is to be prossecced must be at the head of the retlist. Other
 * stations that is touched in the prossecing must be pushed at the tail.
 */

void
QaWork::doWork( const kvalobs::kvStationInfo & params,
                kvalobs::kvStationInfoList & retList,
                dnmi::db::Connection & con,
                const std::string & logPath )
{
  retList.push_back( params );

  LOGINFO( "QaWork::doWork at:" << miutil::miTime::nowTime()
           << "  Processing " << params.stationID() << " for time "
           << params.obstime() << std::endl );

  try
  {
    // boost::filesystem::path cannot handle paths/with//multiple//slashes//separating/single/elements
    boost::regex re("(\\/\\/)");
    std::string normalizedLog = boost::regex_replace(logPath.empty() ? logpath_ : logPath, re, "/");
    
    CheckRunner checkRunner( params, con, normalizedLog );
    checkRunner();
  }
  catch ( boost::filesystem::filesystem_error & )
  {
    static const std::string devnull = "/dev/null";
    if ( logPath != devnull )
    {
      LOGERROR("Error when trying to open log file. Trying " << devnull << " instead.");
      doWork(params, retList, con, devnull);
    }
  }
  catch ( std::exception & e )
  {
    LOGERROR( e.what() );
  }
}
