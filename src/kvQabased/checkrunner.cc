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

using namespace std;
using namespace kvalobs;
using namespace milog;

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
                          const string & logp )
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
  // Skip stations with typeid < 0 (aggregated values) (by Vegard Bnes)
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

// Return if any data has been modified by HQC (ugly and temporary hack) (by Vegard Bnes):
bool CheckRunner::hqcCorrected()
{
  const kvQABaseMeteodata::DataFromTime & data_ = getCheckData();

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
  const kvQABaseMeteodata::DataFromTime & data_ = getCheckData();

  for ( kvQABaseMeteodata::DataFromTime::const_iterator it = data_.begin(); it != data_.end(); ++ it )
  {
    typedef kvQABaseDBConnection::obs_data::Container KvDL;
    const KvDL & dl = it->second.data;
    for ( KvDL::const_iterator itb = dl.begin(); itb != dl.end(); ++ itb )
      if ( itb->typeID() == stinfo.typeID() and itb->useinfo().flag( 0 ) == 9 )
        return false;
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

  return checkstr.str();
}


void CheckRunner::resetFlags()
{
  kvQABaseMeteodata::DataFromTime & data_ = getCheckData();

  for ( kvQABaseMeteodata::DataFromTime::iterator it = data_.begin(); it != data_.end(); ++ it )
  {
    typedef kvQABaseDBConnection::obs_data::Container KvDL;
    KvDL & dl = it->second.data;
    for ( KvDL::iterator itb = dl.begin(); itb != dl.end(); ++ itb )
    {
      int missing = itb->controlinfo().flag( flag::fmis );
      int collected = itb->controlinfo().flag( flag::fd );
      kvControlInfo ci;
      ci.set( flag::fmis, missing );
      ci.set( flag::fd, collected );
      itb->controlinfo( ci );
      
      kvUseInfo ui = itb->useinfo();
      for ( int i = 0; i < 5; ++ i )
        ui.set( i, 9 );
      ui.set( 15, 0 );
      itb->useinfo( ui );
    }
  }
}

kvQABaseMeteodata::DataFromTime & CheckRunner::getCheckData()
{
  if ( meteod.obsdata.find( stinfo.stationID() ) == meteod.obsdata.end() )
    if ( ! meteod.loadObsData( stinfo.stationID(), 0, 0 ) )
      throw std::runtime_error( "CheckRunner: Error on preread of data table" );

  kvQABaseMeteodata::DataFromTime & ret = meteod.obsdata[ stinfo.stationID() ];
  return ret;
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
  //  log_( "Final checkstring:" );
  //  log_( "<font color=#007700>" );
  //  log_( final_check );
  //  log_( "</font>" );

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

  // find relevant checks (QC1) for this observation
  list<kvChecks> checks;
  findChecks( checks );
  if ( checks.empty() )
    return logEnd_( "No appropriate checks found" );

  resetFlags();
    
  // Loop through checks
  for ( list<kvChecks>::const_iterator cp = checks.begin(); cp != checks.end(); ++ cp )
    runCheck( * cp, metad, sman );

  logEnd_( "Done processing", Debug );
  LOGINFO( "CheckRunner::runChecks FINISHED" << endl );
}


namespace
{
  boost::filesystem::path logPath( const kvalobs::kvStationInfo & stinfo, const std::string & start_logpath )
  {

    namespace fs = boost::filesystem;
  
    /*
    fs::path log_dir( start_logpath );
    log_dir = log_dir/stinfo.stationID()/stinfo.obstime().isoDate();
    fs::create_directories( log_dir );
    return log_dir;
    */

    ostringstream path;
    path << start_logpath << '/' << stinfo.stationID() << '/' << stinfo.obstime().isoDate() << '/';
    fs::path log_dir( path.str() );
    fs::create_directories( log_dir );

    return log_dir;
  }

  boost::filesystem::path logfilename( const std::string & clock, int version )
  {
      ostringstream filename;
      filename << "log-" << clock;
      if ( version ) filename << '_' << version;
      filename << ".html";
      return filename.str();
  }

  std::string getLogfileName( const kvalobs::kvStationInfo & stinfo, const std::string & start_logpath )
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
    
    std::string logfilename = getLogfileName( stinfo, logpath_ );
    
    LOGINFO( "CheckRunner::runChecks for station:" << stinfo.stationID()
        << " and obstime:" << stinfo.obstime() << endl
        << "Logging all activity to:" << logfilename << endl );
  
    if ( !html->open( logfilename ) )
      throw std::runtime_error( "Failed to create logfile for the html output. Filename:\n" + logfilename );
  
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

//COMMENT:
//22072003 Bxrge Moe
//I have replaced the call of 'system' to run a shell command with a
//a call to mkdir. We have greater control of the creation of the
//directory. The regression system asumes that logdir is set up via
//symblolic links and manipulates the links. In this senario we never
//know where the links points or if the link is in a state of chance. We
//have a race between us and the regression system. It is better that we
//test and create a directory in a controlled way. We never creates a log
//file if the the top most part of the directory dont exist ie. the link
//does not exist. In this senario we may lose a logfile or two, but that
//is not critical. In the case where the link is missing we open /dev/null
//as the logfile.
//
//This solution is not bullet prof if the regresion system copys files
//and a file that is written to by us is at the same time read by the
//regression system. But it works as a quick fix.

/*
HtmlStream * CheckRunner::openHTMLStream()
{
  HtmlStream * html;
  ostringstream ost;
  list<string> pathlist;
  bool error = false;

  ost << stinfo.stationID();

  try
  {
    html = new HtmlStream();
  }
  catch ( ... )
  {
    LOGERROR( "OUT OF MEMMORY, cant allocate HtmlStream!" );
    return 0;
  }

  pathlist.push_back( ost.str() );
  pathlist.push_back( stinfo.obstime().isoDate() );

  ost.str( "" );
  //logpath_ is the directory where we shall put our
  //html files.
  ost << logpath_;

  for ( list<string>::iterator it = pathlist.begin();
        it != pathlist.end() && !error;
        it++ )
  {
    ost << "/" << *it;
    std::string ost_out = ost.str();
    if ( mkdir( ost_out.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH ) < -1 )
    {
      if ( errno == EEXIST )
      {
        continue;
      }
      else
      {
        error = true;
      }
    }
  }

  if ( error )
  {
    LOGINFO( "CheckRunner::runChecks for station:" << stinfo.stationID()
             << " and obstime:" << stinfo.obstime() << endl
             << "Logging all activity to: /dev/null" << endl
             << "A directory in the logpath maybe missing or a dangling link!" );
    html->open( "/dev/null" );
    return html;
  }

  //log version info:
  std::string version = "";
  
  
  ost << "/log-" << stinfo.obstime().isoClock() << version << ".html";

  miutil::miString logfilename = ost.str();
  logfilename.replace( ":", "-" );

  LOGINFO( "CheckRunner::runChecks for station:" << stinfo.stationID()
           << " and obstime:" << stinfo.obstime() << endl
           << "Logging all activity to:" << logfilename << endl );


  if ( !html->open( logfilename ) )
  {
    LOGERROR( "Failed to create logfile for the html output. Filename:\n" <<
              logfilename << "\nUsing /dev/null!" );
    html->open( "/dev/null" );
  }

  Logger::createLogger( "html", html );
//   Logger::logger( "html" ).logLevel( INFO );
  Logger::logger("html").logLevel( DEBUG );

  IDLOGINFO( "html", "<h1>"
             << "CheckRunner::runChecks for station:" << stinfo.stationID()
             << " and obstime:" << stinfo.obstime()
             << "</h1>" << endl );
  IDLOGINFO( "html", "<CODE><PRE>" );


  return html;
}
  */
