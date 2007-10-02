/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: KvAppSimple.cc,v 1.5.2.2 2007/09/27 09:02:44 paule Exp $                                                       

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
#include "KvAppSimple.h"
#include "timer.h"

kvservice::KvAppSimple *kvservice::KvAppSimple::kvAppSimple =0;

kvservice::KvAppSimple::TimerWrap::
TimerWrap(Timer *t, const miutil::miTime &e, int  d, long id):
  timer_(t), expire_(e), delay_(d), timerid_(id)
{
}

kvservice::KvAppSimple::TimerWrap::
~TimerWrap()
{
  delete timer_;
}

void 
kvservice::KvAppSimple::TimerWrap::
add(Timer *timer, const miutil::miTime &time)
{
  expire_=time;
  timer_=timer;
}
      
void         
kvservice::KvAppSimple::TimerWrap::
expire(const miutil::miTime &time)
{
  expire_=time;
}

 
std::string 
kvservice::KvAppSimple::
subscribeDataNotify(const KvDataSubscribeInfoHelper &info)
{
  return app.subscribeDataNotify(info, que);
}
    

std::string 
kvservice::KvAppSimple::
subscribeData(const KvDataSubscribeInfoHelper &info)
{
  return app.subscribeData(info, que);
}
    

std::string 
kvservice::KvAppSimple::
subscribeKvHint()
{
  return app.subscribeKvHint(que);
}
    

void
kvservice::KvAppSimple::
unsubscribe(const std::string &subscriberid)
{
  app.unsubscribe(subscriberid);
}


kvservice::KvAppSimple::
KvAppSimple(int argn, char **argv,  miutil::conf::ConfSection *conf):
  app(argn, argv, conf), nextTimerid(0), shutdown_(false)
{
  kvAppSimple=this;
}

kvservice::KvAppSimple::
~KvAppSimple()
{
}

void 
kvservice::KvAppSimple::
doShutdown()
{
  shutdown_=true;
}

bool 
kvservice::KvAppSimple::
shutdown()
{
  return app.shutdown();
}


miutil::conf::ConfSection* 
kvservice::KvAppSimple::
readConf(const std::string &fname)
{
  return KvApp::readConf(fname);
}

PortableServer::POA_ptr 
kvservice::KvAppSimple::
getPoa()const
{
  return app.getPoa();
}
    
CORBA::ORB_ptr
kvservice::KvAppSimple::
getOrb()const
{
  return app.getOrb();
}

PortableServer::POAManager_ptr 
kvservice::KvAppSimple::
getPoaMgr()const
{
  return app.getPoaMgr();
}
    
bool  
kvservice::KvAppSimple::
putObjInNS(CORBA::Object_ptr objref, 
	   const std::string &name)
{
  return app.putObjInNS(objref, name);
}
    
CORBA::Object_ptr
kvservice::KvAppSimple::
getObjFromNS(const std::string &name)
{
  return app.getObjFromNS(name);
}
    
std::string 
kvservice::KvAppSimple::
corbaRef(CORBA::Object_ptr ptr)
{
  return app.corbaRef(ptr);
}
    

CORBA::Object_ptr 
kvservice::KvAppSimple::
corbaRef(const std::string &ref)
{
  return app.corbaRef(ref);
}
    
    
std::string  
kvservice::KvAppSimple::
corbanameserver()const
{
  return app.corbanameserver();
}
    
 
std::string 
kvservice::KvAppSimple::
kvpathInCorbaNameserver()const
{
  return app.kvpathInCorbaNameserver();
}
    
bool 
kvservice::KvAppSimple::
getKvData(KvGetDataReceiver &dataReceiver,
	  const WhichDataHelper &wd)
{
  return app.getKvData(dataReceiver, wd);
}


bool 
kvservice::KvAppSimple::
getKvParams(std::list<kvalobs::kvParam> &paramList)
{
  return app.getKvParams(paramList);
}

bool 
kvservice::KvAppSimple::
getKvStations(std::list<kvalobs::kvStation> &stationList)
{
  return app.getKvStations(stationList);
}

bool 
kvservice::KvAppSimple::
getKvModelData(std::list<kvalobs::kvModelData> &dataList,
	       const WhichDataHelper &wd)
{
  return getKvModelData(dataList,wd);
}

bool
kvservice::KvAppSimple::
getKvReferenceStations(int stationid, 
		       int paramid, 
		       std::list<kvalobs::kvReferenceStation> &refList)
{
  return app.getKvReferenceStations(stationid, paramid, refList);
}

bool 
kvservice::KvAppSimple::
getKvTypes(std::list<kvalobs::kvTypes> &typeList)
{
  return app.getKvTypes(typeList);
}
    
