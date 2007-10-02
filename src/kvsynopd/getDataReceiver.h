/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: getDataReceiver.h,v 1.1.2.5 2007/09/27 09:02:23 paule Exp $                                                       

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
#ifndef __kvsynop_getDataReceiver_h__
#define __kvsynop_getDataReceiver_h__

#include <boost/shared_ptr.hpp>
#include <kvservice/kvcpp/KvGetDataReceiver.h>
#include <kvalobs/kvDbGate.h>
#include "App.h"

class GetKvDataReceiver : public kvservice::KvGetDataReceiver{

  App                      &app;
  kvalobs::kvDbGate        &gate;
  dnmi::thread::CommandQue &que;
  miutil::miTime           fromTime; //generate Synop from this time
  const std::string        logid; 

 public:
  GetKvDataReceiver(App                      &app_, 
		    const miutil::miTime     &fromTime_,
		    dnmi::thread::CommandQue &que_,
		    kvalobs::kvDbGate        &gate_,
		    const std::string        &logid_=""):
    app(app_),
    gate(gate_),
    que(que_),
    fromTime(fromTime_),
    logid(logid_)
    {}
    
    virtual ~GetKvDataReceiver(){};

  bool next(kvservice::KvObsDataList &datalist);
};

#endif
