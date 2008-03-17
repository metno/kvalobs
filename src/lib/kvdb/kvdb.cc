/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: db.cc,v 1.6.6.2 2007/09/27 09:02:25 paule Exp $                                                       

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
#include "kvdb.h"


void
dnmi::db::DRow::setResult(Result *r)
{
  result=r;
}


std::string& 
dnmi::db::DRow::operator[](int i)
{
    if(!result)
	throw SQLException("Programming error, DRow not initialized.");
  
    if(i>=size()){
	char buf[100];
	sprintf(buf, "Range error (size=%d  index=%d)!", size(), i);
	throw SQLException(buf);
    }

    return Row::operator[](i);

}


std::string& 
dnmi::db::DRow::operator[](const std::string &fieldName)
{
    if(!result)
	throw SQLException("Programming error, DRow not initialized.");

    int i=result->fieldIndex(fieldName);
  
    if(i<0){
	throw SQLException("Fieldname error: "+fieldName);
    }
  
    return Row::operator[](i);
}
    

dnmi::db::CIDRow 
dnmi::db::DRow::begin()const
{
    if(!result)
	throw SQLException("Programming error, DRow not initialized.");

    return Row::begin();
}

dnmi::db::CIDRow 
dnmi::db::DRow::end()const
{
    if(!result)
	throw SQLException("Programming error, DRow not initialized.");
  
    return Row::end();
}


int      
dnmi::db::DRow::fields()const
{
  return result->fields();
}


std::string 
dnmi::db::DRow::fieldName(int i)const
{
  return result->fieldName(i);
}

std::list<std::string> 
dnmi::db::DRow::getFieldNames()const
{
  return result->getFieldNames();
}


dnmi::db::DRow &
dnmi::db::Result::next()
{ 
  data.setResult(this);
  nextImpl();
  return data;
}



std::list<std::string> 
dnmi::db::Result::getFieldNames()const
{
  std::list<std::string> ret;
  int n=fields();
  
  for(int i=0; i<n; i++)
    ret.push_back(fieldName(i));

  return ret;
}






























