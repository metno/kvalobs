/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: kvSubscriberBase.cc,v 1.2.6.1 2007/09/27 09:02:39 paule Exp $                                                       

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
#include "kvSubscriberBase.h"

void KvSubscriberBase::connection(bool connected, bool now) {
  boost::mutex::scoped_lock lock(mutex);

  if (connected) {
    t1NoConnection = 0;
    return;
  }

  if (now) {
    t1NoConnection = -1;
    t2NoConnection = -1;
    return;
  }

  if (t1NoConnection == 0)
    time(&t1NoConnection);
  else
    time(&t2NoConnection);
}

bool KvSubscriberBase::removeThisSubscriber(int durationInSeconds) {
  int dif = 0;
  boost::mutex::scoped_lock lock(mutex);

  if (t1NoConnection < 0 && t2NoConnection < 0)
    return true;

  if (t1NoConnection != 0 && t2NoConnection != 0) {
    dif = t2NoConnection - t1NoConnection;

    if (dif < 0)
      dif *= -1;

    if (dif >= durationInSeconds)
      return true;
  }

  return false;
}

