/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: paramlist.cc,v 1.3.2.2 2007/09/27 09:02:31 paule Exp $                                                       

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
#include <kvalobs/paramlist.h>


std::string
findParamIdInList(const ParamList &pl, int id)
{
  CIParamList it=pl.begin();
  
  for(;it!=pl.end(); it++){
    if(it->id()==id)
      return it->kode();
  }

  return std::string();
}

std::ostream& 
operator<<(std::ostream &os, const ParamList &p)
{
  CIParamList it=p.begin();
  
  os << "ParamList:\n";
  
  for(;it!=p.end(); it++)
    os << "  " << it->kode() << "  " << it->id() << std::endl;

  return os;
}
  