bool 
kvservice::KvAppSimple::
getKvObsPgm(std::list<kvalobs::kvObsPgm> &obsPgm,
	    const std::list<long> &stationList,
	    bool aUnion)
{
  return app.getKvObsPgm(obsPgm,stationList, aUnion);
}
    
bool 
kvservice::KvAppSimple::
getKvOperator(std::list<kvalobs::kvOperator> &operatorList)
{
  return app.getKvOperator(operatorList);
}

const CKvalObs::CDataSource::Result_var
kvservice::KvAppSimple::
sendDataToKv(const char *data, const char *obsType)
{
  return app.sendDataToKv(data, obsType);
}



void 
kvservice::KvAppSimple::
onKvHintEvent(bool up)
{
}

void
kvservice::KvAppSimple::
onKvDataNotifyEvent(KvWhatListPtr what)
{
}

void 
kvservice::KvAppSimple::
onKvDataEvent(KvObsDataListPtr data)
{
}

bool 
kvservice::KvAppSimple::
onStartup()
{
  return true;
}

void 
kvservice::KvAppSimple::
onShutdown()
{
}


long 
kvservice::KvAppSimple::
addTimer(Timer *timer, int periodInSec)
{
  TimerWrap *wrap;
  miutil::miTime tNow(time(NULL));
  
  if(!timer || periodInSec<0)
    return -1;

  tNow.nowTime();
  
  tNow.addSec(periodInSec);

  try{
    wrap=new TimerWrap(timer, tNow, periodInSec, nextTimerid);
    nextTimerid++;
  }
  catch(...){
    return -1;
  }

  insertTimerWrap(wrap);
  
  return wrap->timerid();

}

long 
kvservice::KvAppSimple::
addTimeJob(Timer *timer, const miutil::miTime &time)
{
  TimerWrap *wrap;

  if(time.undef() || !timer)
    return -1;

  try{
    wrap=new TimerWrap(timer, time, -1, nextTimerid);
    nextTimerid++;
  }
  catch(...){
    return -1;
  }

  insertTimerWrap(wrap);
  
  return wrap->timerid();
}

void 
kvservice::KvAppSimple::
insertTimerWrap(TimerWrap *wrap)
{
  std::list<TimerWrap*>::iterator it=timerList.begin();

  for(; it!=timerList.end() && wrap->expire()>(*it)->expire(); it++);

  timerList.insert(it, wrap);
}


void 
kvservice::KvAppSimple::
removeTimer(long timerid)
{
  std::list<TimerWrap*>::iterator it=timerList.begin();
    
  for(; it!=timerList.end(); it++){
    if((*it)->timerid()==timerid){
      delete *it;
      timerList.erase(it);
      return;
    }
  }
}
    

void 
kvservice::KvAppSimple::
run()
{
  const int MAX_WAIT=10;
  dnmi::thread::CommandBase *cmd;
  int   waited;

  if(!onStartup())
    app.doShutdown();

  while(!app.shutdown()){
    cmd=que.get(1);
    
    if(!cmd || waited>MAX_WAIT){
      waited=0;
      
      if(!timerList.empty()){
	miutil::miTime tNow(time(NULL));
	
	std::list<TimerWrap*>::iterator it=timerList.begin();
      
	if((*it)->expire()<=tNow){
	  TimerWrap *wrap=*it;
	  Timer *t=wrap->timer();
	  
	  t->timeWrap=wrap;
	  
	  timerList.erase(it);
	  
	  if(!wrap->periodic()){
	    wrap->timer(0);
	    t->exec();
	    
	    //Has the timer requested re inserting in the list.
	    if(wrap->timer()){
	      insertTimerWrap(wrap);
	    }else{
	      //Reinsert timer in wrap so the destructor deletes it.
	      wrap->timer(t);
	      delete wrap;
	    }
	  }else{ //a periodic timer
	    t->exec();
	    
	    //Check if the timer has requested removing of this timer object
	    if(!wrap->timer()){
	      //Reinsert timer in wrap so the destructor deletes it.
	      wrap->timer(t);
	      delete wrap;
	    }else{
	      miutil::miTime newTime(wrap->expire());
	      newTime.addSec(wrap->delay());
	      wrap->expire(newTime);
	      insertTimerWrap(wrap);
	    }
	  }
	}
      }
      
      if(!cmd)
	continue;
    }
  
    waited++;


    KvEventBase *event=dynamic_cast<KvEventBase*>(cmd);
    
    if(event){
      event->dispatchEvent(*this);
    }
         
    delete cmd;
    cmd=0;

    if(shutdown_)
      app.doShutdown();
  }

  onShutdown();
}

