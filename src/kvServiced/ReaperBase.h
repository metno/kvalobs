/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: ReaperBase.h,v 1.1.2.3 2007/09/27 09:02:39 paule Exp $                                                       

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
#ifndef __ReaperBase_h__
#define __ReaperBase_h__

#include <omniORB4/CORBA.h>
#include <time.h>
#include <boost/thread/mutex.hpp>

class ReaperBase {
  time_t lastAccess_;
  bool running;
  bool active;
  PortableServer::ObjectId_var objId;
  bool timedout;
  int timeToLive_;
  boost::mutex mutex;

 public:
  ReaperBase();
  virtual ~ReaperBase() {
  }

  //This function return true if deactivate is called.
  //It means that the object is no longer active an can be 
  //garbage collected.
  bool isActive() const;

  //Set the object to deactive state.
  void deactivate();

  //Set the running state!
  void setRunning(bool running, bool &isActive);

  //Is running returns true if the service is
  //running right now.
  bool isRunning(time_t &lastAccess) const;

  void setObjId(PortableServer::ObjectId *id) {
    objId = id;
  }

  bool isTimedout() const {
    return timedout;
  }
  void setTimedout() {
    timedout = true;
  }

  void setTimeToLive(int sec);
  int getTimeToLive() const;

  //Cleanup the resources this object is using.
  virtual void cleanUp()=0;
  virtual void addRef()=0;
  virtual void removeRef()=0;

};

class IsRunningHelper {
  ReaperBase &rb;

 public:
  IsRunningHelper(ReaperBase &rb_, bool &isActive)
      : rb(rb_) {
    rb.setRunning(true, isActive);
  }

  ~IsRunningHelper() {
    bool dummy;
    rb.setRunning(false, dummy);
  }
};

#endif
