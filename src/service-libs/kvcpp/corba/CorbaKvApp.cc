/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: CorbaKvApp.cc,v 1.2.2.3 2007/09/27 09:02:46 paule Exp $                                                       

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
#include "CorbaKvApp.h"
#include "CorbaGetFunction.h"
#include <milog/milog.h>
#include <miconfparser/confparser.h>
#include <kvskel/kvService.hh>
#include <miconfparser/valelement.h>
#include <signal.h>

using namespace std;
using namespace miutil;
using namespace kvalobs;
using namespace kvservice;
using namespace CKvalObs::CDataSource;
using namespace CKvalObs::CService;
using namespace kvservice::corba::priv;
using namespace kvservice::priv;
using namespace miutil::conf;

namespace {
  volatile sig_atomic_t sigTerm=0;
  void sig_term(int);
  void setSigHandlers();
  bool terminateFunc();
}
			
namespace kvservice			
{
  namespace corba
  {
    CorbaKvApp::CorbaKvApp( int &argc, char **argv, 
			    miutil::conf::ConfSection *conf, 
			    const char *options[][2] )
      : CorbaHelper::CorbaApp( argc, argv, options )
      , corbaThread( NULL )
      , refService(kvService::_nil())
      , shutdown_( false )
    {
      CorbaGetFunction::corbaApp = this;
      corbaThread = new ::kvservice::priv::KvCorbaThread( argc, argv, conf, options );

      setSigHandlers();


      ValElementList valElem;
      string         val;

      valElem=conf->getValue("corba.nameserver");

      if(valElem.empty()){
	LOGFATAL("No nameserver <corba.nameserver> in the configurationfile!");
	exit(1);
      }

      nameserver=valElem[0].valAsString();

      if(nameserver.empty()){
	LOGFATAL("The key <corba.nameserver> in the configurationfile has no value!");
	exit(1);
      }
      
      setNameservice(nameserver);
      
      LOGINFO("Using corba nameserver at <" << nameserver << ">!");
      
      
      valElem=conf->getValue("corba.path");
      
      if(valElem.empty()){
	LOGFATAL("No path <corba.path> in the configurationfile!");
	exit(1);
      }
      
      nameserverpath=valElem[0].valAsString();
      
      if(nameserverpath.empty()){
	LOGFATAL("The key <corba.path> in the configurationfile has no value!");
	exit(1);
      }
      
      if(*nameserverpath.rbegin()!='/')
	nameserverpath.append("/");
      
      if(*nameserverpath.begin()!='/')
	nameserverpath.insert(0, "/");
      
      LOGINFO("Using kvalobs in path <" 
	      << nameserver << "::" << nameserverpath << ">!");

      while( not corbaThread->isInitialized() )
	sleep(1);
  
     dataInput = CKvalObs::CDataSource::Data::_nil();
    }

    
    CorbaKvApp::~CorbaKvApp( )
    {
      doShutdown();
    }

    conf::ConfSection* CorbaKvApp::readConf( const string &fname )
    {
      conf::ConfParser parser;
      conf::ConfSection *conf;
      ifstream fis;
      
      fis.open(fname.c_str());

      if ( !fis ) {
	LOGERROR("Cant open the configuration file <" << fname << ">!");
      } 
      else {
	LOGINFO("Reading configuration from file <" << fname << ">!");
	conf = parser.parse( fis );
	if ( !conf ) {
	  LOGERROR("Error while reading configuration file: <" << fname 
		   << ">!" << endl << parser.getError() );
	}
	else {
	  LOGINFO("Configuration file loaded!");
	  return conf;
	}
      }
      return 0;
    }

    std::string CorbaKvApp::kvpathInCorbaNameserver() const
    {
      string::size_type len = nameserverpath.size(); 
      return nameserverpath.substr(1, len -2 );
    }


