/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvSubscriberCollection.cc,v 1.2.6.3 2007/09/27 09:02:39 paule Exp $                                                       

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
#include <dnmithread/mtcout.h>
#include <time.h>
#include <sstream>
#include "kvSubscriberCollection.h"
#include <milog/milog.h>
#include <string>
#include <corbahelper/corbaApp.h>
#include <stdio.h>
#include <iomanip>
#include <boost/lexical_cast.hpp>
#include <unistd.h>
#include <fileutil/dir.h>

using namespace std;
using namespace miutil;

KvSubscriberCollection::KvSubscriberCollection()
{
    char *env=getenv("KVALOBS");

    if(!env){
	subPath="./";
    }else{
	subPath=env;
	
	if(!subPath.empty() && subPath[subPath.length()-1]!='/')
	    subPath+="/";

	subPath+="var/kvalobs/service/subscribers/";
    }
    
    readAllSubscribersFromFile();
	
}

KvSubscriberCollection::KvSubscriberCollection(const std::string &fname)
{
    subPath=fname;

    if(!subPath.empty() && subPath[subPath.length()-1]!='/')
	subPath+="/";

    readAllSubscribersFromFile();
}

KvSubscriberCollection::~KvSubscriberCollection()
{
  LOGDEBUG("KvSubscriberCollection::deleted!");
}


bool
KvSubscriberCollection::createSubscriberid(
    KvSubscriberBasePtr p,
    const std::string &servicename
    )
{
  int                COUNT_MAX=1000;
  int                count=0;
  std::ostringstream ost;
  std::string        subid;
  time_t             t;
  char               tBuf[30];
  struct tm          tTm;
  std::set<KvSubscriberBasePtr, KvSubscriberBasePtrOps>::iterator it;
  
  // The chema used to create a uniqe subscriberid:
  // 'kvalobs_service_servicename_datotid_helper'
  // helper is used too garanti uniqnes if datotid and servicename 
  // is equal. 

  if(time(&t)<0){
    LOGERROR("createSubscriberid: time() failed!");
    return false;
  }

  if(!gmtime_r( &t, &tTm)){
    LOGERROR("createSubscriberid: gmtime_r() failed!");
    return false;
  }
    
  sprintf(tBuf, "%04d%02d%02dT%02d%02d%02d", 
	  tTm.tm_year+1900, tTm.tm_mon+1, tTm.tm_mday,
	  tTm.tm_hour, tTm.tm_min, tTm.tm_sec);
  
  for(count=0; 
      count<COUNT_MAX && subid.empty(); 
      count++){
    ost.str("");   //reset ost.
    ost << "kvalobs_service_" << servicename << "_" << tBuf << "_" << count;
    
    subid=ost.str();

    for(it=subscribers_.begin(); it!=subscribers_.end(); it++){
      if((*it)->subscriberid()==subid)
	break;
    }

    if(it!=subscribers_.end())
      subid.erase();
  }
  
  if(subid.empty()){
    LOGERROR("createSubscriberid: cant create subscriberid!\n");
    return false;
  }

  p->subscriberid(subid);
  subscribers_.insert(p);

  LOGDEBUG("#subscribers_: " << subscribers_.size());

  return true;
}



void 
KvSubscriberCollection::removeSubscriberid(KvSubscriberBasePtr p)
{
  std::set<KvSubscriberBasePtr, KvSubscriberBasePtrOps>::iterator it;
  
  it=subscribers_.find(p);

  if(it!=subscribers_.end()){
    subscribers_.erase(it);
    LOGDEBUG("KvSubscriberCollection::removeSubscriberid: " 
	     << p->subscriberid()
	     << endl);
    removeSubscriberFile(p->subscriberid());

    return;
  }

  LOGDEBUG("KvSubscriberCollection::removeSubscriberid: " 
	   << p->subscriberid()
	   << " is not among the subscribers!");
}

void 
KvSubscriberCollection::removeSubscriberid(const std::string &subscriberid)
{
  std::set<KvSubscriberBasePtr, KvSubscriberBasePtrOps>::iterator it;

  LOGDEBUG("removeSubscriberid(" << subscriberid <<").....!\n");

  for(it=subscribers_.begin(); 
      it!=subscribers_.end(); 
      it++){

      if((*it)->subscriberid()==subscriberid){
	  LOGDEBUG("KvSubscriberCollection::removeSubscriberid: " 
		   << (*it)->subscriberid()
		   << endl);
	  subscribers_.erase(it);
	  removeSubscriberFile(subscriberid);
	  return;
      }
  }
  
  LOGDEBUG("KvSubscriberCollection::removeSubscriberid: " 
	   << subscriberid
	   << " is not among the subscribers. (FATAL?)");
	   
}




bool
KvSubscriberCollection::addDataNotifySubscriber(
			  KvDataNotifySubscriberPtr p
			  )
{
  boost::mutex::scoped_lock lock(mutex);
  

  if(!createSubscriberid(p, "datanotify"))
    return false;

  allStationsDataNotifySubscribers.push_back(p);
  writeSubscriberFile(p->subscriberid());

  return true;
}

