/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvQABaseMeteodata.cc,v 1.1.2.6 2007/09/27 09:02:21 paule Exp $                                                       

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

#include "kvQABaseMeteodata.h"
#include "kvQABaseTypes.h"
#include <set>
#include <sstream>
#include <limits>
#include <milog/milog.h>
#include <boost/lexical_cast.hpp>
#include <kvalobs/kvDataOperations.h>
#include <algorithm>
#include <cctype>

using namespace std;
using namespace miutil;
using namespace kvalobs;


kvQABaseMeteodata::kvQABaseMeteodata( kvQABaseDBConnection & dbcon,
                                      const kvalobs::kvStationInfo & stationinfo )
    : dbcon_( dbcon )
    , stationinfo_( stationinfo )
    , saveCache_( dbcon )
{}

/*
  load all observation-data for one station, several timesteps
*/
bool
kvQABaseMeteodata::loadObsData( const int sid,
                                const int tstart,
                                const int tstop )
{
  IDLOGDEBUG( "html", "kvQABaseMeteodata::loadObsData " << endl );

  if ( stationinfo_.obstime().undef() )
    return false;

  // find proper miTimes for required timespan
  miTime stime = stationinfo_.obstime();
  stime.addMin( tstop );
  miTime etime = stationinfo_.obstime();
  etime.addMin( tstart );

  IDLOGDEBUG( "html", "kvQABaseMeteodata::loadObsData for times: "
              << stime << " until " << etime
              << endl );
  // check if required data already exist..
  if ( obsdata.count( sid ) > 0 )
  {
    if ( stime >= minObsTime_[ sid ] )
      stime = maxObsTime_[ sid ];
    if ( etime <= maxObsTime_[ sid ] )
      etime = minObsTime_[ sid ];

    if ( stime >= minObsTime_[ sid ] && etime <= maxObsTime_[ sid ] )
    {
      IDLOGDEBUG( "html", "...already loaded..." << endl );
      return true;
    }
    else if ( stime < minObsTime_[ sid ] )
    {
      minObsTime_[ sid ] = stime;
    }
    else if ( etime > maxObsTime_[ sid ] )
    {
      maxObsTime_[ sid ] = etime;
    }
  }
  else
  {
    minObsTime_[ sid ] = stime;
    maxObsTime_[ sid ] = etime;
  }

  // fetch observation-data from db
  if ( !dbcon_.getObservations( sid, stime, etime, obsdata[ sid ] ) )
  {
    IDLOGERROR( "html", "kvQABaseMeteodata::loadObsData ERROR"
                << " could not get observations from DB."
                << endl );
    return false;
  }
  
//  cout << "From database:" << endl;
//  for ( std::map<miutil::miTime, kvQABaseDBConnection::obs_data>::const_iterator mit = obsdata[sid].begin(); mit != obsdata[sid].end(); ++ mit ) {
//    cout << mit->first << endl;
//    const std::vector<kvalobs::kvData> & d = mit->second.data;
//    for ( std::vector<kvalobs::kvData>::const_iterator it = d.begin(); it != d.end(); ++ it )
//      cout << "\t\t" << * it << endl;
//  }
    
  return true;
}


/*
  load all text-data for one station, several timesteps
*/
bool
kvQABaseMeteodata::loadTextData( const int sid,
                                 const int tstart,
                                 const int tstop )
{
  IDLOGDEBUG( "html", "kvQABaseMeteodata::loadTextData "
              << endl );
  if ( stationinfo_.obstime().undef() )
    return false;

  // find proper miTimes for required timespan
  miTime stime = stationinfo_.obstime();
  stime.addMin( tstop );
  miTime etime = stationinfo_.obstime();
  etime.addMin( tstart );

  IDLOGDEBUG( "html", "kvQABaseMeteodata::loadTextData for times: "
              << stime << " until " << etime
              << endl );
  // check if required data already exist..
  if ( textdata.count( sid ) > 0 )
  {
    if ( stime >= minTextTime_[ sid ] && etime <= maxTextTime_[ sid ] )
    {
      IDLOGDEBUG( "html", "...already loaded..."
                  << endl );
      return true;
    }
    else if ( stime < minTextTime_[ sid ] )
    {
      minTextTime_[ sid ] = stime;
    }
    else if ( etime > maxTextTime_[ sid ] )
    {
      maxTextTime_[ sid ] = etime;
    }
  }
  else
  {
    minTextTime_[ sid ] = stime;
    maxTextTime_[ sid ] = etime;
  }

  // fetch text-data from db
  if ( !dbcon_.getTextData( sid, stime, etime, textdata[ sid ] ) )
  {
    IDLOGERROR( "html", "kvQABaseMeteodata::loadTextData ERROR"
                << " could not get observations from DB."
                << endl );
    return false;
  }

  return true;
}


