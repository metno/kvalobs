/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: SendDataToQaBase.h,v 1.2.6.3 2007/09/27 09:02:38 paule Exp $                                                       

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
#ifndef __SendDataToQaBase_h__
#define __SendDataToQaBase_h__

#include <kvskel/qabase.hh>
#include <dnmithread/CommandQue.h>
#include <puTools/miString>
#include <vector>
#include <puTools/miTime>
#include <string>
/**
 *
 */ 

class SendDataToQaBase{
  dnmi::thread::CommandQue      &inputque;
  miutil::miTime                stime_start;
  miutil::miTime                stime_stop;
  std::string                   logpath;
  std::vector<miutil::miString> &stations;
  std::string                   type;
  CKvalObs::CQaBase::QaBaseInput_var refQaBase;
  CKvalObs::CManager::CheckedInput_var callback;

 public:

  SendDataToQaBase(std::vector<miutil::miString> &stations_,
  		             std::string &type_,
		   miutil::miTime time_start_,
		   miutil::miTime time_stop_,
		   const std::string &logpath_,
		   dnmi::thread::CommandQue &que,
		   CKvalObs::CQaBase::QaBaseInput_ptr ptr,
		   CKvalObs::CManager::CheckedInput_ptr callback_
		   )
    :inputque(que), stime_start(time_start_), stime_stop(time_stop_),
    logpath(logpath_),stations(stations_), type(type_), refQaBase(ptr),callback(callback_)
    {
    }

  void
    createDummyData(CKvalObs::StationInfo &st, 
		     const std::string& spos, 
		     const std::string& stime);
  
  void operator()();
};

#endif
