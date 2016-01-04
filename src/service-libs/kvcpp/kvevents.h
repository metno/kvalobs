/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: kvevents.h,v 1.2.2.3 2007/09/27 09:02:45 paule Exp $                                                       

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
#ifndef __kvservice_kvevents_h__
#define __kvservice_kvevents_h__

#include <dnmithread/CommandQue.h>
#include "kvservicetypes.h"
#include "kveventinterface.h"

namespace kvservice {

class KvEventBase : public dnmi::thread::CommandBase {
 public:
  KvEventBase() {
  }
  ;
  virtual ~KvEventBase() {
  }
  ;

  virtual void dispatchEvent(KvEventInterface &eventInterface)=0;

  /**
   * from CommandBase. 
   * Do nothing!
   */
  bool executeImpl() {
    return true;
  }
  ;
};

class DataEvent : public KvEventBase {
  KvObsDataListPtr data_;
 public:
  DataEvent(KvObsDataListPtr data)
      : data_(data) {
  }

  KvObsDataListPtr data() {
    return data_;
  }

  void dispatchEvent(KvEventInterface &eventInterface);

};

class DataNotifyEvent : public KvEventBase {
  KvWhatListPtr what_;

 public:

  DataNotifyEvent(KvWhatListPtr what)
      : what_(what) {
  }

  KvWhatListPtr what() {
    return what_;
  }

  void dispatchEvent(KvEventInterface &eventInterface);

};

class HintEvent : public KvEventBase {
  bool upEvent_;

 public:

  HintEvent(bool upEvent)
      : upEvent_(upEvent) {
  }

  bool upEvent() const {
    return upEvent_;
  }
  bool downEvent() const {
    return !upEvent_;
  }

  void dispatchEvent(KvEventInterface &eventInterface);

};
}

#endif