/*
  load all model-data for one station, several timesteps
*/
bool
kvQABaseMeteodata::loadModelData( const int sid,
                                  const int tstart,
                                  const int tstop )
{
  IDLOGDEBUG( "html", "kvQABaseMeteodata::loadModelData "
              << endl );
  if ( stationinfo_.obstime().undef() )
    return false;

  // find proper miTimes for required timespan
  miTime stime = stationinfo_.obstime();
  stime.addMin( tstop );
  miTime etime = stationinfo_.obstime();
  etime.addMin( tstart );

  IDLOGDEBUG( "html", "kvQABaseMeteodata::loadModelData for times: "
              << stime << " until " << etime
              << endl );
  // check if required data already exist..
  if ( modeldata.count( sid ) > 0 )
  {
    if ( stime >= minModelTime_[ sid ] && etime <= maxModelTime_[ sid ] )
    {
      IDLOGDEBUG( "html", "...allready loaded..."
                  << endl );
      return true;
    }
    else if ( stime < minModelTime_[ sid ] )
    {
      minModelTime_[ sid ] = stime;
    }
    else if ( etime > maxModelTime_[ sid ] )
    {
      maxModelTime_[ sid ] = etime;
    }
  }
  else
  {
    minModelTime_[ sid ] = stime;
    maxModelTime_[ sid ] = etime;
  }

  // fetch  model-data from db
  if ( !dbcon_.getModelData( sid, stime, etime, modeldata[ sid ] ) )
  {
    IDLOGERROR( "html", "kvQABaseMeteodata::loadModelData ERROR"
                << " could not get model-data from DB."
                << endl );
    return false;
  }

  return true;
}


// return data for one parameter
std::string
kvQABaseMeteodata::data_asPerl( const kvQABaseScriptManager& sman,                    // active Script-Manager
                                const kvObsPgmList & oplist )                     // obs_pgm
{
  sman.getVariables( kvQABase::obs_data, obs_vars );
  sman.getVariables( kvQABase::refobs_data, refobs_vars );
  sman.getVariables( kvQABase::model_data, model_vars );

  // if no data needed - return
  if ( obs_vars.pars.empty() && refobs_vars.pars.empty() && model_vars.pars.empty() )
  {
    IDLOGINFO( "html", "No observation- or model-data needed" );
    return "# No observation- or model-data needed\n";
  }

  // using observation_program // if normal station:
  if ( ! oplist.empty() )
  {
    // ensure that requested obs.parameters are active in observation_program
    for ( vector<kvQABase::script_par>::const_iterator itrpa = obs_vars.pars.begin(); itrpa != obs_vars.pars.end(); itrpa++ )
      if ( itrpa->normal ) {
        IDLOGINFO( "html", "Checking obs_pgm for par:" << itrpa->name << " nr:" << itrpa->paramid << endl );
        if ( find_if(oplist.begin(), oplist.end(), kvalobs::inspect::has_paramid(itrpa->paramid)) == oplist.end() )
          throw SkipCheck(); // parameter inactive or not found in obs_program
      }
  }


  // Read and put data into script_var structure
  fillObsVariables( obs_vars );
  fillObsVariables( refobs_vars );
  fillModelVariables( model_vars );
  
  std::ostringstream ret;
  ret << "\n";
  ret << "#==========================================\n";
  ret << "#  Data\n";
  ret << "#\n";

  // add observation-time as variabel  
  const miTime obstime = stationinfo_.obstime();
  ret   << "my @obstime=("  
  	<< obstime.year() << "," << obstime.month() << "," << obstime.day() << "," 
        << obstime.hour() << "," << obstime.min() << "," << obstime.sec() << ");\n\n";

  // add meteorological data strings
  ret << sman.makePerlVariables( obs_vars ) + "\n";
  ret << sman.makePerlVariables( refobs_vars ) + "\n";
  ret << sman.makePerlVariables( model_vars ) + "\n";

  ret << "#\n";
  ret << "#==========================================\n";
  ret << "\n";

  return ret.str();
}

