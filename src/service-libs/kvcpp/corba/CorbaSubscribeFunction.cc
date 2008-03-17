/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: CorbaSubscribeFunction.cc,v 1.2.2.2 2007/09/27 09:02:46 paule Exp $                                                       

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
class Subscribe
{
protected:

  CorbaKvApp *app;

  virtual CORBA::String_var doSubscribe()
  {
    CORBA::String_var ret = 
      service->subscribeDataNotify(*info.getDataSubscribeInfo(), refSub);
    if(strlen(static_cast<char*>(ret))==0){
      LOGERROR("WARNING: subscribeDataNotify returned a failindicator!\n");
	    
      getPoa()->deactivate_object(sub->getId());
      sub->_remove_ref();
      return string();
    }
    //omni_mutex_lock lock(mutex);
    IDataNotifyList it=dataNotifySubs.find(subid);
    if ( it != dataNotifySubs.end() ) {
      LOGERROR("DUPLICATE subid <Subscriber>!");
    }
    dataNotifySubs[subid] = ptr;
    return std::string(static_cast<char *>(ret));
  }

public:
  template<class Subscriber>
  std::string operator()( Subscriber *sub ) 
  {
    kvService_ptr     service; 
    bool              forceNS=false;
    bool              usedNS;
    int               sensor;
    
    try{
      PortableServer::ObjectId *id;
      id = getPoa()->activate_object( sub );
      sub->setId( id );
    }
    catch(...){
      LOGERROR("Cant activate <Subscriber>!");
      delete sub;
      return string();
    }
    
    try{
      refSub = sub->_this();
    }
    catch(...){
      LOGERROR("Cant obtain a referance to <Subscriber>!");
      getPoa()->deactivate_object(sub->getId());
      sub->_remove_ref();
      return string();
    }
    
    try{
      for( int i = 0; i < 2; i++ ) {
	service = lookUpManager(forceNS, usedNS);

	try {
	  return doSubscribe();
	}
	catch(CORBA::TRANSIENT &ex){
	  LOGINFO("WARNING: Exception CORBA::TRANSIENT!\n");
	}
	catch(CORBA::COMM_FAILURE &ex){
	  LOGINFO("WARNING: Exception CORBA::COMM_FAILURE!\n");
	}
	catch(...){
	  LOGINFO("WARNING: Exception unknown!\n");
	  getPoa()->deactivate_object(sub->getId());
	  sub->_remove_ref();
	  return string();
	}
	
	if(usedNS){
	  LOGINFO("WARNING: cant subscribe to <DataNotify>!\n");
	  getPoa()->deactivate_object(sub->getId());
	  sub->_remove_ref();
	  return string();
      }
	
	forceNS=true;
      }
  }
    catch(LookUpException &ex){
      LOGINFO("WARNING: KvQtCorbaApp::subscribeDataNotify: " << ex.what() << endl);
    }
    catch(...){
      LOGINFO("WARNING: KvQtCorbaApp::subscribeDataNotify: hmmm, very strange, a unkown exception!\n");
    }
    
    getPoa()->deactivate_object(sub->getId());
    sub->_remove_ref();
    return string();
  }
}



























struct SubscribeHolder {
  typedef DataNotifySubscriber           Subscriber;
  typedef kvDataNotifySubscriber_var     kvSubscriber_var;
  typedef DataNotifyList                 SubscriberList;
  typedef IDataNotifyList                ISubscriberList;
}

