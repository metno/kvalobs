/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: ThrPoolQue.cc,v 1.8.6.2 2007/09/27 09:02:33 paule Exp $                                                       

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
#include "ThrPoolQue.h"

using namespace std;

namespace{

  bool defaultAfterFunc(dnmi::thread::CommandBase *cmd);

  class ThrPoolQueThread
  {
    dnmi::thread::CommandQue &que;
    dnmi::thread::ThreadPoolQue &thrPool;

  public:
    explicit ThrPoolQueThread(dnmi::thread::CommandQue &q, 
			      dnmi::thread::ThreadPoolQue &thrPQ):
      que(q), thrPool(thrPQ)
    {
    }
    
    void operator()()
    {
      dnmi::thread::CommandBase *cmd;
      CERR("Thread created, tid(" << this << ")!\n");
      
      while(!thrPool.getKillHint()){
	try{
	  cmd=que.get(thrPool.getTimeout());

	  //CERR("ThreadPoolQue: tid(" << this << ") command: " 
	  //     << (cmd?"TRUE\n":"FALSE\n"));
	
	  if(cmd){
	    if(thrPool.getBeforeFunc()){
	      thrPool.getBeforeFunc()(cmd);
	    }
	    
	    cmd->execute();

	    CERR("ThreaPool: after execute....\n");

	    if(thrPool.getAfterFunc()){
	      thrPool.getAfterFunc()(cmd);
	    }
	  }

	  if(thrPool.getKillFunc()){
	    if(thrPool.getKillFunc()())
	      thrPool.killSignal();
	  }
	}
	catch(...){
	}
      }

      //Send the suspend signal to the que.
      que.suspend();
    }
  };


};


dnmi::thread::ThreadPoolQue::ThreadPoolQue(unsigned int size_, 
					   CommandQue   &q,
					   int          timeoutInSec)
  :que(q),size(size_), doKill(false), timeout(timeoutInSec), 
   beforeFunc(0), afterFunc(0), killFunc(0)
{
  afterFunc=defaultAfterFunc;
}

void 
dnmi::thread::ThreadPoolQue::run()
{
  for(int i=0; i<size; i++){
    thrGroup.create_thread(ThrPoolQueThread(que, *this));
  }
}
	

bool 
dnmi::thread::ThreadPoolQue::killSignal()
{
  boost::mutex::scoped_lock Lock(mutex);
  
  if(doKill)
    return false;
  
  doKill=true;
  que.signal();
  return true;
}

void 
dnmi::thread::ThreadPoolQue::join()
{
  thrGroup.join_all();
}

namespace{
  bool 
  defaultAfterFunc(dnmi::thread::CommandBase *cmd)
  {
    delete cmd;
  }
};