bool
KvSubscriberCollection::addDataNotifySubscriber(
                                KvDataNotifySubscriberPtr p, 
				long                      stationid 
				)
{
  boost::mutex::scoped_lock lock(mutex);

  std::string subscriberid;
  bool subscriberidCreated=false;

  if(p->subscriberid().empty()){
    if(!createSubscriberid(p, "datanotify"))
      return false;

    subscriberidCreated=true;
  }

  try{
    stationDataNotifySubscribers.insert(make_pair(stationid, p));
  }
  catch(...){
    if(subscriberidCreated)
      removeSubscriberid(p);

    return false;
   }

   writeSubscriberFile(p->subscriberid());
   return true;
}




void 
KvSubscriberCollection::removeDataNotifySubscriber(const std::string &subscriberid)
{
  list<KvDataNotifySubscriberPtr>::iterator it;

  LOGDEBUG("removeDataNotifySubscriberid(" <<subscriberid <<").....!\n");

  it=allStationsDataNotifySubscribers.begin(); 

  while(it!=allStationsDataNotifySubscribers.end()){
    if((*it)->subscriberid()==subscriberid){
      allStationsDataNotifySubscribers.erase(it);
      it=allStationsDataNotifySubscribers.begin(); 
    }else
	it++;
  }

  multimap<long, KvDataNotifySubscriberPtr>::iterator it1,ittmp;
  
  it1=stationDataNotifySubscribers.begin();

  while(it1!=stationDataNotifySubscribers.end()){
    ittmp=it1;
    it1++;

    if(ittmp->second->subscriberid()==subscriberid){
      stationDataNotifySubscribers.erase(ittmp);
    }
  }
}

void 
KvSubscriberCollection::removeHintSubscriber(const std::string &subscriberid)
{
    std::list<KvHintSubscriberPtr>::iterator it;
    
    it=hintSubscriberList.begin();
    
    for(;it!=hintSubscriberList.end(); it++){
	if((*it)->subscriberid()==subscriberid){
	    hintSubscriberList.erase(it);
	    return;
	}
    }
}


void 
KvSubscriberCollection::removeDataSubscriber(const std::string &subscriberid)
{
  list<KvDataSubscriberPtr>::iterator it;

  LOGDEBUG("removeDataSubscriberid(" <<subscriberid <<").....!\n");

  it=allStationsDataSubscribers.begin(); 

  while(it!=allStationsDataSubscribers.end()){
    if((*it)->subscriberid()==subscriberid){
      allStationsDataSubscribers.erase(it);
      it=allStationsDataSubscribers.begin(); 
    }else
	it++;
  }

  multimap<long, KvDataSubscriberPtr>::iterator it1,ittmp;
  
  it1=stationDataSubscribers.begin();

  while(it1!=stationDataSubscribers.end()){
    ittmp=it1;
    it1++;

    if(ittmp->second->subscriberid()==subscriberid){
      stationDataSubscribers.erase(ittmp);
    }
  }

}



void 
KvSubscriberCollection::removeSubscriber(const std::string &subscriberid)
{
  boost::mutex::scoped_lock lock(mutex);

  removeDataNotifySubscriber(subscriberid);
  removeDataSubscriber(subscriberid);
  removeHintSubscriber(subscriberid);
  removeSubscriberid(subscriberid);
  removeSubscriberFile(subscriberid);
}

/*
 * Walk through all the subscribers lists and find all subscribers that
 * we cant connect to and remove them from our list of subscribers.
 */
void 
KvSubscriberCollection::
removeDeadSubscribers(int durationInSeconds)
{
  std::list<std::string> subList;

  {
    boost::mutex::scoped_lock lock(mutex);
    
    for(std::list<KvDataNotifySubscriberPtr>::iterator 
	  it=allStationsDataNotifySubscribers.begin();
	it!=allStationsDataNotifySubscribers.end();
	it++){
      if((*it)->removeThisSubscriber(durationInSeconds)){
	subList.push_back((*it)->subscriberid());
      }
    }
    
    
    for(std::multimap<long, KvDataNotifySubscriberPtr>::iterator 
	  it=stationDataNotifySubscribers.begin();
	it!=stationDataNotifySubscribers.end();
	it++){
      if(it->second->removeThisSubscriber(durationInSeconds)){
	subList.push_back(it->second->subscriberid());
      }
    }
    
    for(std::list<KvDataSubscriberPtr>::iterator 
	  it=allStationsDataSubscribers.begin();
	it!=allStationsDataSubscribers.end();
	it++){
      if((*it)->removeThisSubscriber(durationInSeconds)){
	subList.push_back((*it)->subscriberid());
      }
    }
  
    for(std::multimap<long, KvDataSubscriberPtr>::iterator 
	  it=stationDataSubscribers.begin();
	it!=stationDataSubscribers.end();
	it++){
      if(it->second->removeThisSubscriber(durationInSeconds)){
	subList.push_back(it->second->subscriberid());
      }
    }
  }
  
  for(std::list<std::string>::iterator it=subList.begin();
      it!=subList.end();
      it++){
    removeSubscriber(*it);
  }
}


