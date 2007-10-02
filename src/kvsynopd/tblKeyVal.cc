/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: tblKeyVal.cc,v 1.1.6.2 2007/09/27 09:02:23 paule Exp $                                                       

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
#include "tblKeyVal.h"
#include <milog/milog.h>

using namespace std;
using namespace miutil;
using namespace dnmi;

void 
TblKeyVal::
createSortIndex() 
{
  sortBy_=key_;
}
  
void 
TblKeyVal::
clean()
{
  key_.erase();
  val_.erase();

  createSortIndex();
}


bool 
TblKeyVal::
set(const dnmi::db::DRow &r_)
{
  db::DRow               &r=const_cast<db::DRow&>(r_);
  string                 buf;
  list<string>           names=r.getFieldNames();
  list<string>::iterator it=names.begin();
 
  for(;it!=names.end(); it++){
    try{
      buf=r[*it];
      
      if(*it=="key"){
	key_=buf;
      }else if(*it=="val"){
	val_=buf;
      }else{
	LOGWARN("TblKeyVal::set .. unknown entry:" << *it << std::endl);
      }
    }
    catch(...){
      LOGWARN("TblKeyVal: unexpected exception ..... \n");
    }  
  }
 
  createSortIndex();
  return true;
}

bool 
TblKeyVal::
set(const TblKeyVal &s)
{
  key_ = s.key_;  
  val_ = s.val_;  

  createSortIndex();

  return true;
}



bool 
TblKeyVal::
set(const std::string &key,
    const std::string &val)
{
  key_ = key;  
  val_ = val;  

  createSortIndex();

  return true;
}

miutil::miString 
TblKeyVal::
toSend() const
{
  ostringstream ost;
 
  ost << "(" 
      << quoted(key_)   << ","         
      << quoted(val_)   
      << ")";      

  return ost.str();
}


miutil::miString 
TblKeyVal::
uniqueKey()const
{
  ostringstream ost;
  
  ost << " WHERE key="  << quoted(key_);

  return ost.str();
}



miutil::miString 
TblKeyVal::
toUpdate()const
{
  ostringstream ost;
  
  ost << "SET val=" << quoted(val_)     
      << " WHERE   key=" << quoted(key_);

  return ost.str();
}