template<class SI>
string 
subscribeDataNotify( const KvDataSubscribeInfoHelper &info, 
		     dnmi::thread::CommandQue &que )
{
  // HUSK: legg til fra KvApp sin metode.
  
  SI::Subscriber       *sub;  
  SI::kvSubscriber_var refSub;  
  kvService_ptr     service; 
  CORBA::String_var ret;
  bool              forceNS=false;
  bool              usedNS;
  int               sensor;
      
  try{
    sub= new Subscriber(que);
  }
  catch(...){
    LOGERROR("NOMEM: cant allocate space for <Subscriber>!");
    return string();
  }
  
  try{
    PortableServer::ObjectId *id;
    id=getPoa()->activate_object(sub);
    sub->setId(id);
  }
  catch(...){
    LOGERROR("Cant activate <Subscriber>!");
    delete sub;
    return string();
  }
  
  try{
    refSub=sub->_this();
  }
  catch(...){
    LOGERROR("Cant obtain a referance to <Subscriber>!");
    getPoa()->deactivate_object(sub->getId());
    sub->_remove_ref();
    return string();
  }
  
  try{
    for(int i=0; i<2; i++){
      service=lookUpManager(forceNS, usedNS);
      
      try{
	ret=service->subscribeDataNotify(*info.getDataSubscribeInfo(), 
					 refSub);
	if(strlen(static_cast<char*>(ret))==0){
	  LOGERROR("WARNING: subscribeDataNotify returned a failindicator!\n");
	  
	  getPoa()->deactivate_object(sub->getId());
	  sub->_remove_ref();
	  return string();
	}
	
	//addDataNotifySubscriber(sub, string(static_cast<char *>(ret)));
	//omni_mutex_lock lock(mutex);
	IDataNotifyList it=dataNotifySubs.find(subid);
	if(it!=dataNotifySubs.end()){
	  LOGERROR("DUPLICATE subid <Subscriber>!");
	}
	dataNotifySubs[subid]=ptr;
	return std::string(static_cast<char *>(ret));
      }
      catch(CORBA::TRANSIENT &ex){
	LOGINFO("WARNING: Exception CORBA::TRANSIENT!\n");
      }
      catch(CORBA::COMM_FAILURE &ex){
	LOGINFO("WARNING: Exception CORBA::COMM_FAILURE!\n");
      }
      catch(...){
	LOGINFO("WARNING: Exception unknown!\n");
	getPoa()->deactivate_object(sub->getId());
	sub->_remove_ref();
	return string();
      }
      
      if(usedNS){
	LOGINFO("WARNING: cant subscribe to <DataNotify>!\n");
	getPoa()->deactivate_object(sub->getId());
	sub->_remove_ref();
	return string();
      }
      
      forceNS=true;
    }
  }
  catch(LookUpException &ex){
    LOGINFO("WARNING: KvQtCorbaApp::subscribeDataNotify: " << ex.what() << endl);
  }
  catch(...){
    LOGINFO("WARNING: KvQtCorbaApp::subscribeDataNotify: hmmm, very strange, a unkown exception!\n");
  }
  
  getPoa()->deactivate_object(sub->getId());
  sub->_remove_ref();
      return string();
}