    kvService_ptr CorbaKvApp::lookUpManager( bool forceNS, bool & usedNS )
    {
    	CORBA::Object_var                 obj;
    	CKvalObs::CService::kvService_ptr ptr;
    	std::string path(nameserverpath+"kvService");
    	usedNS=false;

    	Lock lock(mutex);
    
    	while(true){
    		if(forceNS){
    			usedNS=true;
	  
    			obj=getObjFromNS(path);
	  
    			if(CORBA::is_nil(obj))
    				throw LookUpException("EXCEPTION: Can't obtain a reference for 'kvService'\n           from the CORBA nameserver!");
	  
    			ptr=CKvalObs::CService::kvService::_narrow(obj);
	  
    			if(CORBA::is_nil(ptr))
    				throw LookUpException("EXCEPTION: Can't narrow reference for 'kvService'!");
	  
    			refService=CKvalObs::CService::kvService::_duplicate(ptr);
	  
    			return ptr;
    		}
	
    		if(CORBA::is_nil(refService))
    			forceNS=true;
    		else
    		  return CKvalObs::CService::kvService::_duplicate(refService);
    	}
    }

    bool CorbaKvApp::shutdown() const
    {
      // Lock lock(mutex);
      return shutdown_ || sigTerm;
    }

    void CorbaKvApp::doShutdown()
    {
      {
	Lock lock(mutex);
	if ( shutdown_ ) {
	  //LOGDEBUG1("The CORBA subsystem is allready shutdown!");
	  return;
	}        
	shutdown_=true;
	sigTerm=1;
      }

      LOGDEBUG( "Shutdown..." );

      //Unsubscribe from kvalobs
      unsubscribeAll();
      
      CorbaHelper::CorbaApp::getCorbaApp()->getOrb()->shutdown(true);
      corbaThread->join( 0 );
      //delete corbaThread_; //This cause a segmentation fault
      corbaThread = NULL;
      LOGDEBUG1("AFTER: join");
    }
    

    void CorbaKvApp::run()
    {
   	 while ( not shutdown() ) {
   		 work();
   		 sleep(1);
   	 }
    }

    bool 
    CorbaKvApp::
    getKvRejectDecode( const RejectDecodeInfo &decodeInfo, 
		       RejectDecodeIterator &it )
    {
      getKvRejectDecodeFunc func( decodeInfo, it );
      return func( "getKvRejectDecode" );
    }
    bool 
    CorbaKvApp::
    getKvParams( list<kvParam> &paramList )
    {
      getKvParamsFunc func( paramList );
      return func( "getKvParams" );
    }
    bool 
    CorbaKvApp::
    getKvStations( list<kvStation> &stationList )
    {
      getKvStationsFunc func( stationList );
      return func( "getKvStations" );
    }
    bool 
    CorbaKvApp::
    getKvModelData( list<kvModelData> &dataList,
		    const WhichDataHelper &wd )
    {
      getKvModelDataFunc func( dataList, wd );
      return func( "getKvModelData" );
    }
    bool 
    CorbaKvApp::
    getKvReferenceStations( int stationid, int paramid,
			    list<kvReferenceStation> &refList )
    {
      getKvReferenceStationsFunc func( stationid, paramid, refList );
      return func( "getKvReferenceStations" );
    }
    bool 
    CorbaKvApp::
    getKvTypes( list<kvTypes> &typeList )
    {
      getKvTypesFunc func( typeList );
      return func( "getKvTypes" );
    }
    bool 
    CorbaKvApp::
    getKvOperator( list<kvOperator> &operatorList )
    {
      getKvOperatorFunc func( operatorList );
      return func( "getKvOperator" );
    }
    bool
    CorbaKvApp::
    getKvStationParam( list<kvStationParam> &stParam,
		       int stationid, int paramid, int day )
    {
      getKvStationParamFunc func( stParam, stationid, paramid, day );
      return func( "getKvStationParam" );
    }
    bool 
    CorbaKvApp::
    getKvStationMetaData( std::list<kvalobs::kvStationMetadata> &stMeta, int stationid, const std::string & metadataName)
    {
        getKvStationMetaDataFunc func( stMeta, stationid, metadataName );
        return func( __func__ );
    }
    bool
    CorbaKvApp::
    getKvObsPgm( list<kvObsPgm> &obsPgm,
		 const list<long> &stationList, bool aUnion )
    {
      getKvObsPgmFunc func( obsPgm, stationList, aUnion );
      return func( "getKvObsPgm" );
    }

    bool
    CorbaKvApp::
    connectToKvInput( bool reConnect ) {

      if ( reConnect || CORBA::is_nil(dataInput) ) {

	LOGDEBUG("Looking up CORBA Data object");

	dataInput = CKvalObs::CDataSource::Data::
	  _narrow( this->getObjFromNS( nameserverpath + "kvinput" ) );

	if ( CORBA::is_nil(dataInput) )
	  return false;
      }
      return true;
    }

