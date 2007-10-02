/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: timer.h,v 1.1.6.2 2007/09/27 09:02:44 paule Exp $                                                       

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
#ifndef __kvservice_timer_h__
#define __kvservice_timer_h__

#include <puTools/miTime>
#include "KvAppSimple.h"

namespace kvservice{

  class Timer{

    friend class KvAppSimple;
    KvAppSimple::TimerWrap *timeWrap;

  public:
    Timer(){};
    virtual ~Timer(){};

    /**
     * remove the Timer if it is a periodic timer.
     * Does nothing if it is not.
     */
    void remove();

    /**
     * If this is a one time Timer, ie. not periodic, put it back on the list
     * list of one Time jobs.
     */
    void add(const miutil::miTime &time);

    bool isPeriodic();
    
    virtual void exec()=0;
  };

}

#endif
