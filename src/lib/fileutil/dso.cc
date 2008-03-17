/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: dso.cc,v 1.2.6.2 2007/09/27 09:02:28 paule Exp $                                                       

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
#include <dlfcn.h>
#include <string>
#include "dso.h"


dnmi::file::DSO::DSO(const std::string &dsoFile, bool resolveNow):handle(0)
{
  load(dsoFile, resolveNow);
}

dnmi::file::DSO::~DSO()
{
  if(handle)
    dlclose(handle);
}
            
void  
dnmi::file::DSO::load(const std::string &dsoFile, bool resolveNow)
{
  if(handle)
    throw   DSOException(std::string("Shared object allready open <")+
		       dsofile + std::string(">!"));
  
  if(resolveNow)
    handle=dlopen(dsoFile.c_str(), RTLD_GLOBAL | RTLD_NOW);
  else
    handle=dlopen(dsoFile.c_str(), RTLD_GLOBAL | RTLD_LAZY);

  if(!handle){
    const char *err=dlerror();
    
    if(err)
      throw DSOException(err);
    else
      throw DSOException("UNKNOWN ERROR: dlopen!\n");
  }

  dsofile=dsoFile;
}


std::string 
dnmi::file::DSO::getLastError()
{
  char *error=dlerror();
  
  if(error)
    return std::string(error);
  
  return std::string("No Error");
}

void* 
dnmi::file::DSO::operator[](const std::string &name)
{
  void *ret;
  const char *error;

  if(!handle)
    DSOException("No DSO file is open!");

  ret=dlsym(handle, name.c_str());
  error=dlerror();

  if(error!=0)
    throw DSOException(error);

  return ret;
}

