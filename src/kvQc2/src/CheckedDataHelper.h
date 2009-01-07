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

///Borrowed from kvalobs Qc1 kvmanagerd to send messages to the kvServiced.

#ifndef __CheckedDataHelper_h__
#define __CheckedDataHelper_h__

#include <kvdb/kvdb.h>
#include <kvalobs/kvStationInfo.h>
#include "Qc2App.h"

class CheckedDataCommandBase;

class CheckedDataHelper
{
  Qc2App  &app;
  dnmi::db::Connection *con;

  friend class  CheckedDataCommandBase;
  bool  serviceAlive_;

 public:
  CheckedDataHelper(Qc2App &app_)
    :app(app_), con(0)
    {}
  
  bool                  serviceAlive()const{ return serviceAlive_;}
  void                  connection(dnmi::db::Connection *con_){con=con_;}
  dnmi::db::Connection* connection()const{ return con;}
  bool sendDataToService(const kvalobs::kvStationInfoList &si);
};


#endif