void 
KvSubscriberCollection::forAllDataNotifySubscribers(
			   DataNotifySubscriberFuncBase &obj,
			   long                         stationid
			   )
{
  list<KvDataNotifySubscriberPtr>::iterator it;
  
  boost::mutex::scoped_lock lock(mutex);

  it=allStationsDataNotifySubscribers.begin(); 

  for(;it!=allStationsDataNotifySubscribers.end(); it++){
    obj.func(*it);
  }

  multimap<long, KvDataNotifySubscriberPtr>::iterator itu, itl;
   
  itu=stationDataNotifySubscribers.lower_bound(stationid);

  if(itu!=stationDataNotifySubscribers.end()){
     itl=stationDataNotifySubscribers.upper_bound(stationid);
     
     for(;itu!=itl; itu++){
       obj.func(itu->second);
     }
  }
}







bool 
KvSubscriberCollection::addDataSubscriber(KvDataSubscriberPtr p)
{
  boost::mutex::scoped_lock lock(mutex);
  
  if(!createSubscriberid(p, "data"))
    return false;

  allStationsDataSubscribers.push_back(p);

  writeSubscriberFile(p->subscriberid());
  return true;

}


bool 
KvSubscriberCollection::addDataSubscriber(KvDataSubscriberPtr p, 
					  long stationid)
{
  boost::mutex::scoped_lock lock(mutex);

  multimap<long, KvDataSubscriberPtr>::iterator itu, itl;
  bool subscriberidCreated=false;

  if(p->subscriberid().empty()){
    if(!createSubscriberid(p, "data"))
      return false;

    subscriberidCreated=true;
  }

  try{
      stationDataSubscribers.insert(make_pair(stationid, p));
  }
  catch(...){
    if(subscriberidCreated)
      removeSubscriberid(p);
    
    return false;
   }

  writeSubscriberFile(p->subscriberid());
   return true;
}

bool  
KvSubscriberCollection::addKvHintSubscriber(KvHintSubscriberPtr sub )
{
    using namespace CKvalObs::CService;

    if(!createSubscriberid(sub, "kvHint"))
	return false;
    
    hintSubscriberList.push_back(sub);
    writeSubscriberFile(sub->subscriberid());

    return true;
}


  
void 
KvSubscriberCollection::sendKvHintUp()
{
    using namespace CKvalObs::CService;

    std::list<KvHintSubscriberPtr>::iterator it;
    kvHintSubscriber_var sub;
    it=hintSubscriberList.begin();
    
    while(it!=hintSubscriberList.end()){
	try{
	    sub=(*it)->subscriber();
	    sub->kvUp();
	    it++;
	}
	catch(...){
	  std::list<KvHintSubscriberPtr>::iterator itTmp=it;
	  it++;
	  LOGINFO("SUBSCRIBER NOT LISTNING: Deletes the subscriber <" 
		  << (*itTmp)->subscriberid() << "> from the list of\n" <<
		  "kvHint subscribers.");
	  removeSubscriber((*itTmp)->subscriberid());
	}
    }
}

void 
KvSubscriberCollection::sendKvHintDown()
{
    using namespace CKvalObs::CService; 
    kvHintSubscriber_var sub;
    std::list<KvHintSubscriberPtr>::iterator it;
    
    it=hintSubscriberList.begin();
    
    for(;it!=hintSubscriberList.end(); it++){
	try{
	    sub=(*it)->subscriber();
	    sub->kvDown();
	}
	catch(...){
	  LOGINFO("SUBSCRIBER NOT LISTNING: Deletes the subscriber <" 
		  << (*it)->subscriberid() << "> from the list of\n" <<
		  "kvHint subscribers.");
	  removeSubscriber((*it)->subscriberid());
	}
    }

}
  

void 
KvSubscriberCollection::forAllDataSubscribers(DataSubscriberFuncBase &obj, 
					      long stationid)
{
  list<KvDataSubscriberPtr>::iterator it;
  
  boost::mutex::scoped_lock lock(mutex);

  it=allStationsDataSubscribers.begin(); 

  for(;it!=allStationsDataSubscribers.end(); it++){
    obj.func(*it);
  }

  multimap<long, KvDataSubscriberPtr>::iterator itu, itl;
   
  itl=stationDataSubscribers.lower_bound(stationid);

  if(itl!=stationDataSubscribers.end()){
     itu=stationDataSubscribers.upper_bound(stationid);
     
     for(;itl!=itu; itl++){
       obj.func(itl->second);
     }
  }

}



bool 
KvSubscriberCollection::hasDataSubscribers()const
{
  if(allStationsDataSubscribers.empty() &&
     stationDataSubscribers.empty())
    return false;

  return true;
}
bool 
KvSubscriberCollection::hasDataNotifySubscribers()const
{
  if(allStationsDataNotifySubscribers.empty() &&
     stationDataNotifySubscribers.empty())
    return false;

  return true;
}


