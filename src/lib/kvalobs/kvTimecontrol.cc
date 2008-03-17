/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvTimecontrol.cc,v 1.4.6.2 2007/09/27 09:02:31 paule Exp $                                                       

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
#include <kvalobs/kvTimecontrol.h>

using namespace std;
using namespace miutil;

miString  kvalobs::kvTimecontrol::toSend() const
{
  ostringstream ost;
  ost << "("
      << fromday_     << ","
      << today_       << ","
      << time_        << ","
      << priority_    << ","     
      << quoted(qcx_) << ")";      

  return ost.str();
}

bool kvalobs::kvTimecontrol::set( int fromday__,
				  int today__,
				  int time__,
				  int priority__,
				  const miutil::miString& qcx__)
{
  fromday_ = fromday__; 
  today_   = today__;   
  time_    = time__;    
  priority_= priority__;
  qcx_     = qcx__;     
  sortBy_  = miString(fromday_)+ miString(today_) + miString(time_);
  return true;
}




bool kvalobs::kvTimecontrol::set(const dnmi::db::DRow& r_)
{
   dnmi::db::DRow &r=const_cast<dnmi::db::DRow&>(r_);
  string       buf;
  list<string> names=r.getFieldNames();
  list<string>::iterator it=names.begin();
  

  for(;it!=names.end(); it++){
    try{
      buf=r[*it];
      if(*it=="fromday"){
	fromday_=atoi(buf.c_str());
      }else if(*it=="today"){
	today_=atoi(buf.c_str());
      }else if(*it=="time"){
	time_=atoi(buf.c_str());
      }else if(*it=="priority"){
	priority_=atoi(buf.c_str());
      }else if(*it=="qcx"){
	qcx_=buf;
      }
    }
    catch(...){
      CERR("kvTimeControl: exception ..... \n");
    }  
  }
  
  sortBy_= miString(fromday_) + miString(today_) + miString(time_);
  return true;
}



miutil::miString 
kvalobs::kvTimecontrol::uniqueKey()const
{
  ostringstream ost;
  
  ost << " WHERE fromday=" << fromday_  << " AND "
      << "       today="   << today_    << " AND "
      << "       time="    << time_     << " AND "
      << "       priority="    << priority_;

  return ost.str();
}
