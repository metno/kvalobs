/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: ConnectionCache.cc,v 1.3.6.2 2007/09/27 09:02:18 paule Exp $                                                       

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
#include <milog/milog.h>
#include "ConnectionCache.h"

ConnectionCache::~ConnectionCache() {
  IConnections it = connection.begin();

  for (; it != connection.end(); it++)
    delete it->first;
}

void ConnectionCache::addConnection(dnmi::db::Connection* con) {
  Lock lck(m);
  bool isEmpty = (nFree == 0);

  IConnections it = connection.find(con);

  if (it == connection.end()) {
    connection[con] = true;

    it = connection.begin();
    nFree = 0;

    while (it != connection.end()) {
      if (it->second)
        nFree++;
      it++;
    }

    if (nFree > 0 && isEmpty)
      cond.notify_all();

    return;
  }

}

dnmi::db::Connection*
ConnectionCache::findFreeConnection() {
  Lock lk(m);

  if (nFree == 0) {
    while (nFree == 0)
      cond.wait(lk);
  }

  IConnections it = connection.begin();

  for (; it != connection.end(); it++) {
    if (it->second == true) {
      nFree--;
      it->second = false;
      return it->first;
    }
  }

  nFree = 0;

  return 0;
}

bool ConnectionCache::freeConnection(dnmi::db::Connection* con) {
  bool ret = false;
  Lock lock(m);

  IConnections it = connection.find(con);

  if (it != connection.end()) {
    it->second = true;
    ret = true;
    nFree++;
    cond.notify_all();
  }

  if (!ret)
    LOGINFO("freeConnection: Unknown connection!");

  return ret;
}