std::ofstream*
KvSubscriberCollection::writeFileHeader(
    const std::string &subscriberid, 
    const kvalobs::KvDataSubscriberInfo *si,
    const std::string &corbaref)
{
    ofstream *fs;
    string path(subPath+subscriberid+".sub");
    miTime tNow(miTime::nowTime());

    try{
	fs=new ofstream();  
	fs->open(path.c_str(), ios_base::out|ios_base::trunc); 
	
	if(!fs->is_open()){
	    LOGERROR("Cant create subscriber info file <" << path << ">!");
	    delete fs;
	    return 0;
	}else{
	    LOGINFO("Writing subscriber <" << subscriberid << "> " << endl
		    <<"to file <" << path << ">!");
	}
    }catch(...){
	LOGERROR("OUT OF MEM: cant create an instance of ofstream!");
   	return 0;
    }

    /*
     * OBS, OBS
     * The 'Last Call: ' line must match the 'Last Call: ' line in the function
     * updateSubscriberFile. 
     */

    (*fs) << "Last call: " 
	  << left << setw(30) << setfill('#') << tNow.isoTime() << endl;

    (*fs) << "Created: "  << tNow.isoTime() << endl
	  << "Subid: " << subscriberid << endl
	  << "CORBA ref: " << corbaref << endl
	  << "StatusId: ";


    if(si){
	switch(si->status()){
	case CKvalObs::CService::All:        (*fs) << "All";        break;
	case CKvalObs::CService::OnlyFailed: (*fs) << "OnlyFailed"; break;
	case CKvalObs::CService::OnlyOk:     (*fs) << "OnlyOk";     break;
	}
    }
    
    
    (*fs) << endl;
    (*fs) << "QcId:";
    
    if(si){
	if(!si->qcAll()){
	    for(int i=0; i<si->qc().length(); i++){
		switch(si->qc()[i]){
		case CKvalObs::CService::QC1:  (*fs) << " QC1";  break;
		case CKvalObs::CService::QC2d: (*fs) << " QC2d"; break;
		case CKvalObs::CService::QC2m: (*fs) << " QC2m"; break;
		case CKvalObs::CService::HQC:  (*fs) << " HQC";  break;
		}
	    }
	}
    }
    
    (*fs) << endl;
    (*fs) << "Stations:";
       
    return fs;
}

bool 
KvSubscriberCollection::updateSubscriberFile(
    const std::string &subscriberid,
    const miutil::miTime &timeForLastCall
    )
{
    string path(subPath+subscriberid+".sub");
    fstream fs;

    fs.open(path.c_str(), ios_base::out|ios_base::in); 

    if(!fs.is_open()){
	LOGERROR("Can't open file: <" << path <<">");
	return false;
    }
 
    /*
     * OBS, OBS, OBS, OBS, OBS
     * The 'Last Call: ' line must match the 'Last Call: ' line in the function
     * writeFileHeader. 
     */
    fs << "Last call: " 
	<< left << setw(30) << setfill('#') << timeForLastCall.isoTime() 
	<< endl;

    fs.close();

    return true;
}


void
KvSubscriberCollection::readAllSubscribersFromFile()
{
    dnmi::file::Dir dir;
    string          file;
    

    if(!dir.open(subPath,"*.sub")){
      LOGERROR("DIR OPEN ERROR: Cant open directory <" << subPath << ">. " << dir.getErr());
      return;
    }

    try{
      while(dir.hasNext()){
	file=dir.next();
	
	//I dont check the return value from readSubscriberFromFile
 	//because the errors are reported in the function. And it doesn't
	//matter for the controll flow here.
	readSubscriberFile(subPath+file);
      }
    }
    catch(dnmi::file::DirException &ex) {
      LOGERROR("SUBSCRIBER READ DIR ERROR: cant read the subscribers in the\n"
	       << "directory <" << subPath << ">. " << ex.what());
    }
    catch(...){
      LOGERROR("UNEXPECTED EXCEPTION: While reading the subscribers in the\n"
	       << "directory <" << subPath << ">");
    }
}


