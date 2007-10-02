/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: ObjReaper.cc,v 1.1.6.2 2007/09/27 09:02:39 paule Exp $                                                       

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

#include <time.h>
#include "ObjReaper.h"

ObjReaper::
ObjReaper(ServiceApp &app_)
  :app(app_)
{
}


/**
 * ObjReaper run in a thread and collects ReaperBase objects
 * that is deactivated or has timed out. If a object has timed out
 * it means that it has not been used called upon in a TIMEOUT time. This 
 * indicates that the client either crached or has forgotten to relase
 * the object. At the momment this logic is only used by the iterartor
 * interfaces.
 */
void 
ObjReaper::
operator()()
{
  const int NEXT_TIME=60; 
  time_t now;
  time_t next;

  time(&next);
  
  while(!app.shutdown()){
    time(&now);
    
    if(next>now){
      sleep(1);
      continue;
    }
    
    next+=NEXT_TIME;
    app.cleanUpReaperObj();
  
  }
}


