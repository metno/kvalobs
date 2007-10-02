/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvCorbaThread.h,v 1.4.6.3 2007/09/27 09:02:46 paule Exp $                                                       

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
#ifndef __kvCorbaThread_h__
#define __kvCorbaThread_h__

#include <omnithread.h>
#include "kvQtCorbaApp.h"
#include "kvDataNotifySubscriberImpl.h"

namespace kvservice{
  namespace priv{
    class KvQtCorbaThread : public omni_thread
      {
	KvQtCorbaThread(const KvQtCorbaThread &);
	KvQtCorbaThread();
	KvQtCorbaThread &operator=(const KvQtCorbaThread&);
	~KvQtCorbaThread(){} 
	
	KvQtCorbaApp  app;
	bool          isInitialized_;
	
      public:
	KvQtCorbaThread(int argn, char **argv,  const char *options[0][2]=0);
	
	void *run_undetached(void*);
	
	bool  isInitialized()const{ return  isInitialized_;}
	
	const KvQtCorbaApp & getCorbaApp() const { return app; } 
      };
  }
}

#endif





