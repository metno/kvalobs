/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: checkrunner.cc,v 1.1.2.8 2007/09/27 09:02:21 paule Exp $                                                       

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
#include "kvPerlParser.h"
#undef list

#include <errno.h>
#include "checkrunner.h"

#include "kvQABaseTypes.h"
#include "kvQABaseDBConnection.h"
#include "kvQABaseScriptManager.h"
#include "kvQABaseMetadata.h"

#include <list>

#include <milog/milog.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <stdexcept>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/lexical_cast.hpp>

using namespace std;
using namespace kvalobs;
using namespace milog;
using boost::filesystem::path;

namespace
{
  enum LogType { Debug, Info, Warn, Error };

  inline void log_( const std::string & msg, LogType t = Info )
  {
    switch ( t )
    {
      case Debug:
        IDLOGDEBUG( "html", msg << endl );
        break;
      case Info:
        IDLOGINFO( "html", msg << endl );
        break;
      case Warn:
        IDLOGWARN( "html", msg << endl );
        break;
      case Error:
        IDLOGERROR( "html", msg << endl );
        break;
    }
  }

  void logEnd_( const std::string & msg, LogType t = Info )
  {
    log_( msg, t );
    IDLOGINFO( "html", "</PRE></CODE>" );
    Logger::removeLogger( "html" );
    if ( t == Error )
      throw std::runtime_error( msg );
  }
}

CheckRunner::CheckRunner( const kvStationInfo & params,
                          dnmi::db::Connection & con_,
                          const path & logp )
    : stinfo( params )
    , dbcon( & con_ )
    , meteod( dbcon, params )
    , logpath_( logp )
{
  if ( !dbcon.dbOk() )
    throw runtime_error( "No database connection" );
}

CheckRunner::~CheckRunner()
{}

bool CheckRunner::shouldProcess()
{
  // Skip stations with typeid < 0 (aggregated values)
  if ( stinfo.typeID() < 0 ) {
    log_( "Incoming data are agregated" );
    return false;
  }

  // make sure we are supposed to run checks for this station
  // - if station not in obs_pgm: skip it!
  // ...unless this is an unknown ship, stinfo.stationID()>10000000
  if ( stinfo.stationID() <= 10000000 )
  {
    if ( ! dbcon.getObsPgm( stinfo.stationID(), stinfo.obstime(), oprogramlist ) )
      throw std::runtime_error( "Error when accesssing obs_pgm from database" );
    if ( oprogramlist.empty() ) {
      log_( "Station does not exist in obspgm" );
      return false;
    }
  }
  if ( hqcCorrected() ) {
    log_( "Incoming data has been controlled by HQC" );
    return false;
  }
  if ( dataWasCheckedBefore() ) {
    log_( "Checks have been run before on this data" );
    return false;
  }

  return true;
}

// Return if any data has been modified by HQC
bool CheckRunner::hqcCorrected()
{
  const kvQABaseMeteodata::DataFromTime & data_ = meteod.getData();

  for ( kvQABaseMeteodata::DataFromTime::const_iterator it = data_.begin(); it != data_.end(); ++ it )
  {
    typedef kvQABaseDBConnection::obs_data::Container KvDL;
    const KvDL & dl = it->second.data;
    for ( KvDL::const_iterator itb = dl.begin(); itb != dl.end(); ++ itb )
      if ( itb->typeID() == stinfo.typeID() and itb->controlinfo().flag( 15 ) )
        return true;
  }
  return false;
}

bool CheckRunner::dataWasCheckedBefore()
{
  const kvQABaseMeteodata::DataFromTime & data_ = meteod.getData();

  for ( kvQABaseMeteodata::DataFromTime::const_iterator it = data_.begin(); it != data_.end(); ++ it )
  {
    typedef kvQABaseDBConnection::obs_data::Container KvDL;
    const KvDL & dl = it->second.data;
    for ( KvDL::const_iterator itb = dl.begin(); itb != dl.end(); ++ itb ) {
//      cout << "Considering: " << * itb << endl;
      if ( itb->typeID() == stinfo.typeID() and itb->useinfo().flag( 0 ) == 9 )
        return false;
    }
  }
  return true;
}


void CheckRunner::updateStaticVariables()
{
  //Bxrge Moe
  //2004.9.21
  //Have changed how frequent the static tables is updated.
  //They was updated once for every read. I have changed this
  //to one time every hour. This mean that we may have wrong data
  //for at most one hour. We can remedy this by restarting kvalobs.
  //In the future can we implement a reread by a signal, ex SIGHUP.
  if ( updateStaticTime.undef() || updateStaticTime < miutil::miTime::nowTime() )
  {
    updateStaticTime = miutil::miTime::nowTime();
    updateStaticTime.addHour( 1 );

    dbcon.updateStatics();
  }
}