    const Result_var 
    CorbaKvApp::
    sendDataToKv( const char *data, const char *obsType )
    {
      milog::LogContext context("sendDataToKv");

      if ( KvApp::kvApp->shutdown() ) {
	const char *msg = "The CORBA subsystem is shutdown!!!";
	LOGERROR( msg );
	Result_var r( new Result );
	r->res = ERROR;
	r->message = msg;
	return r;
      }

      for ( int i = 0; i < 2; i++ ) {
	if ( ! connectToKvInput( (bool) i ) )
	  break;
	try {
	  CKvalObs::CDataSource::Result_var ret = dataInput->newData(data, obsType);
	  return ret;
	}
	catch(CORBA::TRANSIENT &ex){
	  LOGWARN("Exception CORBA::TRANSIENT!");
	}
	catch(CORBA::COMM_FAILURE &ex){
	  LOGWARN("Exception CORBA::COMM_FAILURE!");
	}
	catch(...){
	  LOGERROR("Exception unknown!");
	  if ( ! i ) {
	    CKvalObs::CDataSource::Result_var r(new CKvalObs::CDataSource::Result);
	    r->res = CKvalObs::CDataSource::ERROR;
	    r->message = "Unable to connect to Data Input Daemon.";
	    return r;
	  }
	}
      }
      LOGERROR("Cannot perform operation");
      CKvalObs::CDataSource::Result_var r(new CKvalObs::CDataSource::Result);
      r->res = CKvalObs::CDataSource::ERROR;
      r->message = "Unable to look up Data Input Daemon.";
      return r;
    }

    bool 
    CorbaKvApp::
    getKvData( KvGetDataReceiver &dataReceiver, const WhichDataHelper &wd )
    {
      getKvDataFunc func( dataReceiver, wd, terminateFunc );
      return func( "getKvData" );
    }


    bool 
    CorbaKvApp::
    getKvData( KvObsDataList &dataList, const WhichDataHelper &wd )
    {
      getKvDataFunc_deprecated func( dataList, wd, terminateFunc );
      return func( "getKvData(deprecatedversion)" );
    }


    bool                     
    CorbaKvApp::
    addDataNotifySubscriber(DataNotifySubscriber *ptr, 
			    const std::string &subid)
    {
      Lock lock(mutex);
      
      IDataNotifyList it=dataNotifySubs.find(subid);
      
      if(it!=dataNotifySubs.end()){
	LOGERROR("DUPLICATE subid <DataNotifySubscriber>!");
	return false;
      }
      
      dataNotifySubs[subid]=ptr;
      
      return true;
    }
    
    bool 
    CorbaKvApp::
    addDataSubscriber(DataSubscriber *ptr, 
		      const std::string    &subid)
    {
      Lock lock(mutex);
      
      IDataList it=dataSubs.find(subid);
      
      if(it!=dataSubs.end()){
	LOGERROR("DUPLICATE subid <DataSubscriber>!");
	return false;
      }
      
      dataSubs[subid]=ptr;
      
      return true;
      
    }
    
    bool 
    CorbaKvApp::
    addHintSubscriber(HintSubscriber *ptr,
		      const std::string &subid)
    {
      Lock lock(mutex);
      IHintList it=hintSubs.find(subid);
      
      if(it!=hintSubs.end()){
	LOGERROR("DUPLICATE subid <HintSubscriber>!");
	return false;
      }
      
      hintSubs[subid]=ptr;
      
      return true;
    }
    

