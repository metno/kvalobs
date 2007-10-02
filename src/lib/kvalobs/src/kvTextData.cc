/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvTextData.cc,v 1.7.6.2 2007/09/27 09:02:31 paule Exp $                                                       

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
#include <kvalobs/kvTextData.h> 

using namespace std;
using namespace miutil;


miString kvalobs::kvTextData::toSend() const
{
   ostringstream ost;
   ost << "("
       << stationid_         << ","
       << quoted(obstime_)   << ","
       << quoted(original_)  << ","
       << paramid_           << ","
       << quoted(tbtime_)    << ","
       << typeid_            << ")";
   return ost.str();
}


bool kvalobs::kvTextData::set( int                     sta,
                               const miutil::miTime&   obt,
                               const miutil::miString& org,
                               int                     pid,
                               const miutil::miTime&   tbt,
                               int                     typ )
{
  stationid_ = sta;
  obstime_   = obt;
  original_  = org;
  paramid_   = pid;
  tbtime_    = tbt;
  typeid_    = typ;
  sortBy_    = miString(sta)+obt.isoTime();
  return true;
}


bool kvalobs::kvTextData::set(const dnmi::db::DRow& r_)
{
  dnmi::db::DRow &       r     = const_cast<dnmi::db::DRow&>(r_);
  list<string>           names = r.getFieldNames();
  list<string>::iterator it    = names.begin();
  miString               buf;

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
      }else if(*it=="tbtime"){
        tbtime_=miTime(buf);
      }else if(*it=="typeid"){
        typeid_=atoi(buf.c_str());
      }
    }
    catch(...){
      CERR("kvTextData: exception ..... \n");
    }
  }
  sortBy_= miString(stationid_)+obstime_.isoTime();
  return true;
}

miutil::miString 
kvalobs::kvTextData::uniqueKey()const
{
  ostringstream ost;
  
  ost << " WHERE stationid=" << stationid_                 << " AND "
      << "       obstime="   << quoted(obstime_.isoTime()) << " AND "
      << "       paramid="   << paramid_                   << " AND "
      << "       typeid="    << typeid_;

  return ost.str();
}

