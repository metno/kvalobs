/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: StdLayout.cc,v 1.3.2.2 2007/09/27 09:02:32 paule Exp $                                                       

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
#include <milog/StdLayout1.h>
#include <time.h>
#include <sstream>

using namespace std;

milog::StdLayout1::StdLayout1()
{
}

milog::StdLayout1::~StdLayout1()
{
}

std::string 
milog::StdLayout1::formatMessage(const std::string &msg,
				LogLevel ll,
				const std::string &context)
{
  std::stringstream os;
  struct tm tm_;
  time_t    t;
  char      tb[32];
  string    ss;
  string    dash;
  string::size_type i;
  string::size_type prev;
  

  time(&t);
  localtime_r(&t, &tm_);

  sprintf(tb, "%04d-%02d-%02d %02d:%02d:%02d ",
	  tm_.tm_year+1900, tm_.tm_mon+1, tm_.tm_mday,
	  tm_.tm_hour, tm_.tm_min, tm_.tm_sec);

  os << tb;

  switch(ll){
  case NOTSET: os << "NOTSET "; break;
  case FATAL:  os << "FATAL ";  break;
  case ERROR:  os << "ERROR ";  break;
  case WARN:   os << "WARN ";   break;
  case INFO:   os << "INFO ";   break;
  case  DEBUG: os << "DEBUG ";  break;
  case  DEBUG1: os << "DEBUG1 ";  break;
  case  DEBUG2: os << "DEBUG2 ";  break;
  case  DEBUG3: os << "DEBUG3 ";  break;
  case  DEBUG4: os << "DEBUG4 ";  break;
  case  DEBUG5: os << "DEBUG5 ";  break;
  case  DEBUG6: os << "DEBUG6 ";  break;
  }

  if( ! context.empty()){
      os << "(" << context << "): ";
  } else {
     os << ": ";
  }

  if(msg.empty())
      return os.str() << endl;

  dash = string( os.str().length()-1, '-' )+" ";

  prev = msg.find_first_not_of("\n", 0);

  if(prev==string::npos)
      return os.str();

  bool first=true;
  i=msg.find("\n", prev);

  while(i!=string::npos){
      ss=msg.substr(prev, i-prev+1); //Includes the endl

      if( first ) {
         os << ss;
         first=false;
      } else {
         os << dash << ss;
      }

      prev=i+1;
      i=msg.find("\n", prev);
  }

  if(prev<msg.length()){
      ss=msg.substr(prev);
      if( first )
         os << ss << endl;
      else
         os << dash << ss << endl;
  }

  return os.str();
}