bool 
KvSubscriberCollection::readSubscriberFile(const std::string &fname)
{
    miString::size_type i;
    miString  buf;
    miString key;
    miString val;
    ifstream fs;
    miTime   lastCall;
    miTime   created;
    miString subid;
    miString cref;
    miString statusid;
    vector<miutil::miString> qcIdList;
    vector<miutil::miString> stationList;
    bool hasStationKey=false;
    bool allStations=false;
    bool hasQcIdKey=false;
    string prefix("kvalobs_service_");
    string subtype;   //Subscriber type 
    bool   res;

    fs.open(fname.c_str(), ios_base::in);

    if(!fs.is_open()){
	LOGERROR("OPEN ERROR: readSubscriberFile <" << fname << ">!");
	return false;
    }

    while(!fs.eof()){
	if(!fs.good())
	    break;

	if(!getline(fs, buf))
	  continue;
	
	buf.trim();

	if(buf.empty())
	  continue;

	i=buf.find(":");
	
	if(i==string::npos){
	    LOGERROR("FORMAT ERROR: readSubscriberFile <" 
		     << fname << "> failed!");
	    fs.close();
	    return false;
	}
	
	key=buf.substr(0, i);
	val=buf.substr(i+1);
	
	val.trim();
	
	if(key.find("Last call")!=string::npos){
	    lastCall=miTime(val);
	}else if(key.find("Created")!=string::npos){
	    created=miTime(val);
	}else if(key.find("Subid")!=string::npos){
	    subid=val;
	}else if(key.find("CORBA ref")!=string::npos){
	    cref=val;
	}else if(key.find("StatusId")!=string::npos){
	    statusid=val;
	}else if(key.find("QcId")!=string::npos){
	    hasQcIdKey=true;
	    qcIdList=val.split();
	}else if(key.find("Stations")!=string::npos){
	    hasStationKey=true;

	    if(val.empty()){
		//Threat as "None"
		allStations=false;
	    }else if(val.find("All")){
		allStations=true;
	    }else if(val.find("None")){
		allStations=false;
	    }else{
		allStations=false;
		stationList=val.split();
	    }
	}else{
	    LOGWARN("Uknown key <" << key << ">: readSubscriberFile <" 
		    << fname << ">!");  
	}


    }
  
    if(!fs.eof()){
	LOGERROR("IO Error: readSubscriberFile <" << fname << "> failed!");
	fs.close();
	return false;
    }
    
    fs.close();

    if(subid.empty() || cref.empty()){
	LOGERROR("Format error: readSubscriberFile <" << fname << "> "
		 << "missing key <Subid> or <CORBA ref>!");
	return false;
    }



    i=subid.find(prefix);

    if(i==string::npos){
	LOGERROR("Inavlid subscriberid [missing prefix]: subid <" << subid <<
		 ">,  readSubscriberFile <" << fname << ">!");
	return false;
    }

    i=subid.find("_", prefix.length());

    if(i==string::npos){
	LOGERROR("Inavlid subscriberid [missing subscriber type]: " << subid <<
		 ">,  readSubscriberFile <" << fname << ">!");
	return false;
    }
    
    subtype=subid.substr(prefix.length(), i-prefix.length());
    
    if(subtype=="datanotify"){
	if(!hasStationKey){
	    LOGERROR("Missing key <Stations> for datanotify subscriber: " <<
		     "readSubscriberFile <" << fname << ">!");
	    return false;
	}

	res=addDataNotifyFromFileFile(subid,
				      cref, 
				      statusid, 
				      qcIdList, 
				      stationList);
	  
    }else if(subtype=="data"){
	if(!hasStationKey){
	    LOGERROR("Missing key <Stations> for data subscriber: " <<
		     "readSubscriberFile <" << fname << ">!");
	    return false;
	}

	res=addDataFromFile(subid,
			       cref, 
			       statusid, 
			       qcIdList, 
			       stationList);
    }else if(subtype=="kvHint"){
	res=addKvHintFromFile(subid, cref);
    }else{
	LOGERROR("Unknown subscriber type: " << subtype << 
		 "\nsubscriberid:          " << subid   << 
		 "\nreadSubscriberFile:    " << fname      );
	return false;
    }

    if(res){
      LOGINFO("Added subscriber <" << subtype 
	      << "> from file <" << fname << ">!");
      return true;
    }else{
      LOGERROR("Could not add subscriber <" << subtype 
	      << "> from file <" << fname << ">!");
      return false;
    }

 }

bool 
KvSubscriberCollection::writeSubscriberFile(const std::string &subid)
{
    string prefix("kvalobs_service_");
    string::size_type i;
    string subtype;   //Subscriber type 

    i=subid.find(prefix);

    if(i==string::npos){
	LOGERROR("Inavlid subscriberid [missing prefix]: " << subid );
	return false;
    }

    i=subid.find("_", prefix.length());

    if(i==string::npos){
	LOGERROR("Inavlid subscriberid [missing subscriber type]: " << subid );
	return false;
    }
    
    subtype=subid.substr(prefix.length(), i-prefix.length());

    if(subtype=="datanotify"){
	return writeDataNotifyFile(subid);
    }else if(subtype=="data"){
	return writeDataFile(subid);
    }else if(subtype=="kvHint"){
	return writeKvHintFile(subid);
    }else{
	LOGERROR("Unknown subscriber type: " << subtype << 
		 " [subscriberid: " << subid << "]");
	return false;
    }

    //Not reached!!!!!!
    return false;
}

