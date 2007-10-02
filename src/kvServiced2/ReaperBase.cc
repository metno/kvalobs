/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: ReaperBase.cc,v 1.1.2.2 2007/09/27 09:02:22 paule Exp $                                                       

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
#include <corbahelper/corbaApp.h>
#include "ReaperBase.h"

ReaperBase::
ReaperBase()
  :lastAccess_(time(0)), running(false), active(true)
{
}


bool 
ReaperBase::
isActive()const
{
  boost::mutex::scoped_lock lock(const_cast<boost::mutex&>(mutex));
  return active;
}

 
void 
ReaperBase::
deactivate()
{
  boost::mutex::scoped_lock lock(const_cast<boost::mutex&>(mutex));
  
  if(active){
    CorbaHelper::CorbaApp::getCorbaApp()->getPoa()->deactivate_object(objId);
    _remove_ref();
    active=false;
  }
}


void
ReaperBase::
setRunning(bool running)
{
  boost::mutex::scoped_lock lock(const_cast<boost::mutex&>(mutex));
  time(&lastAccess_);
  running=running;
}


bool 
ReaperBase::
isRunning(time_t &lastAccess)const
{
  boost::mutex::scoped_lock lock(const_cast<boost::mutex&>(mutex));
  lastAccess=lastAccess_;
  return running;
}