kvQABaseMeteodata::DataFromTime & kvQABaseMeteodata::preloadData(const kvalobs::kvStationInfo & si)
{
  if ( obsdata.find( si.stationID() ) == obsdata.end() )
    if ( ! loadObsData( si.stationID(), 0, 0 ) )
      throw std::runtime_error( "Error on preread of data table" );

  DataFromTime & ret = obsdata[ si.stationID() ];
  return ret;
}

void kvQABaseMeteodata::resetFlags(const kvalobs::kvStationInfo & si)
{
  DataFromTime & data_ = preloadData(si);

  for ( DataFromTime::iterator it = data_.begin(); it != data_.end(); ++ it )
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


void
kvQABaseMeteodata::fillObsVariables( kvQABase::script_var & vars )
{
  typedef std::set<int> TimeOffsets;
  TimeOffsets alltimes; // unique list of timeoffsets

  for ( vector<int>::iterator pp = vars.allpos.begin(); pp != vars.allpos.end(); pp++ )
  {
    IDLOGINFO( "html", " -OBS processing for pos:" << * pp << endl );

    // load data
    if ( ! loadObsData( *pp, vars.timestart, vars.timestop ) )
      throw std::runtime_error( "Error when attempting to load obsdata" );

    // if still no data...give up
    if ( obsdata[ *pp ].empty() )
    {
      IDLOGERROR( "html", "no OBSERVATIONS found for station:" << *pp
                  << " and timesteps:" << vars.timestart
                  << "," << vars.timestop
                  << endl );
      throw std::runtime_error( "Unable to find observations for station/timestep" );
    }

    // fetch text data
    if ( !loadTextData( *pp, vars.timestart, vars.timestop ) )
      throw std::runtime_error("Error when attempting to load text data");

    // loop observation times
    for ( DataFromTime::iterator tp = obsdata[ *pp ].begin(); tp != obsdata[ *pp ].end(); ++ tp )
    {
      int mindiff = miTime::minDiff( tp->first, stationinfo_.obstime() );

      if ( mindiff > vars.timestart )
        continue;
      if ( mindiff < vars.timestop )
        continue;

      IDLOGINFO( "html", "   -OBS processing for time:" << tp->first << endl );
      alltimes.insert( mindiff );

      // loop parameters and search for them in data
      int n = vars.pars.size();
      for ( int j = 0; j < n; j++ )
      {
        int vid = vars.pars[ j ].paramid;
        if ( vid > 1000 )                    // skip text_data!
          continue;
        int sid = vars.pars[ j ].sensor;
        int lid = vars.pars[ j ].level;
        int tid = vars.pars[ j ].typeID;
        string vname = vars.pars[ j ].name;
        string value = kvQABase::missing_value;
        int status = kvQCFlagTypes::status_orig_and_corr_missing;
        kvalobs::kvControlInfo cinfo;

        IDLOGINFO( "html", "     -OBS processing for par:" << vname
                   << " nr:" << vid << endl );

        // find parameter in data-structs
        kvQABaseDBConnection::obs_data::Container & container_ = tp->second.data;
        kvQABaseDBConnection::obs_data::Container::const_iterator find;

        /* check for:
        - correct parameter
        - specific sensor OR first available
        - specific level  OR first available
        - specific typeID OR the original stationinfo.typeID()!!
        ( -typeID == typeID )
        */
        
        // The following two loops should probably be written better some time:
        //
        // The sum of the two loops is: in case a check parameter wants _any_ 
        // typeid, we prefer the parameter with typeid from the incoming data
        // (stationinfo_), before any others. This is done in loop 1. If no 
        // such typeid/parameter exists we try to find any other one (in 
        // loop 2).
                
        // look for exact match:
        for ( find = container_.begin(); find != container_.end(); ++ find )
          if ( find->paramID() == vid &&
               ( sid == -32767 || find->sensor() == sid ) &&
               ( lid == -32767 || find->level() == lid ) &&
               ( ( tid == -32767 && abs( find->typeID() ) == abs( stationinfo_.typeID() ) )
               || abs( find->typeID() ) == abs( tid ) ) )
            break;
        // or exact except typeID:
        if ( find == container_.end() && tid == -32767)
          for ( find = container_.begin(); find != container_.end(); ++ find )
            if ( find->paramID() == vid &&
                 ( sid == -32767 || find->sensor() == sid ) &&
                 ( lid == -32767 || find->level() == lid ) ) {
//              cout << "FOUND: " << find->typeID() << " (looked for " << tid << ")" << endl;
              break;
            }
        if ( find != container_.end() ) // found
        {
          IDLOGDEBUG( "html", "Found OBSERVATION parameter:" << vname
                      << " for station:" << *pp
                      << " and timestep:" << mindiff
                      << endl );
          
          value = boost::lexical_cast<std::string>( find->original() );
          
          // fetch controlinfo
          cinfo = find->controlinfo();
          // fetch status from controlinfo
          status = cinfo.MissingFlag();
          // ensure missing_data flag sanity
          vars.missing_data |= ( status > 0 );
          // set correct sensor, level and typeID in vars
          vars.pars[ j ].sensor = find->sensor();
          vars.pars[ j ].level = find->level();
          vars.pars[ j ].typeID = find->typeID();
        }
        else
        {
          IDLOGINFO( "html", "Missing OBSERVATION parameter:" << vname
                     << " for station:" << *pp
                     << " and timestep:" << mindiff
                     << endl );
          vars.missing_data = true;
        }

        // add data value
        vars.pars[ j ].values[ *pp ][ mindiff ].value = value;
        vars.pars[ j ].values[ *pp ][ mindiff ].status = status;
        vars.pars[ j ].values[ *pp ][ mindiff ].cinfo = cinfo;
      }
    }

    // ============================================================================

    // loop text_data observation times
    for ( TDataFromTime::iterator ttp = textdata[ *pp ].begin(); ttp != textdata[ *pp ].end(); ttp++ )
    {
      int mindiff = miTime::minDiff( ttp->first, stationinfo_.obstime() );

      if ( mindiff > vars.timestart )
        continue;
      if ( mindiff < vars.timestop )
        continue;

      IDLOGINFO( "html", "   -OBS processing for time:"
                 << ttp->first << endl );
      alltimes.insert( mindiff );

      // loop parameters and search for them in data
      int n = vars.pars.size();
      for ( int j = 0; j < n; j++ )
      {
        int vid = vars.pars[ j ].paramid;
        if ( vid <= 1000 )                    // skip normal observation data!
          continue;
        int tid = vars.pars[ j ].typeID;
        string vname = vars.pars[ j ].name;
        string value = kvQABase::missing_value;
        int status = kvQCFlagTypes::status_orig_and_corr_missing;
        kvalobs::kvControlInfo cinfo;

        IDLOGINFO( "html", "     -OBS processing for par:" << vname
                   << " nr:" << vid << endl );

        // find parameter in data-structs
        int i, np = ttp->second.data.size();
        for ( i = 0; i < np; i++ )
        {
          /* check for:
             - correct parameter
             - specific typeID OR the original stationinfo.typeID()!!
               ( -typeID == typeID )
          */
          if ( ttp->second.data[ i ].paramID() == vid &&
               ( ( tid == -32767 && abs( ttp->second.data[ i ].typeID() )
                   == abs( stationinfo_.typeID() ) ) ||
                 abs( ttp->second.data[ i ].typeID() ) == abs( tid ) ) )
          {
            // the observation here is the original value
            value = ttp->second.data[ i ].original();
            // set status= OK
            status = kvQCFlagTypes::status_ok;
            // ensure missing_data flag sanity
            vars.missing_data |= ( status > 0 );
            // set correct typeID in vars
            vars.pars[ j ].typeID = ttp->second.data[ i ].typeID();
            break;
          }
        }
        if ( i == np )
        {
          IDLOGINFO( "html", "Missing OBSERVATION parameter:" << vname
                     << " for station:" << *pp
                     << " and timestep:" << mindiff
                     << endl );
          vars.missing_data = true;
        }

        // add data value
        vars.pars[ j ].values[ *pp ][ mindiff ].value = value;
        vars.pars[ j ].values[ *pp ][ mindiff ].status = status;
        vars.pars[ j ].values[ *pp ][ mindiff ].cinfo = cinfo;
      }
    }
  } // position-loop

  // update complete set of timeoffsets (in reverse order)
  vars.alltimes.clear();

  for ( TimeOffsets::reverse_iterator rt = alltimes.rbegin(); rt != alltimes.rend(); rt++ )
    vars.alltimes.push_back( *rt ); // add timestep to list
}



void
kvQABaseMeteodata::fillModelVariables( kvQABase::script_var& vars )
{
  set<int> alltimes; // unique list of timeoffsets

  vector<int>::iterator pp = vars.allpos.begin();
  for ( ; pp != vars.allpos.end(); pp++ )
  {
    IDLOGINFO( "html", " - MODEL processing for pos:" << *pp << endl );

    // load data
    if ( !loadModelData( *pp, vars.timestart, vars.timestop ) )
      throw std::runtime_error("Unable to load model data");

    // if still no data...give up
    if ( modeldata[ *pp ].size() == 0 )
    {
      IDLOGERROR( "html", "no MODELDATA found for station:" << *pp
                  << " and timesteps:" << vars.timestart << "," << vars.timestop
                  << endl );
      throw std::runtime_error("Unable to find model data for station/timestep" );
    }

    // loop times
    map<miTime, kvQABaseDBConnection::model_data>::iterator tp;
    for ( tp = modeldata[ *pp ].begin(); tp != modeldata[ *pp ].end(); tp++ )
    {
      IDLOGINFO( "html", "   -MODEL processing for time:" << tp->first << endl );
      int mindiff = miTime::minDiff( tp->first, stationinfo_.obstime() );

      if ( mindiff > vars.timestart )
        continue;
      if ( mindiff < vars.timestop )
        continue;

      alltimes.insert( mindiff );

      // loop parameters
      int n = vars.pars.size();
      for ( int j = 0; j < n; j++ )
      {
        int vid = vars.pars[ j ].paramid;
        string vname = vars.pars[ j ].name;
        string value = kvQABase::missing_value;
        int status = kvQCFlagTypes::status_ok;
        IDLOGINFO( "html", "     -MODEL processing for par:" << vname
                   << " nr:" << vid << endl );

        // find parameter in data-structs
        int i, np = tp->second.data.size();
        for ( i = 0; i < np; i++ )
        {
          if ( tp->second.data[ i ].paramID() == vid )
          {
            ostringstream ost;
            ost << tp->second.data[ i ].original();
            value = ost.str();
            status = kvQCFlagTypes::status_ok;
            break;
          }
        }
        if ( i == np )
        {
          IDLOGERROR( "html", "Missing MODEL parameter:" << vname
                      << " for station:" << *pp
                      << " and timestep:" << mindiff
                      << endl );
          status = kvQCFlagTypes::status_original_missing;
          vars.missing_data = true;
          // should we really........
          throw std::runtime_error("Missing model parameter " + vname);
        }

        // add data value
        vars.pars[ j ].values[ *pp ][ mindiff ].value = value;
        vars.pars[ j ].values[ *pp ][ mindiff ].status = status;
      }
    }
  }

  // update complete set of timeoffsets (in reverse order)
  vars.alltimes.clear();
  set<int>::reverse_iterator rt = alltimes.rbegin();
  for ( ; rt != alltimes.rend(); rt++ )
    vars.alltimes.push_back( *rt ); // add timestep to list
}






void kvQABaseMeteodata::setFlag( ScriptReturnType::const_iterator param, const ScriptReturnType & scriptRet, kvalobs::kvData & newdata, const kvalobs::kvChecks & check )
{
  const string & key = param->first;
  int flagvalue = static_cast<int>( param->second );

  kvalobs::kvControlInfo cinfo = newdata.controlinfo();
  kvalobs::kvUseInfo uinfo = newdata.useinfo();

  IDLOGINFO( "html", "=====================================================" << endl <<
             "kvQABaseMeteodata::updateParameters controlinfo BEFORE:"
             << cinfo << endl );

  // ensure that new flagvalue is larger than old
  int oldflagvalue;
  cinfo.getControlFlag( check.medium_qcx(), oldflagvalue );
  if ( oldflagvalue < flagvalue )
    cinfo.setControlFlag( check.medium_qcx(), flagvalue );

  IDLOGINFO( "html", "kvQABaseMeteodata::updateParameters controlinfo  AFTER:" << cinfo << endl <<
             "=====================================================" << endl );
  newdata.controlinfo( cinfo );

  // update cfailed if control-flag != 1
  if ( flagvalue != 1 )
  {
    miString cfailed = newdata.cfailed();
    if ( cfailed.length() > 0 )
      cfailed += ",";

    // Added by Vegard B�nes:
    // Check for subcheck variable for param, station, time.
    // if we are to set cfailed, this will be added to the string.
    string subcheck = "";
    string::size_type pos = key.rfind( '_' );
    string firstPart = key.substr( 0, pos + 1 );
    string subcheckKey = firstPart + type_subcheck;
    IDLOGINFO( "html", "Setting cfailed - checking for subcheck, key(" << subcheckKey << ")" << endl );
    map<string, double>::const_iterator find = scriptRet.find( subcheckKey );
    if ( find != scriptRet.end() )
    {
      ostringstream ost;
      ost << "." << ( int ) find->second;
      IDLOGINFO( "html", "Found key!" << endl );
      subcheck = ost.str();
    }

    cfailed += check.qcx() + subcheck + ":" + miString( check.language() );
    IDLOGINFO( "html", "cfailed = " << cfailed << endl );
    newdata.cfailed( cfailed );
  }

  updateUseInfo( newdata );
}

void kvQABaseMeteodata::setMissing( ScriptReturnType::const_iterator param, kvalobs::kvData & newdata )
{

  kvalobs::kvControlInfo cinfo = newdata.controlinfo();

  IDLOGINFO( "html", "=====================================================" << endl <<
             "kvQABaseMeteodata::updateParameters controlinfo BEFORE:" << cinfo << endl );

  int newVal = static_cast<int>( param->second );
  if ( newVal >= 2 )
    kvalobs::reject( newdata );
  else
  {
    cinfo.setControlFlag( kvQCFlagTypes::f_fmis, newVal );
    newdata.controlinfo( cinfo );
  }

  IDLOGINFO( "html", "kvQABaseMeteodata::updateParameters controlinfo  AFTER:" << newdata.controlinfo() << endl <<
             "=====================================================" << endl );

  updateUseInfo( newdata );
}

void kvQABaseMeteodata::updateUseInfo( kvalobs::kvData & d )
{
  // This is not neccessary for the database, as KvalobsCache will set the 
  // correct useinfo just before saving to the database
  // We still keep this function in order to perform logging of impact on 
  // useflags after having run each check.
	
  kvalobs::kvUseInfo uinfo = d.useinfo();
  
  IDLOGINFO( "html", "=====================================================" << endl <<
             "kvQABaseMeteodata::updateParameters     useinfo BEFORE:" << uinfo << endl );

  uinfo.setUseFlags( d.controlinfo() );

  IDLOGINFO( "html", "kvQABaseMeteodata::updateParameters     useinfo  AFTER:" << uinfo << endl <<
             "=====================================================" << endl );
  d.useinfo( uinfo );
}


void kvQABaseMeteodata::setCorrected( float newVal, kvalobs::kvData & data )
{
  data.corrected( newVal ); // Flags?
}


namespace
{
  size_t getIdx_( const string & s )
  {
    try
    {
      return boost::lexical_cast<size_t>( s );
    }
    catch ( boost::bad_lexical_cast & )
    {
      return numeric_limits<size_t>::max();
    }
  }
}

kvalobs::kvData & kvQABaseMeteodata::getModifiedData( ObsKeys & updated_obs,
    ScriptReturnType::const_iterator pp, const std::string & type )
{
  vector<miString> vs = miString( pp->first ).split( "_" );

  if ( vs.size() < 4 )
    throw invalid_argument( "Bad parametername from script: " + pp->first +
                            " Expected <name>_<timeindex>_<stationindex>_<datatype>" );

  //     string type = vs.back().downcase();                               // datatype

  size_t sidx = getIdx_( vs[ vs.size() - 2 ] );
  size_t tidx = getIdx_( vs[ vs.size() - 3 ] );

  string par;  // parameter name
  int n = vs.size() - 1;
  for ( int i = 0; i < n - 2; i++ )                     // rebuild parameter name...
    par += ( ( i ? "_" : "" ) + vs[ i ] );

  string tmp_key = par + vs[ n - 2 ] + vs[ n - 1 ];

  IDLOGDEBUG( "html", " par:" << par << " tidx:" << tidx << " sidx:" << sidx << " type:" << type << endl );

  // next, identify correct obs-parameter
  if ( type != type_flag && type != type_corrected && type != type_missing )
    throw invalid_argument( "Unknown type of data:" + type );
  if ( tidx >= obs_vars.alltimes.size() )
    throw out_of_range( "Timeindex from script out of bounds:" + boost::lexical_cast<string>( tidx ) );
  if ( sidx >= obs_vars.allpos.size() )
    throw out_of_range( "Stationindex from script out of bounds:" + boost::lexical_cast<string>( tidx ) );


  int ipar, npar = obs_vars.pars.size();
  for ( ipar = 0; ipar < npar; ipar++ )
    if ( obs_vars.pars[ ipar ].signature == par )
      break;
  if ( ipar == npar )
    throw invalid_argument( "Parameter not found:" + par );

  // found station, time and parameter
  int timeoff = obs_vars.alltimes[ tidx ];
  int station = obs_vars.allpos[ sidx ];
  int paramid = obs_vars.pars[ ipar ].paramid;
  int sensor = obs_vars.pars[ ipar ].sensor;
  int level = obs_vars.pars[ ipar ].level;
  int typeID = obs_vars.pars[ ipar ].typeID;

  // calculate parameter-time
  miTime time = stationinfo_.obstime();
  time.addMin( timeoff );
  IDLOGDEBUG( "html", "LOOK FOR station:" << station << " timeoff:" << timeoff
              << " paramid:" << paramid
              << " sensor:" << ( sensor != -32767 ? miString( sensor ) : "ANY" )
              << " level:" << ( level != -32767 ? miString( level ) : "ANY" )
              << " typeID:" << ( typeID != -32767 ? miString( typeID ) : "ANY" )
              << " time:" << time << endl );

  // check if we really have loaded obsdata for these..
  if ( obsdata.count( station ) == 0 )
    throw runtime_error( "Obsdata not found for station:" + boost::lexical_cast<string>( station ) );
  if ( obsdata[ station ].count( time ) == 0 )
    throw runtime_error( "Obsdata not found for time:" + time.isoTime() );

  // find correct parameter and typeID
  kvQABaseDBConnection::obs_data::Container & container_ = obsdata[ station ][ time ].data;
  int np = container_.size();
  for ( ipar = 0; ipar < np; ++ ipar )
  {
    const kvalobs::kvData & d = container_[ ipar ];
    if ( paramid == d.paramID() &&
         sensor == d.sensor() &&
         level == d.level() &&
         typeID == d.typeID() )
      break;
  }
  if ( ipar == np )
  {
    ostringstream ss;
    ss << "Obsdata not found for paramid: " << paramid;
    ss << " sensor:" << ( sensor != -32767 ? miString( sensor ) : "ANY" );
    ss << " level:" << ( level != -32767 ? miString( level ) : "ANY" );
    ss << " typeID:" << ( typeID != -32767 ? miString( typeID ) : "ANY" );

    throw runtime_error( ss.str() );
  }

  // store keys for saving
  if ( updated_obs.find( tmp_key ) == updated_obs.end() )
  {
    obs_keys & obsk = updated_obs[ tmp_key ];
    obsk.stationID = station;
    obsk.time = time;
    obsk.typeID = typeID;
    obsk.paridx = ipar;
  }

  return container_[ ipar ];
}

void kvQABaseMeteodata::updateSingleParam( ObsKeys & updated_obs,
    const ScriptReturnType::const_iterator pp,
    const ScriptReturnType & params, 
    const kvalobs::kvChecks & check)
{
  const string & key = pp->first;
  double value = pp->second;

  // skip subcheck keys here..
  if ( key == type_subcheck )
    return ;

  IDLOGINFO( "html", "Checking key:" << key << " with value:" << value << endl );

  string::size_type pos = key.rfind( '_' );
  if ( pos == string::npos )
  {
    IDLOGERROR( "html", "Invalid format on return value from script: " << key << endl );
    return ;
  }
  string type( key, pos + 1, string::npos );
  transform( type.begin(), type.end(), type.begin(), (int(*)(int))tolower ); // convert type to lowercase

  try
  {
    kvalobs::kvData & newdata = getModifiedData( updated_obs, pp, type ); //obsdata[ station ][ time ].data[ ipar ];

    //     cout << "stored:\t" << newdata << endl;

    // Set results from flag:
    if ( type == type_flag )
      setFlag( pp, params, newdata, check );
    else if ( type == type_missing )
      setMissing( pp, newdata );
    else if ( type == type_corrected )
      setCorrected( value, newdata );

    //     cout << type << "\t" << newdata << endl;
  }
  catch ( exception & e )
  {
    IDLOGERROR( "html", "kvQABaseMeteodata::updateParameters. " << e.what() << endl );
  }
}

void kvQABaseMeteodata::saveInDb( const ObsKeys & updated_obs )
{
  for ( ObsKeys::const_iterator itr = updated_obs.begin(); itr != updated_obs.end(); itr++ )
  {
    kvalobs::kvData newdata = obsdata[ itr->second.stationID ][ itr->second.time ].data[ itr->second.paridx ];
    //     cout << "save\t" << newdata << endl;
    saveCache_.insert( newdata );


    // ..save it to db
    /*       if ( !dbcon_.setObservation( newdata ) ) {
             IDLOGERROR( "html", "kvQABaseMeteodata::updateParameter ERROR"
                         << " could not save parameter to DB."
                         << endl );
             continue;
           }*/
  }
  //   cout << endl;
}


bool kvQABaseMeteodata::updateParameters( const ScriptReturnType & params, const kvalobs::kvChecks & check )
{
  ObsKeys updated_obs;

  for ( ScriptReturnType::const_iterator pp = params.begin(); pp != params.end(); ++ pp )
    updateSingleParam( updated_obs, pp, params, check );

  saveInDb( updated_obs );

  return true;
}

const miString kvQABaseMeteodata::type_flag = "flag";
const miString kvQABaseMeteodata::type_corrected = "corrected";
const miString kvQABaseMeteodata::type_missing = "missing";
const miString kvQABaseMeteodata::type_subcheck = "subcheck";
