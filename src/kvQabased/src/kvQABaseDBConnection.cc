/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvQABaseDBConnection.cc,v 1.1.2.4 2007/09/27 09:02:21 paule Exp $                                                       

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
#include "kvQABaseDBConnection.h"

#include "kvQABaseTypes.h"
#include "kvCronString.h"
#include <kvalobs/kvKeyVal.h>
#include "qabaseApp.h"
#include <puTools/miString>
#include <fstream>
#include <sstream>
#include <list>
#include <algorithm>

#include <milog/milog.h>

// list of algorithms (from db)
std::map<std::string, kvalobs::kvAlgorithms> kvQABaseDBConnection::algolist;
long int kvQABaseDBConnection::algo_timestamp = 0;

// parameter info (from db)
std::map<std::string, kvalobs::kvParam> kvQABaseDBConnection::parameterlist;
long int kvQABaseDBConnection::parameter_timestamp = 0;

long int kvQABaseDBConnection::qcxinfo_timestamp = 0;

miutil::miTime kvQABaseDBConnection::active_timestamp = miutil::miTime::nowTime();

kvQABaseDBConnection::kvQABaseDBConnection( dnmi::db::Connection *con )
    : connection_ok( false )
    , connection_( con )
{
  if ( !con )
    return ;

  dbGate.set( con );
  connection_ok = true;

  // first instantation: load static tables
  if ( algo_timestamp == 0 ||
       parameter_timestamp == 0 ||
       qcxinfo_timestamp == 0 )
    updateStatics();
}

/*
  reread static database-tables
  currently: algorithms, param and qcxinfo
*/
void kvQABaseDBConnection::updateStatics()
{
  IDLOGINFO( "html", "RUNNING kvQABaseDBConnection::updateStatics" << std::endl );
  bool res;
  res = updateAlgoList();
  if ( !res )
  {
    IDLOGERROR( "html",
                "kvQABaseDBConnection::updateStatics ERROR from updateAlgoList"
                << std::endl );
  }

  res = updateParameterList();
  if ( !res )
  {
    IDLOGERROR( "html",
                "kvQABaseDBConnection::updateStatics ERROR from updateParameterList"
                << std::endl );
  }

  res = updateQcxList();
  if ( !res )
  {
    IDLOGERROR( "html",
                "kvQABaseDBConnection::updateStatics ERROR from updateQcxList"
                << std::endl );
  }
}


bool kvQABaseDBConnection::updateAlgoList()
{
  algolist.clear();
  algo_timestamp = 0;

  bool result;
  std::list<kvalobs::kvAlgorithms> slist;

  try
  {
    result = dbGate.select( slist, "" );
  }
  catch ( dnmi::db::SQLException & ex )
  {
    IDLOGERROR( "html", "Exception: " << ex.what() << std::endl );
  }
  catch ( ... )
  {
    IDLOGERROR( "html", "Unknown exception: con->exec(ctbl) .....\n" );
  }

  if ( !result )
    return false;

  std::list<kvalobs::kvAlgorithms>::const_iterator it = slist.begin();
  for ( ;it != slist.end(); it++ )
  {
    //IDLOGINFO("html","Found Algorithm:" << *it << std::endl);
    algolist[ it->checkname() ] = *it;
  }

  algo_timestamp = 1;
  return true;
}

bool kvQABaseDBConnection::getAlgo( const std::string name,
                                    kvalobs::kvAlgorithms& algo )
{
  if ( algo_timestamp == 0 )
    updateAlgoList();

  if ( algolist.count( name ) == 0 )
    return false;

  algo = algolist[ name ];
  return true;
}

bool kvQABaseDBConnection::updateParameterList()
{
  parameterlist.clear();
  parameter_timestamp = 0;

  bool result;
  std::list<kvalobs::kvParam> plist;

  try
  {
    result = dbGate.select( plist, "" );
  }
  catch ( dnmi::db::SQLException & ex )
  {
    IDLOGERROR( "html", "Exception: " << ex.what() << std::endl );
  }
  catch ( ... )
  {
    IDLOGERROR( "html", "Unknown exception: con->exec(ctbl) .....\n" );
  }

  if ( !result )
    return false;

  std::list<kvalobs::kvParam>::const_iterator it = plist.begin();
  for ( ;it != plist.end(); it++ )
  {
    //IDLOGINFO("html","Found Parameter:" << *it << std::endl);
    parameterlist[ it->name() ] = *it;
  }

  parameter_timestamp = 1;
  return true;

}


