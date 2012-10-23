/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvPsSubscriber.cc,v 1.1.2.2 2007/09/27 09:02:22 paule Exp $                                                       

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
#include <stdlib.h>
#include <dnmithread/mtcout.h>
#include "kvPsSubscriber.h"

using namespace std;
using namespace miutil;
using namespace dnmi;


  
bool 
kvPsSubscriber::
set(	int                  stationID,
		int                  typeID,   
   	const miutil::miTime &obstime){
	stationID_=stationID;
	typeID_   =typeID;
	obstime_  =obstime;
	tbtime_   =miTime::nowTime();
	touched_  =tbtime_;
	
	sortBy_=std::string(stationID_)+std::string(typeID_)+obstime_.isoTime();
	   		
}  

bool 
kvPsSubscriber::
set(const dnmi::db::DRow& r_)
{
	db::DRow               &r=const_cast<db::DRow&>(r_);
  	string                 buf;
  	list<string>           names=r.getFieldNames();
  	list<string>::iterator it=names.begin();
 
  	for(;it!=names.end(); it++){
    	try{
     		buf=r[*it];
      
      	if(*it=="stationid"){
				stationID_=atoi(buf.c_str());
      	}else if(*it=="typeid"){
				typeID_=atoi(buf.c_str());
      	}else if(*it=="obstime"){
				obstime_=miTime(buf);
      	}else if(*it=="tbtime"){
				tbtime_=miTime(buf);
      	}else if(*it=="touched"){
				touched_=miTime(buf);
      	}else{
      		CERR("kvPsSubscribers: unknown coloumn name '" << *it << "'.");
      	}
    	}
    	catch(...){
      	CERR("kvPsSubscriber::set: unexpected exception ..... \n");
    	}  
  	}
   
   sortBy_=std::string(stationID_)+std::string(typeID_)+obstime_.isoTime();
  
  	return true;  
}

std::string 
kvPsSubscriber::
toSend() const
{
  ostringstream ost;
  
  ost << "("
      << stationID_         << ","
      << typeID_            << ","     
      << quoted(obstime_)   << ","
      << quoted(tbtime_)    << ","         
      << quoted(touched_)   << ")" ;
  
  return ost.str();
}



miutil::std::string 
kvPsSubscriber::
uniqueKey()const
{
  	ostringstream ost;
  
  	ost << " WHERE stationid=" << stationID_ << " AND "
  		 << "          typeid=" << typeID_    << " AND "
  		 << "         obstime=" << quoted(obstime_);

  return ost.str();
}

miutil::std::string 
kvPsSubscriber::
toUpdate()const
{
	ostringstream ost;
  
 	ost << "SET      touched=" << quoted(miTime::nowTime())
  		 << " WHERE stationid=" << stationID_ << " AND "
  		 << "          typeid=" << typeID_    << " AND "
  		 << "         obstime=" << quoted(obstime_);
  
  	return ost.str();
}
