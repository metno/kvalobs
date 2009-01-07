/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id$                                                       

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
#ifndef __CheckedDataCommandBase_h__
#define __CheckedDataCommandBase_h__

#include <kvskel/commonStationInfo.hh>
#include <kvalobs/kvStationInfoCommand.h>
#include "Qc2App.h"
#include <kvdb/kvdb.h>
#include "CheckedDataHelper.h"

///Qc2 Context:
///A class definition necessary for CheckedDataHelper taken from the Kvalobs Qc1
///manager. Necessary for communicating to the kvServiced that an update has been
///made to the database.

class  CheckedDataCommandBase : public kvalobs::StationInfoCommand{
  CheckedDataCommandBase();
  CheckedDataCommandBase(const CheckedDataCommandBase &);
  CheckedDataCommandBase& operator=(const CheckedDataCommandBase &);
  
 protected:
  CheckedDataHelper *helper_;
  
 public:
  CheckedDataCommandBase(const CKvalObs::StationInfoList &stInfo)
    :kvalobs::StationInfoCommand(stInfo), helper_(0)
    {}

  ~CheckedDataCommandBase(){};

  CheckedDataHelper& helper(){ return *helper_;}
  void helper(CheckedDataHelper *helper__)
             {
	       helper_=helper__;
	       helper_->serviceAlive_=true;
	     }
  
};

#endif