bool kvQABaseDBConnection::getParameter( const std::string& name,
    kvalobs::kvParam& p )
{
  if ( parameter_timestamp == 0 )
    updateParameterList();

  if ( parameterlist.count( name ) == 0 )
    return false;

  p = parameterlist[ name ];

  return true;
}

bool kvQABaseDBConnection::getParameter( const int id,
    kvalobs::kvParam& p )
{
  if ( parameter_timestamp == 0 )
    updateParameterList();

  std::map<std::string, kvalobs::kvParam>::iterator pp = parameterlist.begin();
  for ( ; pp != parameterlist.end(); pp++ )
    if ( pp->second.paramID() == id )
    {
      p = pp->second;
      return true;
    }
  return false;
}

bool kvQABaseDBConnection::getParameterId( const std::string& name,
    int& id )
{
  if ( parameter_timestamp == 0 )
    updateParameterList();

  if ( parameterlist.count( name ) == 0 )
    return false;

  id = parameterlist[ name ].paramID();

  return true;
}


bool kvQABaseDBConnection::updateQcxList()
{
  qcxinfo_timestamp = 0;

  bool result;
  std::list<kvalobs::kvQcxInfo> qlist;

  try
  {
    result = dbGate.select( qlist, "" );
  }
  catch ( dnmi::db::SQLException & ex )
  {
    IDLOGERROR( "html", "Exception: " << ex.what() << std::endl );
  }
  catch ( ... )
  {
    IDLOGERROR( "html", "Unknown exception: con->exec(ctbl) .....\n" );
  }

  if ( !result )
    return false;

  kvalobs::kvControlInfo controlinfo;
  controlinfo.setQcxInfo( qlist );

  qcxinfo_timestamp = 1;
  return true;
}

bool kvQABaseDBConnection::getObsPgm( const int sid,
                                      const miutil::miTime& otime,
                                      kvObsPgmList & oplist )

{
  oplist.clear();

  bool result;

  try
  {
//     result = dbGate.select( oplist, kvQueries::selectObsPgm( sid, otime ) );
    std::list<kvalobs::kvObsPgm> tmp_out;
    result = dbGate.select( tmp_out, kvQueries::selectObsPgm( sid ) );
    for ( std::list<kvalobs::kvObsPgm>::const_iterator it = tmp_out.begin(); it != tmp_out.end(); ++ it )
    {
      if ( it->fromtime() <= otime )
      {
        std::pair<kvObsPgmList::iterator, bool> res = oplist.insert( * it );
        if ( ! res.second and res.first->fromtime() < it->fromtime() )
        {
          oplist.erase( res.first -- );
          oplist.insert( res.first, * it );
        }
      }
    }
  }
  catch ( dnmi::db::SQLException & ex )
  {
    IDLOGERROR( "html", "Exception: " << ex.what() << std::endl );
    return false;
  }
  catch ( ... )
  {
    IDLOGERROR( "html", "Unknown exception: con->exec(ctbl) .....\n" );
    return false;
  }

  return result;
}



bool kvQABaseDBConnection::getChecks( const int sid,
                                      const miutil::miTime& otime,
                                      std::list<kvalobs::kvChecks>& checklist )
{
  checklist.clear();

  std::list<kvalobs::kvChecks> clist; // working copy of checklist
  std::list<int> slist;               // list of stations
  slist.push_back( 0 );
  slist.push_back( sid );

  int language = 1; // only perl-scripts
  bool result;

  //   if (!dbGate.valid()){
  //     IDLOGERROR("html","dbGate is not valid" << std::endl);
  //     return false;
  //   }

  try
  {
    result = dbGate.select( clist, kvQueries::selectChecks( slist, language, otime ) );
  }
  catch ( dnmi::db::SQLException & ex )
  {
    IDLOGERROR( "html", "Exception: " << ex.what() << std::endl );
    return false;
  }
  catch ( ... )
  {
    IDLOGERROR( "html", "Unknown exception: con->exec(ctbl) .....\n" );
    return false;
  }

  if ( !result )
    return false;

  // the list is sorted by 'qcx,stationid'
  // remove duplicates of  'qcx' rows - where stationid!=0 has preference
  std::list<kvalobs::kvChecks>::iterator it, it2;
  for ( it = clist.begin(); it != clist.end(); it++ )
  {
    it2 = it;
    it2++;
    if ( it2 != clist.end() &&
         it->qcx() == it2->qcx() &&
         it->stationID() == 0 )
    {
      clist.erase( it, it2 );
      it = clist.begin();
    }
  }

  // push active entries to final list (using 'obstimes' in checks)
  for ( it = clist.begin(); it != clist.end(); it++ )
  {
    kvalobs::CronString CS( it->active() );
    if ( CS.active( otime ) )
      checklist.push_back( *it );
  }

  return true;
}


