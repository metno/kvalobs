/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvQtEvents.h,v 1.4.6.1 2007/09/27 09:02:47 paule Exp $                                                       

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
#ifndef __kvQtEvents_h__
#define __kvQtEvents_h__

#include <qevent.h> 
#include "kvservicetypes.h"

#define KV_EVENTBASE        (QEvent::User+850) 
#define KV_DATA_EVENT       (KV_EVENTBASE)
#define KV_DATANOTIFY_EVENT (KV_EVENTBASE+1)
#define KV_HINT_EVENT       (KV_EVENTBASE+2)

namespace kvservice{
  namespace priv{
    class KvDataEvent : public QCustomEvent
      {
	KvObsDataListPtr data_;
	
      public:
	
	KvDataEvent(KvObsDataListPtr data)
	  :QCustomEvent(KV_DATA_EVENT), data_(data){}
	
	KvObsDataListPtr data(){ return data_;}
      };

    class KvDataNotifyEvent : public QCustomEvent
      {
	KvWhatListPtr what_;
	
      public:
	
	KvDataNotifyEvent(KvWhatListPtr what)
	  :QCustomEvent(KV_DATANOTIFY_EVENT), what_(what){}
	
	KvWhatListPtr what(){ return what_;}
      };

    class KvHintEvent : public QCustomEvent
      {
	bool upEvent_;
	
      public:
	
	KvHintEvent(bool upEvent)
	  :QCustomEvent(KV_HINT_EVENT), upEvent_(upEvent){}
	
	bool upEvent()const{ return upEvent_;}
	bool downEvent()const{ return !upEvent_;}
      };
  }
}



#endif
