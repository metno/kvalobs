/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvPath.cc,v 1.1.6.3 2007/09/27 09:02:30 paule Exp $                                                       

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
#include <iostream>
#include <boost/algorithm/string/replace.hpp>
#include <kvalobs/kvPath.h>

std::string 
kvPath( const std::string &name, const std::string &system )
{
  std::string ret;
  
  if( name=="pkglibdir" )
     ret = PKGLIBDIR;
  else if( name=="sysconfdir" )
     ret = SYSCONFDIR+std::string("/" + system);
  else if( name=="libdir" )
     ret = LIBDIR;
  else if( name=="bindir" )
     ret = BINDIR;
  else if( name=="datadir" )
     ret = DATADIR+std::string("/" + system );
  else if( name=="localstatedir" )
     ret = LOCALSTATEDIR+std::string("/lib/" + system);
  else if( name=="logdir")
	  ret = LOCALSTATEDIR+std::string("/log/" + system);
  else if( name=="prefix")
     ret = PREFIX;
  else {
    std::cerr << "FATAL: The 'name' (" << name << ") is NOT recognised!\n";
  }

  boost::algorithm::replace_all(ret, "//", "/");
  
  return ret;
}
