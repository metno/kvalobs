/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: pgsqldb.h,v 1.2.2.2 2007/09/27 09:02:26 paule Exp $                                                       

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
#ifndef __kvdb_utild__h__
#define __kvdb_utild__h__

#include <string>
#include <vector>
#include <list>

namespace utils{
std::string
rtrim(const std::string& v, const std::string& trimset = " \t\r\n");

std::string
ltrim(const std::string& v, const std::string& trimset = " \t\r\n");

std::string
trim(const std::string& v, const std::string& trimset = " \t\r\n");

std::vector<std::string>
splitstr(const std::string& val, char ch, bool compress = true);

bool
findKeyVals(const std::string &confstr, std::list<std::string> *keyvals, const std::string &sep="=");

}
#endif