/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: Data.cc,v 1.3.6.3 2007/09/27 09:02:22 paule Exp $                                                       

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
#include <list>
#include <stdlib.h>
#include <stdio.h>
#include "Data.h"
#include <milog/milog.h>

using namespace std;
using namespace miutil;
using namespace dnmi;

void 
Data::
createSortIndex() 
{
  sortBy_=miString(stationid_)+miString(paramid_)+miString(sensor_)+
    miString(level_)+obstime_.isoTime();
}
  
void 
Data::
clean()
{
  stationid_   = 0;  
  obstime_     = miTime::nowTime();  
  original_.erase();
  paramid_     = 0;     
  typeid_      = 0;      
  sensor_      = 0;      
  level_       = 0;       

  createSortIndex();
}

bool
Data::
set(const kvalobs::kvData &data)
{
  	char buf[100];

  	sprintf(buf, "%.2f", data.original());
  
  	set(data.stationID(), data.obstime(), buf, data.paramID(), 
      	data.typeID(), data.sensor(), data.level()); 
      
	return true;
}


bool 
Data::
set(const dnmi::db::DRow &r_)
{
  db::DRow               &r=const_cast<db::DRow&>(r_);
  string                 buf;
  list<string>           names=r.getFieldNames();
  list<string>::iterator it=names.begin();
 
  for(;it!=names.end(); it++){
    try{
      buf=r[*it];
      
      if(*it=="stationid"){
	stationid_=atoi(buf.c_str());
      }else if(*it=="obstime"){
	obstime_=miTime(buf);
      }else if(*it=="original"){
	original_=buf;
      }else if(*it=="paramid"){
	paramid_=atoi(buf.c_str());
      }else if(*it=="typeid"){
	typeid_=atoi(buf.c_str());
      }else if(*it=="sensor"){
	sensor_=atoi(buf.c_str());
      }else if(*it=="level"){
	level_=atoi(buf.c_str());
      }else{
	LOGWARN("Data::set .. unknown entry:" << *it << std::endl);
      }
    }
    catch(...){
      LOGWARN("Data: unexpected exception ..... \n");
    }  
  }
 
  createSortIndex();
  return true;
}

bool 
Data::
set(int pos, const miutil::miTime &obt,    
    const std::string &org, int par, int   typ, int sen, 
    int lvl)
{
  stationid_   = pos;   
  obstime_     = obt;     
  original_    = org;    
  paramid_     = par;     
  typeid_      = typ;      
  sensor_      = sen;      
  level_       = lvl;       

  createSortIndex();

  return true;
}

miutil::miString 
Data::toSend() const
{
  ostringstream ost;
 
  ost << "(" 
      << stationid_       << ","
      << quoted(obstime_) << ","         
      << quoted(original_)<< ","        
      << paramid_         << ","         
      << typeid_          << ","          
      << quoted(sensor_)  << ","
      << level_           << ")";      

  return ost.str();
}


miutil::miString 
Data::
uniqueKey()const
{
  ostringstream ost;
  
  ost << " WHERE stationid=" << stationid_ << " AND "
      << "       obstime="   << quoted(obstime_.isoTime()) << " AND "
      << "       paramid="   << paramid_ << " AND "
      << "       typeid="    << typeid_ << " AND "
      << "       sensor="    << quoted(sensor_) << " AND "
      << "       level="     << level_;

  return ost.str();
}



miutil::miString 
Data::
toUpdate()const
{
  ostringstream ost;
  
  ost << "SET original=" << quoted(original_)
      << " WHERE stationid=" << stationid_ << " AND "
      << "       obstime="   << quoted(obstime_.isoTime()) << " AND "
      << "       paramid="   << paramid_ << " AND "
      << "       typeid="    << typeid_ << " AND "
      << "       sensor="    << quoted(sensor_) << " AND "
      << "       level="     << level_;

  return ost.str();

}