    string 
    CorbaKvApp::subscribeDataNotify( const KvDataSubscribeInfoHelper &info, dnmi::thread::CommandQue &que )
    {
      // HUSK: legg til fra KvApp sin metode.

      DataNotifySubscriber       *sub;  
      kvDataNotifySubscriber_var refSub;  
      kvService_ptr     service; 
      CORBA::String_var ret;
      bool              forceNS=false;
      bool              usedNS;
      int               sensor;
      
      try{
	sub= new DataNotifySubscriber(que);
      }
      catch(...){
	LOGERROR("NOMEM: cant allocate space for <DataNotifySubscriber>!");
	return string();
      }
      
      try{
	PortableServer::ObjectId *id;
	id=getPoa()->activate_object(sub);
	sub->setId(id);
      }
      catch(...){
	LOGERROR("Cant activate <DataNotifySubscriber>!");
	delete sub;
	return string();
      }
      
      try{
	refSub=sub->_this();
      }
      catch(...){
	LOGERROR("Cant obtain a referance to <DataNotifySubscriber>!");
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
	    
	    addDataNotifySubscriber(sub, string(static_cast<char *>(ret)));
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
	LOGERROR("Cant activate <DataNotifySubscriber>!");
	delete sub;
	return string();
      }
      
      try{
	refSub=sub->_this();
      }
      catch(...){
	LOGERROR("Cant obtain a referance to <DataNotifySubscriber>!");
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
	    
	    addDataSubscriber(sub, string(static_cast<char *>(ret)));
	    
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
	    
	    addHintSubscriber(sub, string(static_cast<char *>(ret)));
	    
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

    template<class SubList>
    bool CorbaKvApp::unsubscribe( const string &id, SubList &subList )
    {
      typename SubList::iterator it = subList.find( id );
      if ( it != subList.end() ) {
	unsubscribe_( id );
	getPoa()->deactivate_object(it->second->getId());
	it->second->_remove_ref();
	Lock lock(mutex);
	subList.erase(it);
	return true;
      }
      return false;
    }

    void 
    CorbaKvApp::unsubscribe( const string &id )
    {
      cerr << "CorbaKvApp::unsubscribe( " << id << ");" << endl;
      if ( unsubscribe( id, dataNotifySubs ) )
	return;
      if ( unsubscribe( id, dataSubs ) )
	return;
      if ( unsubscribe( id, hintSubs ) )
	return;
    }

    template<class SubList>
    void CorbaKvApp::unsubscribeAll( SubList &subList )
    {
      for ( typename SubList::const_iterator it = subList.begin(); 
	    it != subList.end(); it++ ) {
	unsubscribe_(it->first);
	getPoa()->deactivate_object(it->second->getId());
	it->second->_remove_ref();
      }
      Lock lock(mutex);
      subList.clear();
    }

    void
    CorbaKvApp::unsubscribeAll()
    {
      cerr << "CorbaKvApp::unsubscribeAll( );" << endl;
      unsubscribeAll( dataNotifySubs );
      unsubscribeAll( dataSubs );
      unsubscribeAll( hintSubs );
    }

    void CorbaKvApp::unsubscribe_(const string &id)
    {
      kvService_ptr service; 
      SubscribeId ret;
      bool        forceNS=false;
      bool        usedNS;
      
      try {
	for(int i=0; i<2; i++){
	  service=lookUpManager(forceNS, usedNS);
	  try{
	    service->unsubscribe(id.c_str());
	    return;
	  }
	  catch(CORBA::TRANSIENT &ex){
	    LOGINFO("WARNING:(KvQtCorbaApp::unsubscribe) Exception CORBA::TRANSIENT!\n");
	  }
	  catch(CORBA::COMM_FAILURE &ex){
	    LOGINFO("WARNING:(KvQtCorbaApp::unsubscribe) Exception CORBA::COMM_FAILURE!\n");
	  }
	  catch(...){
	    LOGINFO("WARNING:(KvQtCorbaApp::unsubscribe) Exception unknown!\n");
	    return;
	  }
	  if(usedNS){
	    LOGINFO("WARNING: cant unsubscribe from kvManagerInput!\n");
	    return;
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
    }
  }
}

namespace{
  void sig_term(int)
  {
    sigTerm=1;
  }
  
  bool terminateFunc()
  {
    return sigTerm==1;
  }
  
  void setSigHandlers()
  {
    sigset_t         oldmask;
    struct sigaction act, oldact;
    
    act.sa_handler=sig_term;
    sigemptyset(&act.sa_mask);
    act.sa_flags=0;
    
    if(sigaction(SIGTERM, &act, &oldact)<0){
      LOGFATAL("Can't install signal handler for SIGTERM");
      exit(1);
    }
    
    act.sa_handler=sig_term;
    sigemptyset(&act.sa_mask);
    act.sa_flags=0;
    
    if(sigaction(SIGINT, &act, &oldact)<0){
      LOGFATAL("Can't install signal handler for SIGTERM");
      exit(1);
    }
  }
}
