/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvWhat.cc,v 1.2.2.3 2007/09/27 09:02:46 paule Exp $                                                       

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
#include "../include/kvWhat.h"

kvservice::KvWhat::KvWhat(const KvWhat &w):
  stationID_(w.stationID_), typeID_(w.typeID_), qcID_(w.qcID_), 
  obsTime_(w.obsTime_)

{
}


kvservice::KvWhat::KvWhat(
	const CKvalObs::CService::kvDataNotifySubscriber::What &what)
{
  stationID_=what.stationID;
  typeID_=what.typeID_;
  obsTime_.setTime(miutil::miString(what.obsTime));
  
  for(CORBA::ULong i=0; i<what.qc.length(); i++)
    qcID_[what.qc[i]]=true; 

}

      
kvservice::KvWhat& 
kvservice::KvWhat::operator=(const kvservice::KvWhat &w)
{
  if(this!=&w){
      stationID_=w.stationID_;
      typeID_=w.typeID_;
      qcID_=w.qcID_; 
      obsTime_=w.obsTime_;
  }

  return *this;
}