bool
KvSubscriberCollection::writeDataNotifyFile(const std::string &subid)
{
    CorbaHelper::CorbaApp *cApp=CorbaHelper::CorbaApp::getCorbaApp();
    CKvalObs::CService::kvDataNotifySubscriber_var subscriber;
    ofstream *fs=0;
    std::list<KvDataNotifySubscriberPtr>::iterator itAllStations;
    std::multimap<long, KvDataNotifySubscriberPtr>::iterator it; 
    string  corbaRef;
    kvalobs::KvDataSubscriberInfo info;

    if(!cApp){
	LOGFATAL("Cant obtain a referance to CorbaHelper::CorbaApp.");
	return false;
    }

    for(itAllStations=allStationsDataNotifySubscribers.begin();
	itAllStations!=allStationsDataNotifySubscribers.end();
	itAllStations++){

	if((*itAllStations)->subscriberid()==subid)
	    break;
    }

    if(itAllStations!=allStationsDataNotifySubscribers.end()){
	subscriber=(*itAllStations)->subscriber();
	corbaRef=cApp->corbaRef(subscriber);
	info=(*itAllStations)->subscriberInfo();

	fs=writeFileHeader(subid, &info, corbaRef);

	if(fs){
	    (*fs) << "All\n";
	    fs->close();
	    delete fs;
	    return true;
	}else
	    return false;
    }
	
    for(it=stationDataNotifySubscribers.begin();
	it!=stationDataNotifySubscribers.end();
	it++){
	
	if(it->second->subscriberid()==subid){
	    if(!fs){
		subscriber=it->second->subscriber();
		corbaRef=cApp->corbaRef(subscriber);
		info=it->second->subscriberInfo();
		
		fs=writeFileHeader(subid, &info, corbaRef);
	
		if(!fs)
		    return false;
		else
		    (*fs) << it->first;
	    }else{
		(*fs) << " " << it->first;
	    }
	}
    }
	
    if(fs){
	(*fs) << std::endl;
	fs->close();
	delete fs;
    }
	

    return true;
}

bool
KvSubscriberCollection::writeDataFile(const std::string &subid)
{

    CorbaHelper::CorbaApp *cApp=CorbaHelper::CorbaApp::getCorbaApp();
    CKvalObs::CService::kvDataSubscriber_var subscriber;
    ofstream *fs=0;
    std::list<KvDataSubscriberPtr>::iterator itAllStations;
    std::multimap<long, KvDataSubscriberPtr>::iterator it; 
    string  corbaRef;
    kvalobs::KvDataSubscriberInfo info;

    if(!cApp){
	LOGFATAL("Cant obtain a referance to CorbaHelper::CorbaApp.");
	return false;
    }

    for(itAllStations=allStationsDataSubscribers.begin();
	itAllStations!=allStationsDataSubscribers.end();
	itAllStations++){

	if((*itAllStations)->subscriberid()==subid)
	    break;
    }

    if(itAllStations!=allStationsDataSubscribers.end()){
	subscriber=(*itAllStations)->subscriber();
	corbaRef=cApp->corbaRef(subscriber);
	info=(*itAllStations)->subscriberInfo();
	
	fs=writeFileHeader(subid, &info, corbaRef);

	if(fs){
	    (*fs) << "All\n";
	    fs->close();
	    delete fs;
	    return true;
	}else
	    return false;
    }
	
    for(it=stationDataSubscribers.begin();
	it!=stationDataSubscribers.end();
	it++){
	
	if(it->second->subscriberid()==subid){
	    if(!fs){
		subscriber=it->second->subscriber();
		corbaRef=cApp->corbaRef(subscriber);
		info=it->second->subscriberInfo();

		fs=writeFileHeader(subid, &info, corbaRef);
	
		if(!fs)
		    return false;
		else
		    (*fs) << it->first;
	    }else{
		(*fs) << " " << it->first;
	    }
	}
    }
	
    if(fs){
	(*fs) << std::endl;
	fs->close();
	delete fs;
    }

    return true;;
}

bool
KvSubscriberCollection::writeKvHintFile(const std::string &subid)
{
    CorbaHelper::CorbaApp *cApp=CorbaHelper::CorbaApp::getCorbaApp();
    CKvalObs::CService::kvHintSubscriber_var subscriber;
    std::list<KvHintSubscriberPtr>::iterator it;
    ofstream *fs=0;
    string  corbaRef;

    if(!cApp){
	LOGFATAL("Cant obtain a referance to CorbaHelper::CorbaApp.");
	return false;
    }

    for(it=hintSubscriberList.begin();
	it!=hintSubscriberList.end();
	it++){

	if((*it)->subscriberid()==subid){
	  subscriber=(*it)->subscriber();
	  corbaRef=cApp->corbaRef(subscriber);
	  fs=writeFileHeader(subid, 0, corbaRef);
	  
	  if(fs){
	    (*fs) << "None\n";
	    fs->close();
	    delete fs;
	    return true;
	  }else{
	    return false;
	  }
	}
    }   
    
    return false;
}

