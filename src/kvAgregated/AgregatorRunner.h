/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 Copyright (C) 2010 met.no

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

#ifndef AGREGATORRUNNER_H_
#define AGREGATORRUNNER_H_

#include "proxy/IncomingHandler.h"
#include <boost/thread/thread.hpp>
#include <dnmithread/CommandQue.h>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <vector>

namespace kvservice
{
namespace proxy
{
class KvalobsProxy;
}
}

class AgregatorRunner : boost::noncopyable
{
public:
	explicit AgregatorRunner(const std::vector<int> & stations, kvservice::proxy::KvalobsProxy & proxy);
	~AgregatorRunner();

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

    dnmi::thread::CommandQue & getCommandQueue() { return queue; }
    const dnmi::thread::CommandQue & getCommandQueue() const { return queue; }

protected:

    void run();
    void awaitData(int timeout);


    bool shutdown;
    boost::thread * thread;

    dnmi::thread::CommandQue queue;
    kvservice::proxy::internal::IncomingHandler incomingHandler;
};

#endif /* AGREGATORRUNNER_H_ */
