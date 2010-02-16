/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: IncomingHandler.cc,v 1.2.2.5 2007/09/27 09:02:16 paule Exp $                                                       

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
#include "IncomingHandler.h"
#include "KvalobsProxy.h"
#include "KvDataSaver.h"
#include "Callback.h"
#include <milog/milog.h>

#include <boost/thread/thread.hpp>
#include <boost/thread/condition.hpp>

#include <sstream>

//#define NDEBUG
#include <cassert>


using namespace std;
using namespace milog;

namespace kvservice
{
  namespace proxy
  {
    namespace internal
    {
      IncomingHandler::HandlerThread::HandlerThread( IncomingHandler & handler )
          : handler( handler )
      {}

      void IncomingHandler::HandlerThread::operator() ()
      {
        LogContext context( "Incoming" );

        while ( 1 )
        {
          KvObsDataListPtr data;
          {
            boost::mutex::scoped_lock lock ( handler.mutex );

//            LOGDEBUG( "Top of loop: " << handler.queue.size()
//                      << " elements in queue." << endl );

            if ( not handler.isStopping() and handler.queue.empty() )
            {
              //LOGDEBUG( "Thread sleeping" );
              handler.condition.wait( lock );
              //LOGDEBUG( "Thread woke up" );
            }

            if ( handler.isStopping() )
              break;

            if ( handler.queue.empty() )
            {
              LOGWARN( "Wakeup on empty queue." );
              continue;
            }

            try
            {
              data = handler.queue.front();
              handler.queue.pop_front();
            }
            catch ( std::exception & e )
            {
              LOGERROR( "Error when getting data: " << e.what() );
              continue;
            }
            catch ( ... )
            {
              LOGERROR( "Error when getting data: Unknown reason" );
              continue;
            }
          }

          try
          {
            handler.process( data );
          }
          catch ( ... )
          {
            boost::mutex::scoped_lock lock ( handler.mutex );
            LOGERROR( "Error when processing data. "
                      "Putting data back in queue." );
            handler.queue.push_front( data );
            handler.condition.notify_one();
          }
        }
        //LOGDEBUG( "Thread terminating" );
      }

      IncomingHandler::IncomingHandler( KvalobsProxy &proxy_,
                                        bool doStartThreads,
                                        int noOfThreads_ )
          : proxy( proxy_ ), noOfThreads( noOfThreads_ ), threadsStopping( true )
      {
        if ( doStartThreads )
          startThreads();
      }

      IncomingHandler::~IncomingHandler( )
      {
        //assert( proxy.stopping() );
        {
          boost::mutex::scoped_lock lock( mutex );
          queue.clear();
        }
        stopThreads();
      }

      void IncomingHandler::onKvDataEvent( KvObsDataListPtr data )
      {
        boost::mutex::scoped_lock lock( mutex );
        queue.push_back( data );
        condition.notify_one();
        //process( data );
      }

      void IncomingHandler::startThreads()
      {
        if ( ! threadsStopping )
          throw ThreadRestart();
        threadsStopping = false;
        for ( int i = 0; i < noOfThreads; ++i )
        {
          HandlerThread ht( * this );
          boost::thread * thread = new boost::thread( ht );
          threads.push_back( thread );
        }
      }

      void IncomingHandler::stopThreads()
      {
        threadsStopping = true;

        condition.notify_all();

        for ( list< boost::thread *>::iterator it = threads.begin();
              it != threads.end();
              ++it )
        {
          boost::thread * t = * it;
          t->join();
          delete t;
        }
        threads.clear();
      }

      bool IncomingHandler::isStopping() const
      {
        return threadsStopping;
      }

      void IncomingHandler::process( KvObsDataListPtr & data )
      {
        if ( not data->empty() )
        {
          ostringstream ss;
          ss << "Data in:" << endl;
          for ( IKvObsDataList i1 = data->begin(); i1 != data->end(); ++i1 )
            for ( CIKvDataList i2 = i1->dataList().begin();
                  i2 != i1->dataList().end(); ++i2 )
              ss << decodeutility::kvdataformatter::createString( * i2 ) << endl;
          LOGDEBUG( ss.str() );

          KvDataSaver ds( proxy );
          ds.next( *data );

          proxy.getCallbackCollection().send( *data );

          LOGINFO( "Station " << data->front().dataList().front().stationID() << " done" );
        }
      }
    }
  }
}
