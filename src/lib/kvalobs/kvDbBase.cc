/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvDbBase.cc,v 1.15.2.3 2007/09/27 09:02:30 paule Exp $                                                       

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
#include <stdio.h>
#include <string.h>
#include <kvalobs/kvDbBase.h>
#include <miutil/miTimeParse.h>
#include <sstream>
#include <limits.h>
#include <float.h>

using namespace miutil;
using namespace std;

const float kvalobs::kvDbBase::FLT_NULL=FLT_MAX;
const int   kvalobs::kvDbBase::INT_NULL=INT_MAX;
const std::string kvalobs::kvDbBase::TEXT_NULL("__:#@#+@##:TEXT_NULL:Qw@$:__");



kvalobs::kvDbBase::
kvDbBase()
{
}
   
kvalobs::kvDbBase::
~kvDbBase()
{
}


miString 
kvalobs::kvDbBase::
toUpdate()const
{
  return miString();
}

miString 
kvalobs::kvDbBase::
insertQuery(bool replace) const
{
  ostringstream ost;
  ost << (replace ? "replace" : "insert" ) 
      << " into " << tableName() << " values " << toSend();
  return ost.str();
}


miString 
kvalobs::kvDbBase::
selectAllQuery() const
{
  return  selectAllQuery(tableName());
}

miString 
kvalobs::kvDbBase::
selectAllQuery(const miutil::miString &tblName)const
{
  ostringstream ost;
  ost <<"select * from " << tblName << " ";
  return ost.str();
}

miString 
kvalobs::kvDbBase::
quoted(const int& in) const
{
  return quoted(miString(in));
}

miString 
kvalobs::kvDbBase::
quoted(const miTime& in) const
{
  return quoted(in.isoTime());
}

miString 
kvalobs::kvDbBase::
quoted(const miString& in) const
{
  miString out = "\'";
  out+=in+"\'";
  return out;
}

miDate 
kvalobs::kvDbBase::
julianDayThatYear(int addOn,int year) const
{  
  miDate date;
  if(year < 1) {
    date  = miDate::today();
    year = date.year();
  }
  date.setDate(year,1,1);
  date.addDay(addOn);
  return date;
}



miutil::miTime
kvalobs::kvDbBase::
decodeTimeWithMsec( const std::string &timespec, int &msec )
{
   return miutil::isoTimeWithMsec( timespec, msec );
}

ostream& 
kvalobs::
operator<<(ostream& out, const kvalobs::kvDbBase& rhs)
{
  out << rhs.toSend();
  return out;
};


bool  
kvalobs::
operator>(const kvDbBase& lhs,const kvDbBase& rhs)
{
  return (lhs.sortBy_ >  rhs.sortBy_);
}
  
bool  
kvalobs::
operator<(const kvDbBase& lhs, const kvDbBase& rhs)
{
  return (lhs.sortBy_ <  rhs.sortBy_);
}
    
bool 
kvalobs::
operator==(const kvDbBase& lhs, const kvDbBase& rhs)
{
  return (lhs.sortBy_ == rhs.sortBy_);
}
    
bool 
kvalobs::
operator!=(const kvDbBase& lhs, const kvDbBase& rhs)
{
  return (lhs.sortBy_ != rhs.sortBy_);
}





