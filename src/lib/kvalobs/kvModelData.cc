/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvModelData.cc,v 1.4.6.2 2007/09/27 09:02:30 paule Exp $                                                       

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
#include <kvalobs/kvModelData.h>
#include <dnmithread/mtcout.h>
#include <sstream>

/*
 * Created by DNMI/IT: borge.moe@met.no
 * at Tue Aug 28 14:53:16 2002 
 */
using namespace std;
using namespace miutil;
using namespace dnmi;


bool 
kvalobs::kvModelData::set(int stationid, 
			  const miutil::miTime &obstime,    
			  int   paramid,    
			  int   level,
			  int   modelid,
			  float original)
{
  stationid_=stationid; 
  obstime_=obstime;    
  paramid_=paramid;    
  level_=level;
  modelid_=modelid;
  original_=original;

  std::ostringstream s;
  s << stationid_;
  s << paramid_;
  s << level_;
  s << obstime_.isoTime();
  sortBy_ = s.str();
  
  return true;
}

  
bool 
kvalobs::kvModelData::set(const dnmi::db::DRow &r_)
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
      }else if(*it=="paramid"){
	paramid_=atoi(buf.c_str());
      }else if(*it=="level"){
	level_=atoi(buf.c_str());
      }else if(*it=="modelid"){
	modelid_=atoi(buf.c_str());
      }else if(*it=="original"){
	sscanf(buf.c_str(),"%f", &original_);
      }
    }
    catch(...){
      CERR("kvModelData: unexpected exception ..... \n");
    }  
  }

  std::ostringstream s;
  s << stationid_;
  s << paramid_;
  s << level_;
  s << obstime_.isoTime();
  sortBy_ = s.str();
  
  return true;
}

std::string
kvalobs::kvModelData::toSend() const
{
  ostringstream ost;
 
  ost << "(" 
      << stationid_       << ","
      << quoted(obstime_) << ","         
      << paramid_         << ","         
      << level_           << ","
      << modelid_         << ","           
      << original_        << ")";
        
  return ost.str();
}

std::string
kvalobs::kvModelData::uniqueKey()const
{
  ostringstream ost;
  
  ost << " WHERE stationid=" << stationid_ << " AND "
      << "       obstime="   << quoted(obstime_.isoTime()) << " AND "
      << "       paramid="   << paramid_ << " AND "
      << "       level="     << level_   << " AND "
      << "       modelid="   << modelid_;      

  return ost.str();
}