bool kvQABaseDBConnection::getMetadata( const int sid,
                                        const miutil::miTime& otime,
                                        const std::string& qcx,
                                        std::list<kvMetadataTable>& tables )
{
  bool result;
  std::string data;

  // fetch metadata from table 'station_param'

  std::list<int> slist; // list of stations
  slist.push_back( 0 );
  slist.push_back( sid );

  std::list<kvalobs::kvStationParam> splist;

  try
  {
    result = dbGate.select( splist,
                            kvQueries::selectStationParam( slist, otime, qcx ) );
  }
  catch ( dnmi::db::SQLException & ex )
  {
    IDLOGERROR( "html", "Exception: " << ex.what() << std::endl );
  }
  catch ( ... )
  {
    IDLOGERROR( "html", "Unknown exception: con->exec(ctbl) .....\n" );
  }

  if ( !result )
    return false;

  std::list<kvalobs::kvStationParam>::const_iterator it = splist.begin();

  for ( ;it != splist.end(); it++ )
  {
    IDLOGDEBUG( "html", "Found StationParams:" << *it << std::endl );

    // get info about parameter
    kvalobs::kvParam kvparam;
    if ( !getParameter( it->paramID(), kvparam ) )
    {
      IDLOGWARN( "html", "kvQABaseDBConnection::getMetadata WARNING "
                 << " getParameter failed, paramid:"
                 << it->paramID() << std::endl );
      return false;
    }

    std::ostringstream ost;
    ost << kvparam.name() << "&"
    << it->level() << "&"
    << it->sensor();

    // unpack metadata-string to table-structures
    result &= kvMetadataTable::processString( ost.str(),
              it->metadata(), tables );
    if ( !result )
    {
      IDLOGWARN( "html", "kvQABaseDBConnection::getMetadata WARNING "
                 << " kvMetadataTable::processString with name:"
                 << ost.str() << " and data:" << it->metadata() << std::endl );
    }
  }

  return result;
}


// Get exactly one observation-parameter
bool kvQABaseDBConnection::getObservation( const int sid,                   // station
    const miutil::miTime& otime,     // obs-time
    const int pid,                   // parameter
    kvalobs::kvData& param )
{
  bool result;

  std::list<kvalobs::kvData> dlist;

  try
  {
    result = dbGate.select( dlist, kvQueries::selectData( sid, pid, otime ) );
  }
  catch ( dnmi::db::SQLException & ex )
  {
    IDLOGERROR( "html", "Exception: " << ex.what() << std::endl );
  }
  catch ( ... )
  {
    IDLOGERROR( "html", "Unknown exception: con->exec(ctbl) .....\n" );
  }

  if ( !result || dlist.size() == 0 )
    return false;

  std::list<kvalobs::kvData>::const_iterator it = dlist.begin();
  for ( ;it != dlist.end(); it++ )
  {
    IDLOGDEBUG( "html", "Found ObsData:" << *it << std::endl );
    param = *it;
  }

  return true;
}


// Set timestamp in Key/Value table
bool kvQABaseDBConnection::saveTimestamp( const miutil::miTime& time )
{
  bool result = false;

  // update timestamp
  //   IDLOGINFO("html","minDiff:" << miutil::miTime::minDiff(time,active_timestamp)
  //      << std::endl);

  if ( miutil::miTime::minDiff( time, active_timestamp ) > 15 )
  {

    IDLOGINFO( "html", "Updating active timestamp to:"
               << time
               << std::endl );

    kvalobs::kvKeyVal keyval( "kvQabased", "checkpoint", time.isoTime() );

    try
    {
      result = dbGate.replace( keyval );
    }
    catch ( dnmi::db::SQLException & ex )
    {
      IDLOGERROR( "html", "Exception: " << ex.what() << std::endl );
    }
    catch ( ... )
    {
      IDLOGERROR( "html", "Unknown exception: con->exec(ctbl) .....\n" );
    }

    active_timestamp = time;
  }

  return result;
}



// Save one observation-parameter to DB
bool kvQABaseDBConnection::setObservation( const kvalobs::kvData& param )
{
  bool result = false;

  try
  {
    result = dbGate.update( param );
  }
  catch ( dnmi::db::SQLException & ex )
  {
    IDLOGERROR( "html", "Exception: " << ex.what() << std::endl );
  }
  catch ( ... )
  {
    IDLOGERROR( "html", "Unknown exception: con->exec(ctbl) .....\n" );
  }

  // update timestamp
  result |= saveTimestamp( param.tbtime() );

  return result;
}


