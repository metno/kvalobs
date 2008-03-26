/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvWorkelement.cc,v 1.2.2.3 2007/09/27 09:02:31 paule Exp $                                                       

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
#include <kvalobs/kvWorkelement.h>

using namespace miutil;
using namespace std;
using namespace dnmi;

void 
kvalobs::kvWorkelement::
createSortIndex()
{
  sortBy_=miString(stationid_)+obstime_.isoTime()+miString(typeid_);
}

kvalobs::kvWorkelement::
kvWorkelement(const kvWorkelement &we)
  : stationid_(we.stationid_),obstime_(we.obstime_),typeid_(we.typeid_),
    tbtime_(we.tbtime_), priority_(we.priority_), 
    process_start_(we.process_start_), qa_start_(we.qa_start_),
    qa_stop_(we.qa_stop_), service_start_(we.service_start_),
    service_stop_(we.service_stop_)

{
  createSortIndex();
}

kvalobs::kvWorkelement& 
kvalobs::kvWorkelement::
operator=(const kvWorkelement &rhs)
{
  if(&rhs!=this){
    stationid_=rhs.stationid_;
    obstime_=rhs.obstime_;
    typeid_=rhs.typeid_;
    tbtime_=rhs.tbtime_;
    priority_=rhs.priority_;
    process_start_=rhs.process_start_;
    qa_start_=rhs.qa_start_;
    qa_stop_=rhs.qa_stop_;
    service_start_=rhs.service_start_;
    service_stop_=rhs.service_stop_;
  
    createSortIndex();
  }

  return *this;
}


bool 
kvalobs::kvWorkelement::
set(int                  sid, 
    const miutil::miTime &obt,
    int                  tid,
    const miutil::miTime &tbt,
    int                  pri,
    const miutil::miTime &process_start,
    const miutil::miTime &qa_start,
    const miutil::miTime &qa_stop,
    const miutil::miTime &service_start,
    const miutil::miTime &service_stop)
{
  stationid_=sid; 
  obstime_=obt;
  typeid_=tid;
  tbtime_=tbt;
  priority_=pri;
  process_start_=process_start;
  qa_start_=qa_start;
  qa_stop_=qa_stop;
  service_start_=service_start;
  service_stop_=service_stop;

  createSortIndex();
}

bool 
kvalobs::kvWorkelement::
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
      }else if(*it=="typeid"){
	typeid_=atoi(buf.c_str());
      }else if(*it=="tbtime"){
	tbtime_=miTime(buf);
      }else if(*it=="priority"){
	priority_=atoi(buf.c_str());
      }else if(*it=="process_start"){
	if(!buf.empty())
	  process_start_=miTime(buf);
      }else if(*it=="qa_start"){
	if(!buf.empty())
	  qa_start_=miTime(buf);
      }else if(*it=="qa_stop"){
	if(!buf.empty())
	  qa_stop_=miTime(buf);
      }else if(*it=="service_start"){
	if(!buf.empty())
	  service_start_=miTime(buf);
      }else if(*it=="service_stop"){
	if(!buf.empty())
	  service_stop_=miTime(buf);
      }else{
	CERR("kvWorkelement::set .. unknown entry:" << *it << std::endl);
      }
    }
    catch(...){
      CERR("kvWorkelement: unexpected exception ..... \n");
    }  
  }
 
  createSortIndex();
  return true;

}

miutil::miString 
kvalobs::kvWorkelement::
toSend()const
{
  ostringstream ost;
  
  ost << "(" 
      << stationid_             << ","
      << quoted(obstime_)       << ","         
      << typeid_                << ","          
      << quoted(tbtime_)        << ","          
      << priority_              << ","
      << (process_start_.undef()?"NULL":quoted(process_start_)) << "," 
      << (qa_start_.undef()?"NULL":quoted(qa_start_))           << ","      
      << (qa_stop_.undef()?"NULL":quoted(qa_stop_))             << ","      
      << (service_start_.undef()?"NULL":quoted(service_start_)) << ","      
      << (service_stop_.undef()?"NULL":quoted(service_stop_))   << ")";
 

  return ost.str();
}

miutil::miString 
kvalobs::kvWorkelement::
toUpdate()const
{
  ostringstream ost;
  bool comma=false;

  ost << "SET ";
  
  if(!process_start_.undef()){
    comma=true;
    ost << "process_start=" << quoted(process_start_);
  }
  
  if(!qa_start_.undef()){
    if(comma)
      ost <<", ";
    
    comma=true;
    ost << "qa_start=" << quoted(qa_start_);
  }
  
  if(!qa_stop_.undef()){
    if(comma)
      ost <<", ";
    
    comma=true;
    ost << "qa_stop=" << quoted(qa_stop_);
  }

  if(!service_start_.undef()){
    if(comma)
      ost <<", ";
    
    comma=true;
    ost << "service_start=" << quoted(service_start_);
  }
   
  if(!service_stop_.undef()){
    if(comma)
      ost <<", ";
    
    comma=true;
    ost << "service_stop=" << quoted(service_stop_);
  }  
  
  ost << " WHERE stationid=" << stationid_ << " AND "
      << "         obstime=" << quoted(obstime_.isoTime()) << " AND "
      << "          typeid=" << typeid_;

  return ost.str();

}

miutil::miString 
kvalobs::kvWorkelement::
uniqueKey()const
{
  ostringstream ost;
  
  ost << " WHERE stationid=" << stationid_ << " AND "
      << "       obstime="   << quoted(obstime_.isoTime()) << " AND "
      << "       typeid="    << typeid_;
  
  return ost.str();
}

void 
kvalobs::kvWorkelement::
process_start(const miutil::miTime &start)
{
  process_start_=start;
}

void 
kvalobs::kvWorkelement::
qa_start(const miutil::miTime &start)
{
  qa_start_=start;
}
 
void 
kvalobs::kvWorkelement::
qa_stop(const miutil::miTime &stop)
{
  qa_stop_=stop;
}

void 
kvalobs::kvWorkelement::
service_start(const miutil::miTime &start)
{
  service_start_=start;
}

void 
kvalobs::kvWorkelement::
service_stop(const miutil::miTime &stop)
{
  service_stop_=stop;
}
