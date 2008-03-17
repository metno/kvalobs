/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: IncomingHandler.h,v 1.1.2.4 2007/09/27 09:02:16 paule Exp $                                                       

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
#ifndef __kvservice__proxy__internal__IncomingHandler_h__
#define __kvservice__proxy__internal__IncomingHandler_h__

#include <list>
#include <stdexcept>
#include <kvcpp/kvevents.h>
#include <kvcpp/KvGetDataReceiver.h>
#include <boost/thread/mutex.hpp>

namespace boost
{
  class condition;
}


namespace kvservice
{
  namespace proxy
  {
    class KvalobsProxy;

    namespace internal
    {
      class IncomingHandler
            : public KvEventInterface
      {
        public:
          IncomingHandler( KvalobsProxy & proxy,
                           bool doStartThreads = true,
                           int noOfThreads = 4 );
          virtual ~IncomingHandler( );

          virtual void onKvDataEvent( KvObsDataListPtr data );

          /**
           * \throws ThreadRestart if threads were already running.
           */
          void startThreads();
          void stopThreads();

          bool stop() const;

        private:

          KvalobsProxy &proxy;

          const int noOfThreads;

          bool threadsStopping;

          boost::condition condition;
          boost::mutex mutex;

          std::list< KvObsDataListPtr > queue;

          class HandlerThread
          {
              IncomingHandler & handler;
            public:
              HandlerThread( IncomingHandler & handler );
              void operator() ();
          };
          friend class HandlerThread;

          /**
           * Used in context of the handler threads.
           */
          void process( KvObsDataListPtr & data );

          std::list< boost::thread *> threads;
      };
      
      struct ThreadRestart : public std::runtime_error
      {
        ThreadRestart()
            : std::runtime_error( "Cannot start threads; already running" )
        {}
      };
    }
  }
}

#endif // __kvservice__proxy__internal__IncomingHandler_h__
