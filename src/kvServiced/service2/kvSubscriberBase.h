/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvSubscriberBase.h,v 1.1.2.3 2007/09/27 09:02:40 paule Exp $                                                       

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
#ifndef __kvSubscriberBase_h__
#define __kvSubscriberBase_h__

#include <dnmithread/mtcout.h>
#include <boost/thread/mutex.hpp>
#include <time.h>

class KvSubscriberBase{

  std::string subscriberid_;
  time_t      t1NoConnection;
  time_t      t2NoConnection;

 public:

  KvSubscriberBase():t1NoConnection(0),t2NoConnection(0){}
  KvSubscriberBase(const KvSubscriberBase &p)
    :subscriberid_(p.subscriberid_), t1NoConnection(p.t1NoConnection),
    t2NoConnection(p.t2NoConnection)
    {
    }

  virtual ~KvSubscriberBase(){}

  KvSubscriberBase& operator=(const KvSubscriberBase &rhs){
    if(this!=&rhs){
      t1NoConnection=rhs.t1NoConnection;
      t2NoConnection=rhs.t2NoConnection;
      subscriberid_=rhs.subscriberid_;
    }
      return *this;
    } 

  void subscriberid(const std::string &subscriberid){ 
    subscriberid_=subscriberid;
  }

  std::string subscriberid()const{ return subscriberid_;}

  /**
   * If we got a connection to the subscriber call this function with true.
   * Call it with false if we did'nt get a connection to the subscriber.
   *
   * @param connected true when we got a connection with the subscriber
   *        false otherwise.
   * @param removeNow true remove this subscriber with first chance. Now if
   *        posible.
   */
  void connection(bool connected, bool removeNow=false);
  bool removeThisSubscriber(int durationInSeconds);

  
};

#endif
