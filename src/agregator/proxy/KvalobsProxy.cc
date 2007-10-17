/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: KvalobsProxy.cc,v 1.2.2.5 2007/09/27 09:02:16 paule Exp $                                                       

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
#include "KvalobsProxy.h"
#include "Callback.h"
#include "KvDataSaver.h"
#include "KvDataReceiver.h"
#include "IncomingHandler.h"
#include <kvalobs/kvDataOperations.h>
#include <kvservice/kvcpp2/KvApp.h>
#include <kvservice/kvcpp2/kvevents.h>
#include <milog/milog.h>
#include <sstream>
#include <utility>
#include <cmath>
#include <algorithm>
#include <boost/thread/thread.hpp>

//#define NDEBUG
#include <cassert>

using namespace std;
using namespace kvalobs;
using namespace miutil;
using namespace milog;
using namespace dnmi::db;
using namespace dnmi::thread;

namespace kvservice
{
  namespace proxy
  {

    KvalobsProxy::KvalobsProxy( Connection & connection_, bool repopulate )
        : connection( connection_ )
        , lastCleaned( miDate::today() )
        , daysToKeep( 35 )
        , shutdown( false )
        , oldestInProxy( miTime::nowTime() )
        , thread( 0 )
    {
      if ( ! KvApp::kvApp )
      {
        const char * msg = "Cannot find an instance of KvApp!";
        LOGFATAL( msg );
        throw runtime_error( msg );
      }

      incomingHandler =
          boost::shared_ptr<internal::IncomingHandler>( new internal::IncomingHandler( * this ) );


      // Make reasonably sure the proxy is correct.
      oldestInProxy.addHour( 2 );

      LogContext context( "KvalobsProxy::KvalobsProxy" );

      kvalobs_subscribe();
      if ( repopulate )
        db_repopulate();
    }

    KvalobsProxy::~KvalobsProxy( )
    {
      LogContext context( "~KvalobsProxy" );
      stop();
    }

    void KvalobsProxy::db_clear()
    {
      Lock lock ( proxy_mutex )
        ;
      LOGDEBUG( "Deleting all entries in proxy database" );
      connection.exec( "delete from data" );
      oldestInProxy = miTime::nowTime();
      oldestInProxy.addHour();
      LOGINFO( "Proxy database cleared" );
    }

    void KvalobsProxy::db_populate( int hours )
    {
      WhichDataHelper wdh( CKvalObs::CService::All );
      miTime to = miTime::nowTime();
      miTime from = to;
      from.addHour( -hours );
      wdh.addStation( 0, from, to );

      internal::KvDataSaver ds( *this );

      LOGDEBUG( "Fetching data from source database." );

      bool result = KvApp::kvApp->getKvData( ds, wdh );
      if ( ! result )
      {
        const char * err_msg = "Unable to retrieve data from main server.";
        LOGFATAL( err_msg );
        throw runtime_error( err_msg );
      }
      Lock lock ( kv_mutex )
        ;
      assert ( ! oldestInProxy.undef() );
      oldestInProxy = min( oldestInProxy, from );
      LOGINFO( "Got data from source database. Current oldest observation:" << oldestInProxy << "." );
    }

    void KvalobsProxy::db_cleanup()
    {
      LOGINFO( "KvalobsProxy::db_cleanup" );

      miTime t = miTime::nowTime();
      t.addDay( - daysToKeep );
      ostringstream query;
      query << "delete from data where obstime < \'" << t << "\';";

      Lock lock ( proxy_mutex )
        ;
      lastCleaned = miDate::today();
      connection.exec( query.str() );
      oldestInProxy = max( oldestInProxy, t );

      LOGINFO( "KvalobsProxy::db_cleanup: Done" );
    }

    void KvalobsProxy::kvalobs_subscribe()
    {
      assert( KvApp::kvApp );
      LOGINFO( "Subscribing to data from source" );
      KvDataSubscribeInfoHelper sih;
      //sih.addStationId( 18815 );
      subscription = KvApp::kvApp->subscribeData( sih, queue );
    }

    Connection &KvalobsProxy::setConnection( Connection &connection )
    {
      Lock lock ( proxy_mutex )
        ;
      Connection &ret = this->connection;
      this->connection = connection;
      return ret;
    }

    void KvalobsProxy::start()
    {
      LogContext context( "KvalobsProxy::start" );
      shutdown = false;
      try
      {
        run();
      }
      catch ( std::exception & e )
      {
        LOGFATAL( e.what() );
        stop();
      }
      catch ( ... )
      {
        LOGFATAL( "Unknown exception!" );
        stop();
      }
      LOGDEBUG( "Stopping..." );
    }

    namespace
    {
      struct startit
      {
        KvalobsProxy *p;
        startit( KvalobsProxy *p ) : p( p )
        {}
        void operator() ()
        {
          p->start();
        }
      };
    }

