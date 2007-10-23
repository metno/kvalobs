/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: AgregatorHandler.cc,v 1.1.2.5 2007/09/27 09:02:15 paule Exp $                                                       

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
#include "AgregatorHandler.h"
#include "KvDataFunctors.h"
#include "paramID.h"
#include <milog/milog.h>

using namespace kvservice;
using namespace kvservice::proxy;
using namespace milog;

namespace agregator
{
  AgregatorHandler *AgregatorHandler::agHandler = 0;

  AgregatorHandler::AgregatorHandler( KvalobsProxy & proxy )
      : Callback( proxy )
  {
    if ( ! agHandler )
      agHandler = this;
  }

  AgregatorHandler::~AgregatorHandler( )
  {
    if ( agHandler == this )
      agHandler = 0;
  }

  void AgregatorHandler::addHandler( AbstractAgregator * handler )
  {
    LOGDEBUG( "Adding handler: " << handler->readParam() << " -> " << handler->writeParam() );
    getProxy().addInteresting( handler->readParam() );
    getProxy().addInteresting( handler->writeParam() );
    handlers.insert( Handler( handler->readParam(), handler ) );
  }

#ifdef AGREGATOR_DEBUG
  struct largeStationNo
  {
    bool operator() ( const kvalobs::kvData & data ) const
    {
      return data.stationID() > 99;
    }
  };
#endif // AGREGATOR_DEBUG


  void AgregatorHandler::newData( KvDataList &data )
  {
#ifdef AGREGATOR_DEBUG
    data.remove_if( largeStationNo() );
#endif // AGREGATOR_DEBUG

    for ( IKvDataList d = data.begin(); d != data.end(); d++ )
    {
      if ( d->paramID() == RA )
      {
        LOGDEBUG( "Found RA parameter in station " << d->stationID()
                  << ", type " << d->typeID()
                  << ". Ignoring all RR_1 observations from this station." );
        StationHasParamid shp( RR_1, &*d );
        data.remove_if( shp );
        break;
      }
    }

    for ( CIKvDataList dl = data.begin(); dl != data.end(); ++ dl )
      process( * dl );
  }
  
  void AgregatorHandler::process( const kvalobs::kvData & data )
  {
      const int paramID = data.paramID();
      HandlerMap::const_iterator it = handlers.lower_bound( paramID );
      const HandlerMap::const_iterator end = handlers.upper_bound( paramID );
      while ( it != end )
      {
        try
        {
          LOGINFO( "Processing:\n" <<
                   decodeutility::kvdataformatter::createString( data ) );
                   
          //it->second->process( data );
          AbstractAgregator * agregator = it->second;
          if ( agregator->isInterestedIn( data ) )
          {
          	std::auto_ptr<kvalobs::kvData> d = agregator->process( data, getRelevantObsList( data, agregator->getTimeSpan( data ) ) );
          	if ( d.get() )
          		save( * d );
          }
          	
          ++it;
          LOGDEBUG( "Processing done" );
        }
        catch ( exception & e )
        {
          LOGFATAL( typeid( e ).name() << ":\n\t" << e.what() );
          throw;
        }
        catch ( ... )
        {
          LOGFATAL( "Unknown exception" );
          throw;
        }
      }
  }
  
  void AgregatorHandler::save( const kvalobs::kvData & d )
  {
    list<kvalobs::kvData> dl;
    dl.push_back( d );
    CKvalObs::CDataSource::Result_var res = getProxy().sendData( dl );

    if ( res->res != CKvalObs::CDataSource::OK )
    {
    	std::ostringstream ss;
    	ss << "Error when submitting data: " << res->message << '\n'  
    	   << decodeutility::kvdataformatter::createString( dl );
      const std::string msg = ss.str();
      throw std::runtime_error( msg );
    }
  }
  
  
  list<kvalobs::kvData>
  AgregatorHandler::getRelevantObsList( const kvalobs::kvData & data,
                                         const AbstractAgregator::TimeSpan & obsTimes )
  {
    list<kvalobs::kvData> ret;

    getProxy().getData( ret, data.stationID(), obsTimes.first, obsTimes.second,
                   data.paramID(), data.typeID(), data.sensor(), data.level() );
    
    return ret;
  }
  
}
