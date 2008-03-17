/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: KvalobsProxy.h,v 1.1.2.7 2007/09/27 09:02:16 paule Exp $                                                       

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
#ifndef __kvservice__proxy__KvalobsProxy_h__
#define __kvservice__proxy__KvalobsProxy_h__

#include <boost/utility.hpp>
#include <kvcpp/kvservicetypes.h>
#include <set>
#include <puTools/miTime>
#include <dnmithread/CommandQue.h>
#include <kvskel/datasource.hh>
#include <decodeutility/kvDataFormatter.h>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/shared_ptr.hpp>

namespace kvservice
{
  namespace proxy
  {
    class Callback;

    namespace internal
    {
      class IncomingHandler;
      class KvDataSaver;
    }


    class KvalobsProxy : boost::noncopyable
    {
      public:
        KvalobsProxy( dnmi::db::Connection & connection, bool repopulate = false );
        ~KvalobsProxy( );

        void getData( KvDataList &data, int station,
                      const miutil::miTime &from, const miutil::miTime &to,
                      int paramid, int type, int sensor, int lvl );

        CKvalObs::CDataSource::Result_var sendData( const KvDataList &data );


        dnmi::db::Connection & setConnection( dnmi::db::Connection & connection );

        /**
         * Add parameter to store in database
         */
        void addInteresting( int param )
        {
          interesting.insert( param );
        }

        const std::set<int> & getInteresting() const
        {
          return interesting;
        }

        /**
         * Start loop, waiting for data
         */
        void start();

        /**
         * Start loop in a new thread.
         */
        void start_thread();

        /**
         * Stop loop. Will not cancel subscription.
         */
        void stop();

        /**
         * Are we about to stop?
         */
        bool stopping() const
        {
          return shutdown;
        }

        /**
         * Process data for the given period of time.
         */
        void processData( const miutil::miTime &from, const miutil::miTime &to );

        /**
         * Wait for data, for timeout seconds. If timeout == 0: wait forever.
         */
        void awaitData( int timeout = 1 );

        // Operations on proxy:
        void db_clear();
        void db_cleanup();
        void db_populate( int hours = 24 );
        void db_repopulate( int hours = 24 )
        {
          db_clear();
          db_populate( hours );
        }

        const miutil::miTime & getOldestInProxy() const
        {
          return oldestInProxy;
        }

        void setOldestInProxy( const miutil::miTime & newTime )
        {
          oldestInProxy = newTime;
        }


      private:  // Proxy database:
        dnmi::db::Connection &connection;
        miutil::miDate lastCleaned;
        const int daysToKeep;

      private:  // Kvalobs subscription:
        void kvalobs_subscribe();

        std::string subscription;

        std::set<int> interesting;

        dnmi::thread::CommandQue queue;

        void run();

        bool shutdown;

        boost::shared_ptr<internal::IncomingHandler> incomingHandler;
        friend class internal::IncomingHandler;

      private:  // Fetch data
        miutil::miTime oldestInProxy;

        void proxy_getData( KvDataList &data, int station,
                            const miutil::miTime &from,
                            const miutil::miTime &to,
                            int paramid, int type, int sensor, int lvl );

        void kvalobs_getData( KvDataList &data, int station,
                              const miutil::miTime &from,
                              const miutil::miTime &to,
                              int paramid, int type, int sensor, int lvl );

        //void rmIrrelevant(  KvDataList &data, std::set<miutil::miTime> &times,
        //                int paramid, int type, int sensor, int lvl );

      private:  // Callback
        friend class Callback;

        typedef std::set<Callback *> CallbackSet;
        typedef CallbackSet::const_iterator CICallbackSet;


        void callback_add( Callback *callback );
        void callback_rm( Callback *callback );

        std::set<Callback *> callbacks;
        boost::mutex callbacks_mutex;

        void callback_send( KvObsDataList &data );
        void callback_send( KvDataList &data );

      private:  // Thread synchronization

        boost::thread *thread;

        friend class internal::KvDataSaver;

        typedef boost::recursive_mutex::scoped_lock Lock;
        mutable boost::recursive_mutex kv_mutex;

        mutable boost::recursive_mutex proxy_mutex;
    };
  }
}

#endif // __kvservice__proxy__KvalobsProxy_h__
