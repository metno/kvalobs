/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: TimeCheck.h,v 1.1.6.1 2007/09/27 09:02:37 paule Exp $                                                       

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
#ifndef __timecheck_myBMblurb_h__
#define __timecheck_myBMblurb_h__

#include <time.h>

class  TimeCheck{
  time_t mtime_;
  time_t checkTime_;

 public:  
  TimeCheck()
    :mtime_(0), checkTime_(0){}
  
  TimeCheck(time_t mt, time_t  ct)
    :mtime_(mt), checkTime_(ct){}
      
  TimeCheck(const TimeCheck &t)
    :mtime_(t.mtime_), checkTime_(t.checkTime_){}
  
  TimeCheck& operator=(const TimeCheck &t){
    if(&t!=this){
	  mtime_=t.mtime_;
	  checkTime_=t.checkTime_;
    }
    return *this;
  }

  time_t mtime()const{ return mtime_;}
  void   mtime(time_t mt){ mtime_=mt;}

  time_t checkTime()const{ return checkTime_;}
  void   checkTime(time_t t){ checkTime_=t;}
};


#endif