bool 
KvSubscriberCollection::addDataNotifyFromFileFile(
    const miutil::miString &subid,
    const miutil::miString &cref, 
    const miutil::miString &statusid, 
    const std::vector<miutil::miString> &qcIdList, 
    const std::vector<miutil::miString> &stationList)
{
    CorbaHelper::CorbaApp *cApp=CorbaHelper::CorbaApp::getCorbaApp();
    CKvalObs::CService::kvDataNotifySubscriber_ptr cptrSub;
    CORBA::Object_var                              cptrCO;
    std::vector<miutil::miString>::const_iterator  it;
    KvDataNotifySubscriberPtr                      subPtr;
    KvDataNotifySubscriber                         *p;
    long                                           stationid;
    int                                            nStations=0;

    if(!cApp){
	LOGFATAL("Cant obtain a referance to CorbaHelper::CorbaApp.");
	return false;
    }
	
    cptrCO=cApp->corbaRef(cref);

    if(CORBA::is_nil(cptrCO)){
	LOGERROR("CORBA: null refernce!");
	return false;
    }

    try{
	cptrSub=CKvalObs::CService::kvDataNotifySubscriber::_narrow(cptrCO);
	
	if(CORBA::is_nil(cptrSub)){
	    LOGERROR("CORBA: null refernce! Wrong type????!!!");
	    CORBA::release(cptrSub);
	    return false;
	}
    }
    catch(...){
	LOGERROR("EXCEPTION: CORBA: null refernce! Wrong type????!!!");
	return false;
    }
    

    kvalobs::KvDataSubscriberInfo info=createKvDataInfo(statusid, qcIdList);
    

    try{
	p=new KvDataNotifySubscriber(info, cptrSub);
	subPtr.reset(p);
    }
    catch(...){
	CORBA::release(cptrSub);
	LOGERROR("OUT OF MEMMORY: KvSubscriberCollection::addDataNotifyFromFileFile!"); 
	return false;
    }

    subPtr->subscriberid(static_cast<std::string>(subid));

    if(stationList.empty()){
	allStationsDataNotifySubscribers.push_back(subPtr);
	subscribers_.insert(subPtr);
	return true;
    }

    for(it=stationList.begin();
	it!=stationList.end();
	it++){
	
	try{
	    stationid=boost::lexical_cast<long>(*it);
	}
	catch(boost::bad_lexical_cast &ex){
	    LOGERROR("NOT A NUMBER: " << *it);
	    continue;
	}
	catch(...){
	    LOGERROR("UNKNOWN Exception: trying stationid=boost::numeric_cast<long>(*it)!");
	    continue;
	}

	try{
	    stationDataNotifySubscribers.insert(make_pair(stationid, subPtr));
	    nStations++;
	}
	catch(...){
	  LOGWARN("OUT OF MEMMORY: cant add datanotify subscriber!");
	}
    }
    
    if(nStations>0){
      subscribers_.insert(subPtr);
      return true;
    }

    return false;
}

kvalobs::KvDataSubscriberInfo
KvSubscriberCollection::createKvDataInfo(
    const miutil::miString              &statusid,
    const std::vector<miutil::miString> &qcIdList)
{
    CKvalObs::CService::QcIdList                   qcList;
    CKvalObs::CService::StatusId                   sid;
    std::vector<miutil::miString>::const_iterator  it;
    CKvalObs::CService::QcId                       qcId;

    if(statusid=="All")
	sid=CKvalObs::CService::All;
    else if(statusid=="OnlyFailed")
	sid=CKvalObs::CService::OnlyFailed;
    else if(statusid=="OnlyOk")
	sid=CKvalObs::CService::OnlyOk;
    else{
	LOGERROR("UNKOWNN CKvalObs::CService::StatusId: <" << statusid
		 << ">!");
	sid=CKvalObs::CService::All;
    }
    
    it=qcIdList.begin();
    
    for(CORBA::Long i=0;
	it!=qcIdList.end();
	it++, i++){
	
	if(*it=="QC1")
	    qcId=CKvalObs::CService::QC1;
	else if(*it=="QC2d")
	    qcId=CKvalObs::CService::QC2d;
	else if(*it=="QC2m")
	    qcId=CKvalObs::CService::QC2m;
	else if(*it=="HQC")
	    qcId=CKvalObs::CService::HQC;
	else{
	    LOGERROR("UNKNOWN CKvalObs::CService::qcId: < " << *it << ">!");
	    continue;
	}
	qcList.length(i+1);
	qcList[i]=qcId;
    }

    return kvalobs::KvDataSubscriberInfo(sid, qcList);

}


bool 
KvSubscriberCollection::addDataFromFile(
    const miutil::miString              &subid,
    const miutil::miString              &cref, 
    const miutil::miString              &statusid, 
    const std::vector<miutil::miString> &qcIdList, 
    const std::vector<miutil::miString> &stationList)
{
    CorbaHelper::CorbaApp *cApp=CorbaHelper::CorbaApp::getCorbaApp();
    CKvalObs::CService::kvDataSubscriber_ptr       cptrSub;
    CORBA::Object_var                              cptrCO;
    std::vector<miutil::miString>::const_iterator  it;
    KvDataSubscriberPtr                            subPtr;
    KvDataSubscriber                               *p;
    long                                           stationid;
    int                                            nStations=0;

    if(!cApp){
	LOGFATAL("Cant obtain a referance to CorbaHelper::CorbaApp.");
	return false;
    }
	
    cptrCO=cApp->corbaRef(cref);

    if(CORBA::is_nil(cptrCO)){
	LOGERROR("CORBA: null refernce!");
	return false;
    }

    try{
	cptrSub=CKvalObs::CService::kvDataSubscriber::_narrow(cptrCO);
	
	if(CORBA::is_nil(cptrSub)){
	    LOGERROR("CORBA: null refernce! Wrong type????!!!");
	    CORBA::release(cptrSub);
	    return false;
	}
    }
    catch(...){
	LOGERROR("EXCEPTION: CORBA: null refernce! Wrong type????!!!");
	return false;
    }
    

    kvalobs::KvDataSubscriberInfo info=createKvDataInfo(statusid, qcIdList);
    

    try{
	p=new KvDataSubscriber(info, cptrSub);
	subPtr.reset(p);
    }
    catch(...){
	CORBA::release(cptrSub);
	LOGERROR("OUT OF MEMMORY: KvSubscriberCollection::addDataFromFileFile!"); 
	return false;
    }

    subPtr->subscriberid(static_cast<std::string>(subid));

    if(stationList.empty()){
	allStationsDataSubscribers.push_back(subPtr);
	subscribers_.insert(subPtr);
	return true;
    }

    for(it=stationList.begin();
	it!=stationList.end();
	it++){
	
	try{
	    stationid=boost::lexical_cast<long>(*it);
	}
	catch(boost::bad_lexical_cast &ex){
	    LOGERROR("NOT A NUMBER: " << *it);
	    continue;
	}
	catch(...){
	    LOGERROR("UNKNOWN Exception: trying stationid=boost::numeric_cast<long>(*it)!");
	    continue;
	}

	try{
	    stationDataSubscribers.insert(make_pair(stationid, subPtr));
	    nStations++;
	}
	catch(...){
	  LOGWARN("OUT OF MEMMORY: cant add data subscriber!");
	}
    }
    
    if(nStations>0){
      subscribers_.insert(subPtr);
      return true;
    }

    return false;
}