    void KvalobsProxy::start_thread()
    {
      LogContext context( "KvalobsProxy::start_thread" );

      LOGDEBUG( "Start thread" );
      startit s( this );
      thread = new boost::thread( s );
    }

    void KvalobsProxy::stop()
    {
      shutdown = true;
      if ( incomingHandler.get() )
        incomingHandler->stopThreads();
      if ( thread )
      {
        LOGDEBUG( "Stopping KvalobsProxy thread" );
        thread->join();
        delete thread;
        thread = 0;
        LOGDEBUG( "KvalobsProxy thread stopped" );
      }
    }

    void KvalobsProxy::processData( const miTime &from, const miTime &to )
    {
      LogContext context( "KvalobsProxy::processData" );
      LOGINFO( "Starting processing of data from " << from << " to " << to );

      WhichDataHelper wdh( CKvalObs::CService::All );
      miTime new_to( to );
      new_to.addSec( -1 );
      wdh.addStation( 0, from, new_to );

      KvDataList dataList;
      internal::KvDataReceiver dr( dataList );
      bool result = KvApp::kvApp->getKvData( dr, wdh );
      if ( ! result )
      {
        const char * err_msg = "Unable to retrieve data from kvalobs.";
        LOGERROR( err_msg );
        return ;
      }

      LOGDEBUG( "Got data. Processing..." );
      callback_send( dataList );

      LOGDEBUG( "Done" );
    }


    void KvalobsProxy::awaitData( int timeout )
    {
      auto_ptr<CommandBase> base( queue.get( timeout ) );
      if ( ! base.get() )
        return ;
      DataEvent *data = dynamic_cast<DataEvent*>( base.get() );

      if ( ! data )
      {
        LOGERROR( "Could not understand data received from kvalobs" );
        return ;
      }

      assert( ( void* ) base.get() == ( void* ) data );

      data->dispatchEvent( * incomingHandler );
    }

    void KvalobsProxy::run()
    {
      LogContext context( "KvalobsProxy::run (main loop)" );
      LOGDEBUG( "Running" );
      while ( not shutdown )
      {
        awaitData( 1 );
        miTime now = miTime::nowTime();
        if ( now.date() > lastCleaned and
             now.clock() > miClock( 2, 15, 0 ) )
          db_cleanup();
      }
    }

    bool different_( const kvData & a, const kvData & b )
    {
      bool va = valid( a );
      bool vb = valid( b );
      if ( va != vb )
        return true;
      if ( va and vb )
      {
        float diff = abs( a.corrected() - b.corrected() );
        return diff > 0.04999;
      }
      return false;
    }

    CKvalObs::CDataSource::Result_var
    KvalobsProxy::sendData( const KvDataList &data )
    {
      LogContext context ( "KvalobsProxy::sendData" );
      CKvalObs::CDataSource::Result_var res( new CKvalObs::CDataSource::Result );
      KvDataList l;
      Lock lock( kv_mutex ); // We must lock everything

      for ( CIKvDataList it = data.begin(); it != data.end(); it++ )
      {
        KvDataList dl;
        getData( dl, it->stationID(), it->obstime(), it->obstime(),
                 it->paramID(), it->typeID(), it->sensor(), it->level() );
        if ( dl.empty() )
        {
          l.push_back( *it );
          LOGDEBUG( "No matching entries in kvalobs - sending unmodified." );
          continue;
        }
        //assert( dl.size() == 1 );
        kvData & d = dl.front();
        LOGDEBUG( "Matching entry:\n" << decodeutility::kvdataformatter::createString( d ) );

        if ( not different_( d, * it ) )
        {
          LOGDEBUG( "New data matches old - will not send correction" );
          continue;
        }
        if ( valid( * it ) )
          d.corrected( it->corrected() );
        else
          reject( d );

        l.push_back( d );
      }
      if ( l.empty() )
      {
        LOGDEBUG( "No new data to send (kvalobs had all data from before)" );
        res->res = CKvalObs::CDataSource::OK;
        res->message = "No data";
        return res;
      }
      miString msg = decodeutility::kvdataformatter::createString( l );
      const char *message = msg.cStr();
      LOGINFO( "Sending data to kvalobs:\n" << message );
      res = KvApp::kvApp->sendDataToKv( message, "kv2kvDecoder" );

      // LOG ERRORS!

      if ( res->res == CKvalObs::CDataSource::OK )
      {
        // LOGDEBUG( "Save in proxy" );
        KvObsDataList odl;
        KvObsData obsdata;
        KvDataList &dl = obsdata.dataList();
        dl.insert( dl.end(), l.begin(), l.end() );
        internal::KvDataSaver ds( *this );
        odl.push_back( obsdata );
        // LOGDEBUG( "Saving " << odl.size() << " elements." );
        ds.next( odl );
      }
      return res;
    }

