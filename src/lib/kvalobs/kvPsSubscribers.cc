/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvPsSubscribers.cc,v 1.1.2.3 2007/09/27 09:02:30 paule Exp $                                                       

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
#include <kvalobs/kvPsSubscribers.h>
#include <dnmithread/mtcout.h>

using namespace std;
using namespace miutil;
using namespace dnmi;


  
bool 
kvalobs::
kvPsSubscribers::
set(const  std::string &name,
    int                     subscribertype,
    const  std::string &comment,
    int                     &delete_after_hours,
    const  std::string &sior,
    const  miutil::miTime   &created)
{
	
	 name_=name;
    subscribertype=subscribertype;
    comment_=comment;
    delete_after_hours_=delete_after_hours;
    sior_=sior;
    created_=created;
    
    sortBy_=name_;

    return true;
	
}
                  
bool 
kvalobs::
kvPsSubscribers::
set(const dnmi::db::DRow& r_)
{
	db::DRow               &r=const_cast<db::DRow&>(r_);
  	string                 buf;
  	list<string>           names=r.getFieldNames();
  	list<string>::iterator it=names.begin();
 
  	for(;it!=names.end(); it++){
    	try{
     		buf=r[*it];
      
      	if(*it=="name"){
				name_=buf;
      	}else if(*it=="subscribertype"){
				subscribertype_=atoi(buf.c_str());
      	}else if(*it=="comment"){
				comment_=buf;
      	}else if(*it=="delete_after_hours"){
				delete_after_hours_=atoi(buf.c_str());
      	}else if(*it=="sior"){
				sior_=buf;
      	}else if(*it=="created"){
				created_=miTime(buf);
      	}else{
      		CERR("kvPsSubscribers: unknown coloumn name '" << *it << "'.");
      	}
    	}
    	catch(...){
      	CERR("kvPsSubscribers: unexpected exception ..... \n");
    	}  
  	}
   
   sortBy_=name_;
  
  	return true;  
}

std::string 
kvalobs::
kvPsSubscribers::
toSend() const
{
  ostringstream ost;
  
  ost << "("
      << quoted(name_)         << ","
      << subscribertype_       << ","     
      << quoted(comment_)      << ","
      << delete_after_hours_   << ","         
      << quoted(sior_)         << ","
      << quoted(created_)      <<")";
  
  return ost.str();
}



std::string
kvalobs::
kvPsSubscribers::
uniqueKey()const
{
  	ostringstream ost;
  
  	ost << " WHERE name=" << quoted(name_);

  return ost.str();
}

std::string
kvalobs::
kvPsSubscribers::
toUpdate()const
{
  ostringstream ost;
  
  ost << "SET      subscribertype=" << subscribertype_ 
      << "             ,comment=" << quoted(comment_)
      << "  ,delete_after_hours=" << delete_after_hours_
      << "                ,sior=" << quoted(sior_)
      << "             ,created=" << quoted(created_) << " "
      << "WHERE name=" << quoted(name_);

  return ost.str();
}


std::string
kvalobs::
kvPsSubscribers::
subscriberid()const
{
	string type_;
	ostringstream ost;
	
	if(name_.empty())
		return std::string();
	
	if(subscribertype_==0)
		type_="data";
	else if(subscribertype_==1)
		type_="notify";
	else
		return std::string();
	
	ost << "ps_" << type_ << "_" << name_; 	
	
	return ost.str();
}

std::string
kvalobs::
kvPsSubscribers::
nameFromSubscriberid(const std::string &subscriberid)
{
	ostringstream ost;
	CommaString cs(subscriberid, "_");
	std::string name;
	
	if(cs.size()<3)
		return "";
	
	ost << cs[2];
	
	for(int i=3; i<cs.size(); i++)
		ost << "_" << cs[i];

	return ost.str();
}


int 
kvalobs::
kvPsSubscribers::
typeFromSubscriberid(const std::string &subscriberid)
{
	ostringstream ost;
	CommaString cs(subscriberid, "_");
	
	if(cs.size()<3)
		return -1;

	if(cs[1]=="data")
		return 0;	
	
	if(cs[1]=="notify")
		return 1;
		
	return -1;
}
