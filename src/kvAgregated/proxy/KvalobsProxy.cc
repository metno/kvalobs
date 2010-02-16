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
#include "ProxyDatabaseConnection.h"
#include <kvalobs/kvDataOperations.h>
#include <kvcpp/KvApp.h>
#include <kvcpp/kvevents.h>
#include <milog/milog.h>
#include <sstream>
#include <utility>
#include <cmath>
#include <algorithm>
#include <functional>
#include <boost/thread/thread.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/xtime.hpp>
#include <boost/scoped_ptr.hpp>

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
	class KvalobsProxy::Cleaner
	{
	public:
		Cleaner(KvalobsProxy & proxy) :
			proxy_(proxy), d(new SharedData)
		{}

		void operator () ()
		{
			d->stop_ = false;
			miutil::miDate lastCleaned = miutil::miDate::today();

			boost::xtime time;
			boost::xtime_get(& time, boost::TIME_UTC);

			while ( not d->stop_ )
			{
				boost::mutex::scoped_lock l(d->mutex_);

				time.nsec = 0;
				time.sec += 300;
				d->condition.timed_wait(l, time);
				if ( d->stop_ )
					break;

				miutil::miTime now = miutil::miTime::nowTime();
				if (now.date() > lastCleaned and now.clock() > miClock(2, 15, 0))
					proxy_.db_cleanup();
			}
		}

		void stop()
		{
			boost::mutex::scoped_lock l(d->mutex_);
			d->stop_ = true;
			d->condition.notify_all();
		}

	private:
		struct SharedData
		{
			boost::condition condition;
	        boost::mutex mutex_;
			bool stop_;
		};
		boost::shared_ptr<SharedData> d;
		KvalobsProxy & proxy_;
	};


    KvalobsProxy::KvalobsProxy( ProxyDatabaseConnection & connection_, const std::vector<int> & stations, bool repopulate )
        : connection( connection_.get() )
        , stations_(stations)
        , oldestInProxy( miTime::nowTime() )
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

      if ( repopulate )
        db_repopulate();

      // yes, this works:
      cleaner_ = new Cleaner(* this);
      cleanerThread_ = new boost::thread(* cleaner_);
    }

    KvalobsProxy::~KvalobsProxy( )
    {
    	cleaner_->stop();
    	cleanerThread_->join();
    	delete cleaner_;
    	delete cleanerThread_;
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
      if ( oldestInProxy.undef() )
    	  oldestInProxy = from;
      else
    	  oldestInProxy = min( oldestInProxy, from );
      LOGINFO( "Got data from source database. Current oldest observation:" << oldestInProxy << "." );
    }

    void KvalobsProxy::db_cleanup()
    {
      LOGINFO( "KvalobsProxy::db_cleanup" );

      miTime t = miTime::nowTime();
      t.addDay( - 35 ); // Keep data for 35 days
      ostringstream query;
      query << "delete from data where obstime < \'" << t << "\';";

      Lock lock ( proxy_mutex );
      connection.exec( query.str() );
      if ( oldestInProxy.undef() )
    	  oldestInProxy = t;
      else
    	  oldestInProxy = max( oldestInProxy, t );

      LOGINFO( "KvalobsProxy::db_cleanup: Done" );
    }

    namespace
    {
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
    }

    CKvalObs::CDataSource::Result_var KvalobsProxy::sendData(const KvDataList &data)
	{
		LogContext context("KvalobsProxy::sendData");
		CKvalObs::CDataSource::Result_var res(new CKvalObs::CDataSource::Result);
		KvDataList l;
		Lock lock(kv_mutex); // We must lock everything

		for (CIKvDataList it = data.begin(); it != data.end(); it++)
		{
			KvDataList dl;
			getData(dl, it->stationID(), it->obstime(), it->obstime(),
					it->paramID(), it->typeID(), it->sensor(), it->level());
			if (dl.empty())
			{
				l.push_back(*it);
				LOGDEBUG( "No matching entries in kvalobs - sending unmodified." );
				continue;
			}
			//assert( dl.size() == 1 );
			kvData & d = dl.front();
			LOGDEBUG( "Matching entry:\n" << decodeutility::kvdataformatter::createString( d ) );

			if (not different_(d, *it))
			{
				LOGDEBUG( "New data matches old - will not send correction" );
				continue;
			}
			if (valid(*it))
				correct(d, it->corrected());
			else
				reject(d);

			l.push_back(d);
		}
		if (l.empty())
		{
			LOGDEBUG( "No new data to send (kvalobs had all data from before)" );
			res->res = CKvalObs::CDataSource::OK;
			res->message = "No data";
			return res;
		}
		miString msg = decodeutility::kvdataformatter::createString(l);
		const char *message = msg.cStr();
		LOGINFO( "Sending data to kvalobs:\n" << message );
		res = KvApp::kvApp->sendDataToKv(message, "kv2kvDecoder");

		// LOG ERRORS!

		if (res->res == CKvalObs::CDataSource::OK)
		{
			// LOGDEBUG( "Save in proxy" );
			KvObsDataList odl;
			KvObsData obsdata;
			KvDataList &dl = obsdata.dataList();
			dl.insert(dl.end(), l.begin(), l.end());
			internal::KvDataSaver ds(*this);
			odl.push_back(obsdata);
			// LOGDEBUG( "Saving " << odl.size() << " elements." );
			ds.next(odl);
		}
		return res;
	}

    void KvalobsProxy::getData( KvDataList &data, int station,
                                const miutil::miTime &from, const miutil::miTime &to,
                                int paramid, int type, int sensor, int lvl )
    {
      //LogContext context( "KvalobsProxy::getData" );

      // Fetch data from kvalobs, if neccessary
      if ( oldestInProxy.undef() or from <= oldestInProxy )
      {
        miTime k_from = from;
        miTime k_to = oldestInProxy.undef() ? to : min( to, oldestInProxy );
        //LOGDEBUG( "Fetching times " << from << " - " << k_to << " from kvalobs" );
        kvalobs_getData( data, station, k_from, k_to, paramid, type, sensor, lvl );
      }

      // Fetch data from proxy, if neccessary
      if ( oldestInProxy.undef() or to > oldestInProxy )
      {
        miTime p_from = oldestInProxy.undef() ? from : max( from, oldestInProxy );
        //LOGDEBUG( "Fetching times " << p_from << " - " << to << " from proxy" );
        KvDataList proxyData;
        proxy_getData( proxyData, station, p_from, to, paramid, type, sensor, lvl );
        for ( KvDataList::const_iterator it = proxyData.begin(); it != proxyData.end(); ++ it )
        {
        	KvDataList::const_iterator find = find_if(data.begin(), data.end(), 
        			bind1st(kvalobs::compare::same_kvData(),*it)); 
        	if ( find == data.end() )
        		data.push_back(* it);
        }
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
  }
}
