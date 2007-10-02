/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvsynopdImpl.h,v 1.5.2.4 2007/09/27 09:02:23 paule Exp $                                                       

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
#ifndef __kvsynopdImpl_h__
#define __kvsynopdImpl_h__

#include <kvskel/kvsynopd.hh>
#include "obsevent.h"
#include <kvskel/adminInterface.h>
#include "App.h"

class kvSynopdImpl: public virtual POA_kvsynopd::synop,
		    public virtual micutil::AdminInterface,
		    public PortableServer::RefCountServantBase 
{
  App &app;
  dnmi::thread::CommandQue &que;

public:
  // standard constructor
  kvSynopdImpl(App &app_, dnmi::thread::CommandQue &que_);
  virtual ~kvSynopdImpl();


  CORBA::Boolean createSynop(CORBA::Short wmono, 
			     const char* obstime, 
			     const micutil::KeyValList& keyVals,
			     kvsynopd::synopcb_ptr callback);
  CORBA::Boolean stations(kvsynopd::StationInfoList_out infoList);
  CORBA::Boolean uptime(CORBA::String_out startTime, 
			CORBA::Long& uptimeInSeconds);
  CORBA::Boolean delays(CORBA::String_out nowTime, 
			kvsynopd::DelayList_out dl);
  CORBA::Boolean reloadConf(CORBA::String_out message);
  CORBA::Boolean reloadCache(const char* fromTime, 
			     kvsynopd::ReloadList_out wmonolist, 
			     CORBA::String_out message);
  CORBA::Boolean getsynop(const kvsynopd::WmoNoList& wmoList, 
			  const char* fromtime, const char* totime, 
			  kvsynopd::SynopList_out synops,
			  CORBA::String_out message);
  CORBA::Boolean getdata(CORBA::Short wmono, const char* obstime, 
			 kvsynopd::DataElementList_out datalist,
			 CORBA::String_out message);


};




#endif
