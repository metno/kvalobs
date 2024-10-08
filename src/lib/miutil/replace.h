/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: miTimeParse.h,v 1.1.2.2 2007/09/27 09:02:32 paule Exp $                                                       

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

#ifndef __REPLACE_H__
#define __REPLACE_H__

#include <string>

namespace miutil {

/**
 * @addtogroup miutil
 * @{
 */

/* @brif replace a substrings in a string with new content.
 * 
 * @param source the string to replace content in.
 * @param what The content to be replaced.
 * @param with The new content.
 * @return a reference to source.
 */
std::string&
replace(std::string &source, const std::string &what, const std::string &with);

/* @brif replace a substrings in a string with new content.
 * 
 * @param source the string to replace content in.
 * @param what The content to be replaced.
 * @param with The new content.
 * @return a reference to source.
 */
std::string
replace_copy(std::string &source, const std::string &what, const std::string &with);
}

#endif /*REPLACE_H_*/