string 
CorbaKvApp::subscribeData( const KvDataSubscribeInfoHelper &info, dnmi::thread::CommandQue &que )
{
  DataSubscriber       *sub;  
  kvDataSubscriber_var refSub;  
  kvService_ptr     service; 
  CORBA::String_var ret;
  bool              forceNS=false;
  bool              usedNS;
  int               sensor;
  
  try{
    sub= new DataSubscriber(que);
  }
  catch(...){
    LOGERROR("NOMEM: cant allocate space for <DataSubscriber>!");
    return string();
  }
  
  try{
    PortableServer::ObjectId  *id;
    
    if(CORBA::is_nil(getPoa())){
      LOGFATAL("getPoa()==NULL");
      return string();
    }
    
    id=getPoa()->activate_object(sub);
    sub->setId(id);
  }
  catch(...){
    LOGERROR("Cant activate <Subscriber>!");
    delete sub;
    return string();
  }
  
  try{
    refSub=sub->_this();
  }
  catch(...){
    LOGERROR("Cant obtain a referance to <Subscriber>!");
    getPoa()->deactivate_object(sub->getId());
    sub->_remove_ref();
    return string();
  }
  
  try{
    for(int i=0; i<2; i++){
      service=lookUpManager(forceNS, usedNS);
      
      try{
	ret=service->subscribeData(*info.getDataSubscribeInfo(), 
				   refSub);
	if(strlen(static_cast<char*>(ret))==0){
	  CERR("WARNING: subscribeData returned with a failindicator!\n");
	  getPoa()->deactivate_object(sub->getId());
	  sub->_remove_ref();
	  return string();
	}
	
	//addDataSubscriber(sub, string(static_cast<char *>(ret)));
	{
	  //omni_mutex_lock lock(mutex);
	  
	  IDataList it=dataSubs.find(subid);
	  
	  if(it!=dataSubs.end()){
	    LOGERROR("DUPLICATE subid <DataSubscriber>!");
	    return false;
	  }
	  
	  dataSubs[subid]=ptr;
	  
	  return true;
	  
	}
	
	
	return std::string(static_cast<char *>(ret));
      }
      catch(CORBA::TRANSIENT &ex){
	LOGINFO("EXCEPTION: CORBA::TRANSIENT!\n");
      }
      catch(CORBA::COMM_FAILURE &ex){
	LOGINFO("EXCEPTION: CORBA::COMM_FAILURE!\n");
      }
      catch(...){
	LOGINFO("EXCEPTION: Exception unknown!\n");
	getPoa()->deactivate_object(sub->getId());
	sub->_remove_ref();
	
	return string();
      }
      
      if(usedNS){
	LOGINFO("WARNING: cant subscribe <data>!\n");
	getPoa()->deactivate_object(sub->getId());
	sub->_remove_ref();
	return string();
      }
      
      forceNS=true;
    }
      }
  catch(LookUpException &ex){
    LOGINFO("LOOKUPEXCEPTION: " << ex.what() << endl);
  }
  catch(...){
    LOGINFO("UNKNOWNEXCEPTION: hmmm, very strange, an unkown exception!\n");
	
  }
  
  getPoa()->deactivate_object(sub->getId());
  sub->_remove_ref();
  
  return string();
}


    string 
    CorbaKvApp::subscribeKvHint( dnmi::thread::CommandQue &que )
    {
      HintSubscriber       *sub;  
      kvHintSubscriber_var refSub;  
      kvService_ptr     service; 
      CORBA::String_var ret;
      bool              forceNS=false;
      bool              usedNS;
      int               sensor;
      
      try{
	sub= new HintSubscriber(que);
      }
      catch(...){
	LOGERROR("NOMEM: cant allocate space for <HintSubscriber>!");
	return string();
      }
      
      try{
	PortableServer::ObjectId *id;
	id=getPoa()->activate_object(sub);
	sub->setId(id);
      }
      catch(...){
	LOGERROR("Cant activate <HineSubscriber>!");
	delete sub;
	return string();
      }
      
      try{
	refSub=sub->_this();
      }
      catch(...){
	LOGERROR("Cant obtain a referance to <HintSubscriber>!");
	getPoa()->deactivate_object(sub->getId());
	sub->_remove_ref();
	return string();
      }
      
      
      try{
	for(int i=0; i<2; i++){
	  service=lookUpManager(forceNS, usedNS);
	  
	  try{
	    ret=service->subscribeKvHint(refSub);
	    
	    if(strlen(static_cast<char*>(ret))==0){
	      LOGINFO("WARNING: subscribeKvHint returned with a failindicator!\n");
	      getPoa()->deactivate_object(sub->getId());
	      sub->_remove_ref();
	      return string();
	    }
	    
	    //addHintSubscriber(sub, string(static_cast<char *>(ret)));
    {
      //omni_mutex_lock lock(mutex);
      IHintList it=hintSubs.find(subid);
      
      if(it!=hintSubs.end()){
	LOGERROR("DUPLICATE subid <HintSubscriber>!");
	return false;
      }
      
      hintSubs[subid]=ptr;
      
      return true;
    }

	    
	    return std::string(static_cast<char*>(ret));
	  }
	  catch(CORBA::TRANSIENT &ex){
	    LOGINFO("EXCEPTION: CORBA::TRANSIENT!\n");
	  }
	  catch(CORBA::COMM_FAILURE &ex){
	    LOGINFO("EXCEPTION: CORBA::COMM_FAILURE!\n");
	  }
	  catch(...){
	    LOGINFO("EXCEPTION: unknown!\n");
	    getPoa()->deactivate_object(sub->getId());
	    sub->_remove_ref();
	    return string();
	  }
	  
	  if(usedNS){
	    LOGINFO("WARNING: cant subscribe <kvHint>!\n");
	    getPoa()->deactivate_object(sub->getId());
	    sub->_remove_ref();
	    return string();
	  }
	  
	  forceNS=true;
	}
      }
      catch(LookUpException &ex){
	LOGINFO("LOKUPEXCEPTION: " << ex.what() << endl);
      }
      catch(...){
	LOGINFO("EXCEPTION: unknown exception!\n");
      } 
      
      getPoa()->deactivate_object(sub->getId());
      sub->_remove_ref();
      return string();
    }