/*
  Read all observation-parameters from DB that matches
  station (sid) and observation times from [stime - etime]
*/
bool kvQABaseDBConnection::getObservations( const int sid,                   // station
    const miutil::miTime& stime,     // start-time
    const miutil::miTime& etime,     // end-time
    std::map<miutil::miTime, obs_data> & data )
{
  bool result;

  IDLOGINFO( "html", "Fetching observations from:" << stime
             << " until " << etime << std::endl );

  std::list<kvalobs::kvData> dlist;

  try
  {
    result = dbGate.select( dlist, kvQueries::selectData( sid, stime, etime ) );
  }
  catch ( dnmi::db::SQLException & ex )
  {
    IDLOGERROR( "html", "Exception: " << ex.what() << std::endl );
  }
  catch ( ... )
  {
    IDLOGERROR( "html", "Unknown exception: con->exec(ctbl) .....\n" );
  }

  if ( !result )
    return false;

  // add new data
  for ( std::list<kvalobs::kvData>::const_iterator it = dlist.begin(); it != dlist.end(); ++ it )
  {
    obs_data & d = data[ it->obstime() ];

    if ( std::find( d.data.begin(), d.data.end(), * it ) == d.data.end() )
    {
      IDLOGDEBUG( "html", "Found ObsData:" << * it << std::endl );
      d.time = it->obstime();
      d.data.push_back( * it );
    }
  }

  return true;
}


/*
  Read all text_data-parameters from DB that matches
  station (sid) and observation times from [stime - etime]
*/
bool kvQABaseDBConnection::getTextData( const int sid,                   // station
                                        const miutil::miTime& stime,     // start-time
                                        const miutil::miTime& etime,     // end-time
                                        std::map<miutil::miTime, text_data>& data )
{
  bool result;

  IDLOGINFO( "html", "Fetching text_data observations from:" << stime
             << " until " << etime << std::endl );

  std::list<kvalobs::kvTextData> tdlist;

  try
  {
    result = dbGate.select( tdlist, kvQueries::selectData( sid, stime, etime ) );
  }
  catch ( dnmi::db::SQLException & ex )
  {
    IDLOGERROR( "html", "Exception: " << ex.what() << std::endl );
  }
  catch ( ... )
  {
    IDLOGERROR( "html", "Unknown exception: con->exec(ctbl) .....\n" );
  }

  if ( !result )
    return false;

  std::list<kvalobs::kvTextData>::const_iterator it;
  // clear data for timepoints found
  for ( it = tdlist.begin();it != tdlist.end(); it++ )
  {
    if ( data.count( it->obstime() ) > 0 )
      data[ it->obstime() ].data.clear();
  }
  // add new data
  for ( it = tdlist.begin();it != tdlist.end(); it++ )
  {
    IDLOGDEBUG( "html", "Found TextData:" << *it << std::endl );
    data[ it->obstime() ].time = it->obstime();
    data[ it->obstime() ].data.push_back( *it );
  }

  return true;
}


/*
  Read all modeldata-parameters from DB that matches
  station (sid) and valid times from [stime - etime]
*/
bool kvQABaseDBConnection::getModelData( const int sid,                   // station
    const miutil::miTime& stime,     // start-time
    const miutil::miTime& etime,     // end-time
    std::map<miutil::miTime, model_data>& data )
{
  bool result;

  model_data d;

  IDLOGINFO( "html", "Fetching modeldata from:" << stime
             << " until " << etime << std::endl );

  std::list<kvalobs::kvModelData> mdlist;

  try
  {
    result = dbGate.select( mdlist, kvQueries::selectModelData( sid, stime, etime ) );
  }
  catch ( dnmi::db::SQLException & ex )
  {
    IDLOGERROR( "html", "Exception: " << ex.what() << std::endl );
  }
  catch ( ... )
  {
    IDLOGERROR( "html", "Unknown exception: con->exec(ctbl) .....\n" );
  }

  if ( !result )
    return false;

  std::list<kvalobs::kvModelData>::const_iterator it;
  // clear data for timepoints found
  for ( it = mdlist.begin();it != mdlist.end(); it++ )
  {
    if ( data.count( it->obstime() ) > 0 )
      data[ it->obstime() ].data.clear();
  }
  // add new data
  for ( it = mdlist.begin();it != mdlist.end(); it++ )
  {
    IDLOGDEBUG( "html", "Found ModelData:" << *it << std::endl );
    data[ it->obstime() ].time = it->obstime();
    data[ it->obstime() ].data.push_back( *it );
  }

  return true;
}