    void KvalobsProxy::getData( KvDataList &data, int station,
                                const miutil::miTime &from, const miutil::miTime &to,
                                int paramid, int type, int sensor, int lvl )
    {
      //LogContext context( "KvalobsProxy::getData" );

      assert( not oldestInProxy.undef() );

      // Fetch data from kvalobs, if neccessary
      if ( from <= oldestInProxy )
      {
        miTime k_from = from;
        miTime k_to = min( to, oldestInProxy );
        //LOGDEBUG( "Fetching times " << from << " - " << k_to << " from kvalobs" );
        kvalobs_getData( data, station, k_from, k_to, paramid, type, sensor, lvl );
      }

      // Fetch data from proxy, if neccessary
      if ( to > oldestInProxy )
      {
        miTime p_from = max( from, oldestInProxy );
        //LOGDEBUG( "Fetching times " << p_from << " - " << to << " from proxy" );
        proxy_getData( data, station, p_from, to, paramid, type, sensor, lvl );
      }
    }

    void KvalobsProxy::proxy_getData( KvDataList &data, int station,
                                      const miutil::miTime &from, const miutil::miTime &to,
                                      int paramid, int type, int sensor, int lvl )
    {
      LogContext context( "proxy_getData" );
      //LOGDEBUG( "KvalobsProxy::proxy_getData" );

      // This should avoid problems caused by database entries '0' and 0:
      int alt_sensor;
      if ( sensor >= '0' )
        alt_sensor = sensor - '0';
      else
        alt_sensor = sensor + '0';

      ostringstream s;
      s << "select * from data where stationid=" << station
      << " and paramid=" << paramid
      << " and typeid=" << type
      //<< " and sensor=" << ((sensor < 10) ? (sensor + '0') : sensor)
      << " and (sensor=" << sensor << " or sensor=" << alt_sensor << ")"
      << " and level=" << lvl;
      if ( from < to )
        s << " and (obstime>\'" << from << "\' and obstime<=\'" << to << "\')";
      else if ( from == to )
        s << " and obstime=\'" << from << "\'";
      else // This is really an error, but...
        s << " and (obstime>\'" << to << "\' and obstime<=\'" << from << "\')";

      //LOGDEBUG( s.str() );

      try
      {
        auto_ptr<Result> res;
        Lock lock ( proxy_mutex )
          ;
        //Lock lock( kv_mutex );
        res.reset( connection.execQuery( s.str() ) );
        while ( res->hasNext() )
          data.push_back( kvData( res->next() ) );
      }
      catch ( exception & e )
      {
        LOGERROR( e.what() );
      }
      catch ( ... )
      {
        LOGERROR( "Unknown error during database lookup" );
      }
    }

    namespace
    {
      struct invalid
      {
        const int paramid;
        const int type;
        const int sensor;
        const int lvl;
        invalid( int paramid, int type, int sensor, int lvl )
            : paramid( paramid ), type( type ), sensor( sensor ), lvl( lvl )
        {}
        bool operator() ( const kvData &data )
        {
          return not ( paramid == data.paramID() and
                       type == data.typeID() and
                       //sensor  == data.sensor() and
                       ( sensor == data.sensor() or abs( sensor - data.sensor() ) == '0' ) and
                       lvl == data.level() );
        }
      };
    }

    void KvalobsProxy::kvalobs_getData( KvDataList &data, int station,
                                        const miutil::miTime &from, const miutil::miTime &to,
                                        int paramid, int type, int sensor, int lvl )
    {
      ostringstream contextstream;
      contextstream << "kvalobs_getData( " << station << ", " << from << ", " << to << ", " << paramid << "< " << type << " )";
      LogContext context( contextstream.str() );

      WhichDataHelper wdh( CKvalObs::CService::All );
      miTime newFrom = from;
      if ( from != to )
        newFrom.addSec(); // We don't want inclusive from

      wdh.addStation( station, newFrom, to );

      internal::KvDataReceiver dr( data );
      // getting data from kvalobs includes saving it in proxy:
      LOGDEBUG( "Fetching data from kvalobs" );
      bool result = KvApp::kvApp->getKvData( dr, wdh );

      if ( ! result )
        LOGERROR( "Unable to retrieve data from kvalobs." );
      
      data.remove_if( invalid( paramid, type, sensor, lvl ) );

      //LOGDEBUG( "Data from kvalobs :\n" << decodeutility::kvdataformatter::createString( data ) );
    }

    void KvalobsProxy::callback_add( Callback *callback )
    {
      //LOGDEBUG( "callback_add" );
      callbacks.insert( callback );
    }

    void KvalobsProxy::callback_rm( Callback *callback )
    {
      //LOGDEBUG( "callback_rm" );
      callbacks.erase( callback );
    }

    void KvalobsProxy::callback_send( KvObsDataList &data )
    {
      for ( CICallbackSet it = callbacks.begin(); it != callbacks.end(); it++ )
        ( *it ) ->newData( data );
    }

    void KvalobsProxy::callback_send( KvDataList &data )
    {
      for ( CICallbackSet it = callbacks.begin(); it != callbacks.end(); it++ )
        ( *it ) ->newData( data );
    }
  }
}
