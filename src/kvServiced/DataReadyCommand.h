/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: DataReadyCommand.h,v 1.2.2.2 2007/09/27 09:02:39 paule Exp $                                                       

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
#ifndef __DataReadyCommand_h__
#define __DataReadyCommand_h__

#include <string>
#include <kvalobs/kvStationInfo.h>
#include <kvalobs/kvStationInfoCommand.h>
#include <kvskel/managerInput.hh>

class DataReadyCommand : public kvalobs::StationInfoCommand {

 protected:
  DataReadyCommand();
  DataReadyCommand(const DataReadyCommand &);
  DataReadyCommand& operator=(const DataReadyCommand &);
  CKvalObs::CManager::CheckedInput_var callback;
  std::string source_;

 public:

  DataReadyCommand(const CKvalObs::StationInfoList &stInfo,
                   const CKvalObs::CManager::CheckedInput_ptr callback);

  DataReadyCommand(const char *source, const CKvalObs::StationInfoList &stInfo,
                   const CKvalObs::CManager::CheckedInput_ptr callback);

  DataReadyCommand(const char *source,
                   const CKvalObs::StationInfoExtList &stInfo,
                   const CKvalObs::CManager::CheckedInput_ptr callback);

  ~DataReadyCommand() {
  }
  ;

  CKvalObs::CManager::CheckedInput_ptr getCallback() const;

  std::string source() const {
    return source_;
  }
  bool executeImpl() {
  }
  ;
  //Do nothing

};

#endif
