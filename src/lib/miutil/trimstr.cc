/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: trimstr.cc,v 1.1.6.2 2007/09/27 09:02:32 paule Exp $                                                       

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
#include "trimstr.h"

void
miutil::trimstr(std::string &str, ETrimStrWhere where, const char *trimset)
{
    std::string::size_type pos;
    int len;

    if(str.length()==0)
	return;

    if(where==TRIMFRONT || where==TRIMBOTH)  //Trim front
    {
	pos=str.find_first_not_of(trimset);

	if(pos==std::string::npos)
	    str.erase();
	else if(pos>0)
	    str.erase(0, pos);
    }

    len=str.length();

    if(len>0 && (where==TRIMBACK || where==TRIMBOTH))  //Trim end
    {
	pos=str.find_last_not_of(trimset);
	
	if(pos==std::string::npos)
	    str.erase();
	else if(pos<(len-1))
	    str.erase(pos+1, len-pos-1);
    }
}
	
