/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvCorbaThread.h,v 1.2.2.3 2007/09/27 09:02:45 paule Exp $                                                       

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
#ifndef __kvservice_priv_kvCorbaThread_h__
#define __kvservice_priv_kvCorbaThread_h__

#include <omnithread.h>
#include <miconfparser/miconfparser.h>
#include "kvDataNotifySubscriberImpl.h"

#include <corbahelper/corbaApp.h>


namespace kvservice{
  namespace priv{
    class KvCorbaThread : public omni_thread
      {
	KvCorbaThread(const KvCorbaThread &);
	KvCorbaThread();
	KvCorbaThread &operator=(const KvCorbaThread&);
	
	//KvCorbaApp  app;
	CorbaHelper::CorbaApp app;
	bool        isInitialized_;
	
      public:
	KvCorbaThread(int argn, char **argv, 
		      miutil::conf::ConfSection *conf,
		      const char *options[][2]=0);

	virtual ~KvCorbaThread(){} 

	
	void *run_undetached(void*);
	
	bool  isInitialized()const{ return  isInitialized_;}
      };
  }
}

#endif