void CheckRunner::findChecks( list<kvChecks> & out )
{
  if ( ! dbcon.getChecks( stinfo.stationID(), stinfo.obstime(), out ) )
    logEnd_( "CheckRunner: Error reading checks", Error );
}

string CheckRunner::getPerlScript( const kvalobs::kvChecks & check, kvQABaseScriptManager & sman ) const
{
  bool sig_ok; // signature ok
  if ( ! sman.findAlgo( check.checkname(), check.checksignature(), stinfo.stationID(), sig_ok ) )
    throw runtime_error( "CheckRunner::runChecks failed in ScriptManager.findAlgo\nAlgorithm not identified!" );
  else if ( ! sig_ok )
    throw runtime_error( "CheckRunner::runChecks failed in ScriptManager.findAlgo\nBAD signature(s)!" );
  
  string ret;
  
  if ( ! sman.getScript( ret ) )
    throw runtime_error( "CheckRunner::runChecks failed in ScriptManager.getScript" );

  return ret;
}

namespace
{
  /**
   * Signals that a particuar check should not be run.
   */
  struct SkipCheck : exception {};
}

string CheckRunner::getMeteoData( const kvalobs::kvChecks & check, kvQABaseScriptManager & sman )
{
  string ret;
  bool skipcheck = false;

//  cout << "REQUEST:\n";
//  cout << '\t' << check.qcx() << "\t-\t" << check.checkname() << endl;
//  cout << '\t' << check.checksignature() << endl;
//  for (kvObsPgmList::const_iterator it = oprogramlist.begin(); it != oprogramlist.end(); ++ it )
//    cout << "\t\t" << it->stationID() << ", " << it->paramID() << ", " << it->typeID() << endl;
  
  if ( ! meteod.data_asPerl( check.qcx(), check.medium_qcx(), check.language(), sman, oprogramlist, skipcheck, ret ) )
    throw runtime_error( "CheckRunner::runChecks failed in meteod.data_asPerl" );
  
  if ( skipcheck )
    throw SkipCheck();
  return ret;
}

string CheckRunner::getMetaData( const kvalobs::kvChecks & check, kvQABaseMetadata & metad, kvQABaseScriptManager & sman ) const
{
  string ret;
  if ( ! metad.data_asPerl( stinfo.stationID(), check.qcx(), stinfo.obstime(), sman, ret ) )
    throw std::runtime_error( "CheckRunner::runChecks failed in metad.data_asPerl" );
  return ret;
}


string CheckRunner::getScript( const kvChecks & check, kvQABaseMetadata & metad, kvQABaseScriptManager & sman )
{
  ostringstream checkstr; // final check string
  checkstr << "#==========================================\n"
  << "# KVALOBS check-script\n"
  << "# type: " << check.qcx() << "\n"
  << "#==========================================\n\n"
  << "use strict;\n";

  string perlScript = getPerlScript( check, sman );
  string meteoData = getMeteoData( check, sman );
  string metaData = getMetaData( check, metad, sman );

  checkstr << metaData << meteoData << perlScript;
  
//  cout << "\n------------------------------------------------------------------\n\n";
//  cout << checkstr.str() << endl; 

  return checkstr.str();
}


void CheckRunner::runCheck( const kvalobs::kvChecks & check, kvQABaseMetadata & metad, kvQABaseScriptManager & sman )
{
  log_( "<HR>" );
  log_( string( "<H2>Check loop, type:" ) + check.qcx() + " name:" + check.checkname() + "</H2>" );

  string final_check;
  try
  {
    final_check = getScript( check, metad, sman );
  }
  catch ( SkipCheck & )
  {
    return log_( "CheckRunner::runCheck skipping check (obs_program)" );
  }
  catch ( runtime_error & e )
  {
    return log_( e.what(), Error );
  }

  // Writing the script to be run to html:
//#define LOG_CHECK_SCRIPT
#ifdef LOG_CHECK_SCRIPT  
    log_( "Final checkstring:" );
    log_( "<font color=#007700>" );
    log_( final_check );
    log_( "</font>" );
#endif

  /* ----- run check ---------------------------------- */
  kvPerlParser parser;                // the perlinterpreter
  map<string, double> retvalues;
  if ( ! parser.runScript( final_check, retvalues ) )
    return log_( "CheckRunner::runCheck failed in parser.runScript", Error );

  // TEST --------------------------------------------------
  log_( "Successfully run check.", Debug );
  IDLOGDEBUG( "html", "Number of return parameters:" << retvalues.size() << endl );
  for ( map<string, double>::iterator dp = retvalues.begin(); dp != retvalues.end(); dp++ )
    IDLOGDEBUG( "html", "Param:" << dp->first << " equals " << dp->second << endl );
  // ----------------------------------------------------------

  if ( not retvalues.empty() )
  {
    // Update parameters with new control-flags and any return-variables - also update kvStationInfo
    IDLOGINFO( "html", "Updating observation(s) with return values from check" << endl );
    if ( ! meteod.updateParameters( retvalues ) )
      log_( "CheckRunner::runCheck failed in updateParameters", Error );
  }
  else
    log_( "No return values from check - skip update..", Warn );
}

