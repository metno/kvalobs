/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: QaWorkCommand.h,v 1.7.2.2 2007/09/27 09:02:37 paule Exp $                                                       

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
#ifndef __QaWorkCommand_h__
#define __QaWorkCommand_h__

#include <kvskel/commonStationInfo.hh>
#include <kvalobs/kvStationInfo.h>
#include <kvalobs/kvStationInfoCommand.h>
#include <kvskel/managerInput.hh>
#include <map>
#include <string>
#include <kvskel/KeyVal.hh>



/**
   \brief Command to QaBase work-queue
   
*/

class QaWorkCommand : public kvalobs::StationInfoCommand{
 public:
  typedef std::map<std::string, std::string>                   TKeyValList;
  typedef std::map<std::string, std::string>::iterator        ITKeyValList;
  typedef std::map<std::string, std::string>::const_iterator CITKeyValList;

 protected:
  QaWorkCommand();
  QaWorkCommand(const QaWorkCommand &);
  QaWorkCommand& operator=(const QaWorkCommand &);
  CKvalObs::CManager::CheckedInput_var callback;
  TKeyValList keyvals;
  

 public:
  QaWorkCommand(const CKvalObs::StationInfo &stInfo);
  QaWorkCommand(const CKvalObs::StationInfo &stInfo, 
		const CKvalObs::CManager::CheckedInput_ptr callback,
		const micutil::KeyValList &args);
  ~QaWorkCommand(){};
  
  bool getKey(const std::string &key, std::string &val)const;
  CKvalObs::CManager::CheckedInput_ptr getCallback()const;
  
  bool       executeImpl(){}; //Do nothing

};

#endif