bool 
KvSubscriberCollection::addKvHintFromFile(const miutil::miString &subid, 
			 const miutil::miString &cref)
{
    CorbaHelper::CorbaApp *cApp=CorbaHelper::CorbaApp::getCorbaApp();
    CKvalObs::CService::kvHintSubscriber_ptr       cptrSub;
    CORBA::Object_var                              cptrCO;
    std::vector<miutil::miString>::const_iterator  it;
    KvHintSubscriberPtr                            subPtr;
    KvHintSubscriber                               *p;

    if(!cApp){
	LOGFATAL("Cant obtain a referance to CorbaHelper::CorbaApp.");
	return false;
    }
	
    cptrCO=cApp->corbaRef(cref);

    if(CORBA::is_nil(cptrCO)){
	LOGERROR("CORBA: null refernce!");
	return false;
    }

    try{
	cptrSub=CKvalObs::CService::kvHintSubscriber::_narrow(cptrCO);
	
	if(CORBA::is_nil(cptrSub)){
	    LOGERROR("CORBA: null refernce! Wrong type????!!!");
	    CORBA::release(cptrSub);
	    return false;
	}
    }
    catch(...){
	LOGERROR("EXCEPTION: CORBA: null refernce! Wrong type????!!!");
	return false;
    }
    
    try{
      p=new KvHintSubscriber(cptrSub);
      subPtr.reset(p);
       }
    catch(...){
      CORBA::release(cptrSub);
      LOGERROR("OUT OF MEMMORY: KvSubscriberCollection::addKvHintFromFileFile!"); 
      return false;
    }

    subPtr->subscriberid(static_cast<std::string>(subid));

    hintSubscriberList.push_back(subPtr);


    return true;
}


bool 
KvSubscriberCollection::removeSubscriberFile(const std::string &subscriberid)
{
  string path(subPath+subscriberid+".sub");
  string newpath(subPath+"/terminated/"+subscriberid+".sub");
  
  if(rename(path.c_str(), newpath.c_str())!=0){
    if(unlink(path.c_str())<0)
      return false;
  }

  return true;
}



std::ostream& operator<<(std::ostream& os, 
			 const KvSubscriberCollection &c)
{
    boost::mutex::scoped_lock lock(const_cast<KvSubscriberCollection&>(c).mutex);
  os << "\nKvSubscriberCollection: \n";
  os << "  allStationDataNotifySubscribers: Antall: " 
     << c.allStationsDataNotifySubscribers.size() 
     << endl;
  
  list<KvDataNotifySubscriberPtr>::const_iterator it;
  
  it=c.allStationsDataNotifySubscribers.begin(); 

  
  for(;it!=c.allStationsDataNotifySubscribers.end(); it++){
    os << "      subscriberid: " << (*it)->subscriberid() 
       << "  " << (*it)->subscriberInfo() << endl;
  }

  os << "\n  stationsDataNotifySubscribers: Antall: " 
     << c.stationDataNotifySubscribers.size() 
     << endl;
  
  multimap<long, KvDataNotifySubscriberPtr>::const_iterator it1, itl, itu;
  
  it1=c.stationDataNotifySubscribers.begin();

  for(;it1!=c.stationDataNotifySubscribers.end(); it1++){
      os << "   stationid: " << it1->first << endl;
         
      itl=c.stationDataNotifySubscribers.lower_bound(it1->first);
      
      if(itl!=c.stationDataNotifySubscribers.end()){
	  itu=c.stationDataNotifySubscribers.upper_bound(it1->first);
	  
	  for(;itl!=itu; itl++){
	      os << "      subscriberid: " << itl->second->subscriberid()
		 << "  " <<  itl->second->subscriberInfo() << endl;
	  }
      }
  }
  
  return os;
}