void CheckRunner::operator() ( bool forceCheck )
{
  openHTMLStream();
  try
  {
    if ( not forceCheck and not shouldProcess() ) {
      logEnd_( "Will not process data" );
      return;
    }
    updateStaticVariables();
  }
  catch ( std::exception & e )
  {
    // We wish to log errors on html:
    logEnd_( e.what(), Error );
    throw;
  }
  
  kvQABaseMetadata metad( dbcon );      // Metadata manager
  kvQABaseScriptManager sman( dbcon );  // Perlscript manager

  //  relevant checks (QC1) for this observation
  list<kvChecks> checks;
  findChecks( checks );
  if ( checks.empty() )
    return logEnd_( "No appropriate checks found" );

  meteod.resetFlags(stinfo);
    
  // Loop through checks
  for ( list<kvChecks>::const_iterator cp = checks.begin(); cp != checks.end(); ++ cp )
    runCheck( * cp, metad, sman );

  logEnd_( "Done processing", Debug );
  LOGINFO( "CheckRunner::runChecks FINISHED" << endl );
}


namespace
{
  path logPath( const kvalobs::kvStationInfo & stinfo, const path & start_logpath )
  {
    path log_dir(start_logpath);
    const path stationDirectory = boost::lexical_cast<string>(stinfo.stationID()); 

    log_dir /= stationDirectory/stinfo.obstime().isoDate();

    boost::filesystem::create_directories( log_dir );
    return log_dir;
  }

  path logfilename( const std::string & clock, int version )
  {
      ostringstream filename;
      filename << "log-" << clock;
      if ( version ) filename << '_' << version;
      filename << ".html";
      return filename.str();
  }

  path getLogfilePath( const kvalobs::kvStationInfo & stinfo, const path & start_logpath )
  {
    namespace fs = boost::filesystem;

    fs::path log_dir = logPath( stinfo, start_logpath );
  
    std::string clock = stinfo.obstime().isoClock();
    std::replace( clock.begin(), clock.end(), ':', '-' );

    fs::path log_file;
    for ( int i = 8; i >= 0; -- i ) {
      log_file = log_dir/logfilename( clock, i );
      if ( fs::exists( log_file ) ) {
	fs::path rename_to = log_dir/logfilename( clock, i +1 );
	fs::remove( rename_to );
	fs::rename( log_file, rename_to );
      }
    }
    return log_file.native_directory_string();
  }
}

HtmlStream * CheckRunner::openHTMLStream()
{
  HtmlStream * html = 0;
  try
  {
    html = new HtmlStream;
    
    path logfile = getLogfilePath( stinfo, logpath_ );
    
    LOGINFO( "CheckRunner::runChecks for station:" << stinfo.stationID()
        << " and obstime:" << stinfo.obstime() << endl
        << "Logging all activity to:" << logfile.native_file_string() << endl );
  
    if ( !html->open( logfile.native_file_string() ) )
      throw std::runtime_error( "Failed to create logfile for the html output. Filename:\n" + logfile.native_file_string() );
  
    Logger::createLogger( "html", html );
    Logger::logger("html").logLevel( DEBUG );
  
    IDLOGINFO( "html", "<h1>"
        << "CheckRunner::runChecks for station:" << stinfo.stationID()
            << " and obstime:" << stinfo.obstime()
            << "</h1>" << endl );
    IDLOGINFO( "html", "<CODE><PRE>" );
  }
  catch ( std::exception & e )
  {
    LOGERROR( "Error when creating logfile: " << e.what() << "\nUsing /dev/null for logging!");
    html->open( "/dev/null" );
  }
  return html;
}
