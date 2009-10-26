/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: SynopWorker.cc,v 1.27.2.21 2007/09/27 09:02:23 paule Exp $                                                       

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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <sstream>
#include <kvalobs/kvDbGate.h>
#include <list>
#include <iomanip>
#include "Data.h"
#include <milog/milog.h>
#include <fstream>
#include <boost/crc.hpp>
#include <boost/cstdint.hpp>
#include "StationInfo.h"
#include "SynopWorker.h"
#include "obsevent.h"
#include "synop.h"
#include <kvalobs/kvPath.h>

using namespace std;
using namespace kvalobs;
using namespace miutil;
using namespace milog;



SynopWorker::SynopWorker(App &app_, 
			 dnmi::thread::CommandQue &que_,
			 dnmi::thread::CommandQue &replayQue_)
  : app(app_), que(que_), replayQue(replayQue_), con(0), 
    swmsg(*(new std::ostringstream()))
{
}
  
void 
SynopWorker::operator()()
{
  	dnmi::thread::CommandBase *com;
  	ObsEvent                  *event;

  	milog::LogContext context("SynopWorker");

  	while(!app.shutdown()){
    	try{
      	com=que.get(1);
    	}
    	catch(dnmi::thread::QueSuspended &e){
      	LOGDEBUG6("EXCEPTION(QueSuspended): Input que is susspended!" << endl);
      	continue;
    	}
   	catch(...){
      	LOGDEBUG("EXCEPTION(Unknown): Uknown exception from input que!" << endl);
      	continue;
    	}

    	if(!com)
      	continue;
    
    	event=dynamic_cast<ObsEvent*>(com);

    	if(!event){
      	LOGERROR("Unexpected event!");
      	delete com;
      	continue;
    	}

    	if(!con){
      	con=app.getNewDbConnection();

      	if(!con){
				LOGFATAL("Cant create a connection to the database!\n");
				delete event;
				continue;
      	}
    	}

    	LOGINFO("New observation: ("<<event->stationInfo()->wmono() << ") "
	    		  << event->obstime() << " regenerate: " 
	    		  << (event->regenerate()? "T":"F") << " client: " 
	    		  << (event->hasCallback()?"T":"F"));

    	try{
#ifdef SMHI_LOG
		FDLogStream *logs=new FDLogStream(DAY, DEFAULT_DAY_FORMAT); //One log per day.
#else
      	FLogStream *logs=new FLogStream(2, 307200); //300k
#endif
      	std::ostringstream ost;

      	ost << kvPath("localstatedir") << "/log/kvsynop/" 
	  			 << event->stationInfo()->wmono() << ".log";

      	if(logs->open(ost.str())){
				Logger::setDefaultLogger(logs);
				Logger::logger().logLevel(event->stationInfo()->loglevel());
				
				//Log to stationspecific logfile also.
				LOGINFO("+++++++ Start processing observation +++++++" << endl 
						  << "New observation: ("<<event->stationInfo()->wmono() << ") "
	    		  		  << event->obstime() << " regenerate: " 
	    		        << (event->regenerate()? "T":"F") << " client: " 
	    		        << (event->hasCallback()?"T":"F"));
      	}else{
				LOGERROR("Cant open the logfile <" << ost.str() << ">!");
				delete logs;
      	}
    	}
    	catch(...){
      	LOGERROR("Cant create a logstream for wmono: " << 
	      		   event->stationInfo()->wmono() );
    	}

    	swmsg.str("");

    	try{
      	newObs(*event);
    	}
    	catch(...){
      	LOGERROR("EXCEPTION(Unknown): Unexpected exception from " <<
	      			"SynopWorker::newObs" << endl);
    	}

		if(event->hasCallback()){
      	replayQue.postAndBrodcast(event);
    	}else{
      	delete event;
    	}

		LOGINFO("------- End processing observation -------");

    	Logger::resetDefaultLogger();
    
    	LOGINFO(swmsg.str());

  	}
  
  	if(con)
   	app.releaseDbConnection(con);
} 


bool
SynopWorker::
readyForSynop(const DataEntryList &data, 
	                  ObsEvent      &e   )const
{
  	bool haveAllTypes;
  	bool mustHaveTypes;
  	bool force;
  	int  min;
  	bool delay;
  	bool relativToFirst;

  	miTime obstime=e.obstime();
  	StationInfoPtr info=e.stationInfo();

  	milog::LogContext context("readyForSynop");

  	miTime delayTime;
  	miTime nowTime(miTime::nowTime());
 
  	haveAllTypes=checkTypes(data, info, obstime, mustHaveTypes);
  	delay=info->delay(obstime.hour(), min, force, relativToFirst);

  	LOGDEBUG3("haveAllTypes:  " << (haveAllTypes?"TRUE":"FALSE") << endl <<
  				 "mustHaveTypes: " << (mustHaveTypes?"TRUE":"FALSE") << endl <<
	    		 "delay: " << (delay?"TRUE":"FALSE") << " min: " << min <<
	    		 " force: " << (force?"TRUE":"FALSE") << " relativToFirst: " <<
	    		 (relativToFirst?"TRUE":"FALSE") << endl <<
	    		 " nowTime: " << nowTime << endl <<
	    		 " #ContinuesTimes: " << data.nContinuesTimes()<< endl);

  	if(!haveAllTypes && !mustHaveTypes){
    	//We do not have all types we need, we are also missing
    	//the types we need to make an incomplete synop. Just
    	//drop this event, dont make a waiting element for it, it is 
    	//useless;
    
    	swmsg << "Missing mustHaveTypes!";
    	return false;
  	}
    
  
  	if(delay){
    	if(relativToFirst){
      	//If we allready have a registred waiting element dont replace it.
      	//This ensures that we only register an waiting element for
      	//the first data received.
    
      	if(haveAllTypes)
				return true;
      
  
      	delayTime=nowTime;
      	delayTime.addMin(min);
      	WaitingPtr wp=e.waiting();
      
      	if(!wp){
				LOGDEBUG1("Delaying (relativeToFirst): " << delayTime);
				app.addWaiting(WaitingPtr(new Waiting(delayTime, 
				      	      obstime, info)),
		      					false,
		       					con);

				swmsg << "Delay (relativToFirst): " << delayTime;
				return false;
      	}else{
				LOGDEBUG1("Is delayed (relativeToFirst) to: " << wp->delay());
				if(wp->delay()<=nowTime){
	  				LOGDEBUG1("Is delayed (relativeToFirst): expired!");
	  				return true;
				}else{
	  				LOGDEBUG1("Is delayed (relativeToFirst): Not expired!");
	  				//Reinsert in the delaylist.
	  				app.addWaiting(wp,false, con);
	  
	  				swmsg << "Delay (relativToFirst) Not expired: " << delayTime;
	  
	  				return false;
				}
			}
    	}else{ //Delay relative to obstime.
      	delayTime=miTime(obstime.date(),miClock(obstime.hour(), min, 0));
    	}
  	}

  	if(haveAllTypes){
    	if(delay){
      	if(!force){
				return true;
      	}else{
				if(delayTime<nowTime)
	  				return true;
	
				try{
	  				app.addWaiting(WaitingPtr(new Waiting(delayTime, obstime, info)),
			 							true,
			 							con);
				}
				catch(...){
	  				LOGFATAL("NOMEM: cant allocate delay element!");
				}

				swmsg << "Delay: " << delayTime;

				return false;
			}
		}else{
      	return true;
   	}
  	}
  
  	if(delay){
    	if(delayTime<nowTime){
      	if(mustHaveTypes){
				return true;
      	}else{
				return false;
      	}
    	}else{
       	try{
	 			app.addWaiting(WaitingPtr(new Waiting(delayTime, obstime, info)),
									true,
									con);
       	}
       	catch(...){
	 			LOGFATAL("NOMEM: cant allocate delay element!");
       	}
    	}
  	}else{
    	if(mustHaveTypes)
      	return true;
  	}
  
  	swmsg << "Not enough data!";
  	return false;
}



/*
  If the event has a callback registred we write some error
  message that can be returned to the celler.
 */
void 
SynopWorker::newObs(ObsEvent &event)
{
	EReadData      dataRes;
  	DataEntryList  data;   
  	SynopDataList  synopData;
  	Synop          synop;
  	string         sSynop;
  	StationInfoPtr info;
  	ostringstream  ost;

  	info=event.stationInfo();
  
  	if(!info->synopForTime(event.obstime().hour())){
  		LOGINFO("Skip SYNOP for time: " << event.obstime() << "  wmono: " << 
  				  info->wmono());
  		swmsg << "Skip SYNOP for time: " << event.obstime() << "  wmono: " << 
  				   info->wmono();
  				 
  		if(event.hasCallback()){
  			event.msg() << "Skip SYNOP for time: " << event.obstime() << 
  						   "  wmono: " << info->wmono();
  			event.synop("");
      	event.isOk(false);
  		}
  		
  		return;
  	}

  	//We check if this is a event for regeneraiting a SYNOP
  	//due to changed data. If it is, the synop for this time
  	//must allready exist. If it don't exist we could generate 
  	//a SYNOP that is incomplete because of incomplete data.

  	if(event.regenerate()){
    	list<TblSynop> tblSynopList;

    	LOGINFO("Regenerate event: wmono " << info->wmono() << ", obstime " <<
	    		  event.obstime());
    
    	if(app.getSavedSynopData(info->wmono(), 
							          event.obstime(), 
			     				       tblSynopList, *con)){
			if(tblSynopList.size()>0){
				LOGINFO("Regenerate event: Regenerate SYNOP.");
      	}else{
				LOGINFO("Regenerate event: No SYNOP exist, don't regenerate!");
				swmsg << "Regenerate: No synop exist.";

				return;
     		}
		}else{
      	LOGERROR("DBERROR: Regenerate event: Cant look up the synop!");
      	swmsg << "Regenerate: DB error!";

      	return;
    	}
  	}


  	dataRes=readData(*con, event, data);

  	if(dataRes!=RdOK){
    	if(event.hasCallback()){
      	event.isOk(false);
      	
      	switch(dataRes){
      	case RdNoStation:
				event.msg() << "NOSTATION: No station congigured!";
				break;
      	case RdNoData:
				event.msg() << "NODATA: No data!";
				break;
      	case RdMissingObstime:
				event.msg() << "MISSING OBSTIME: No data for the obstime!";
				break;
      	default:
				event.msg() << "READERROR: cant read data!";
      	}
    	}

    	swmsg << event.msg().str();

    	return;
  	}
  

  	//If this event comes frrom the DelayControll and
  	//is for data waiting on continues types don't run
  	//it through the readyForSynop test. We know that 
  	//the readyForSynop has previously returned true for
  	//this event.
 
  	if(!event.waitingOnContinuesData()){
    	//Don't delay a observation that is explicit asked for.
    	//A SYNOP that is explicit asked for has a callback.

    	if(!event.hasCallback() && !readyForSynop(data, event)){
      	return;
    	}
  	}

  	//Check if shall test for continuesTypes. We do that
  	//if we have now Waiting pointer or we have a Waiting pointer
  	//but it has not been tested for waiting on continues data.
  	if(!checkContinuesTypes(event, data)){
    	return;
  	}
    
  	app.removeWaiting(info->wmono(), event.obstime(), con);
  
  	LOGINFO("ReadData: wmono: " <<info->wmono() << " obstime: " <<
			  event.obstime() << " # " << data.size());
  
  	try{
    	loadSynopData(data, synopData, event.stationInfo());
  	}
  	catch(...){
    	LOGDEBUG("EXCEPTION(Unknown): Unexpected exception from "<< endl <<
	     		   "SynopWorker::loadSynopData" << endl);
  	}
  
  	CISynopDataList it=synopData.begin();
  	ost.str("");

  	for(int i=0;it!=synopData.end(); i++, it++)
    	ost << it->time() << "  [" << i << "] " << synopData[i].time() <<endl;
  
  	LOGINFO("# number of synopdata: " << synopData.size() << endl<<
   		  "Continues: " << synopData.nContinuesTimes() << endl <<
	   	  "Time(s): " << endl << ost.str());
  
  	LOGDEBUG6(synopData);

  	bool synopOk;

  	try{
#ifdef USE_KVDATA
		synopOk=synop.doSynop(info->wmono(),
			  				   	 info->owner(),
									 atoi(info->list().c_str()),
			  				  		 sSynop, 
			  				  		 info,
			  				  		 synopData,
			  				  		 true,
									 data,
									 app.isExportAsKvData());
#else
    	synopOk=synop.doSynop(info->wmono(),
			  				   	 info->owner(),
									 atoi(info->list().c_str()),
			  				  		 sSynop, 
			  				  		 info,
			  				  		 synopData,
		  				  		     true);
#endif
  	}
  	catch(std::out_of_range &e){
    	LOGWARN("EXCEPTION: out_of_range: wmono: " << info->wmono() <<
	    		  " obstime: "  <<
	    		  ((synopData.begin()!=synopData.end())?
					synopData.begin()->time():"(NULL)") << endl <<
	    		  "what: " << e.what() << endl);
    	synopOk=false;
    	swmsg << "Cant create a synop!" << endl;
  	}
  	catch(DataListEntry::TimeError &e){
     	LOGWARN("Exception: TimeError: wmono: " << info->wmono() << " obstime: "  <<
    			  ((synopData.begin()!=synopData.end())?
				  synopData.begin()->time():"(NULL)") << endl<<
				  "what: " << e.what() << endl);
    	synopOk=false;
    	swmsg << "Cant create a synop!" << endl;
  	}
  	catch(...){
    	LOGWARN("EXCEPTION(Unknown): Unexpected exception in Synop::doSynop:" <<
	    		  endl << "wmono: " << info->wmono() << " obstime: "  <<
	   		  ((synopData.begin()!=synopData.end())?
				  synopData.begin()->time():"(NULL)") << endl);
    	synopOk=false;
    	swmsg << "Cant create a synop!" << endl;
  	}
  
  	if(!synopOk){
    	if(event.hasCallback()){
      	event.isOk(false);
      
      	if(synopData.size()==0)
				event.msg() << "NODATA:(" << event.obstime() <<") cant create synop!";
      	else
				event.msg() << "SYNOPERROR:(" << event.obstime() <<") cant create synop!";
    	}

    	LOGERROR("Cant create SYNOP for <"<< info->wmono()<<"> obstime: " <<
	     		   event.obstime());
    	swmsg << "Cant create a synop!" << endl;
  	}else{
    	boost::crc_ccitt_type crcChecker;
    	boost::uint16_t       crc;
    	boost::uint16_t       oldcrc=0;
    	int                   ccx=0;
    	list<TblSynop>        tblSynopList;
    	bool                  newSynop=true;
    	bool                  bccx=false;
    	string                mySynop;

    	crcChecker.process_bytes(sSynop.c_str(), sSynop.length());
    	crc=crcChecker.checksum();
    
    	if(app.getSavedSynopData(info->wmono(), event.obstime(), tblSynopList, *con)){
      	if(tblSynopList.size()>0){
				ccx=tblSynopList.front().ccx();
				oldcrc=tblSynopList.front().crc();
	
				if(oldcrc!=crc){
	  				mySynop=tblSynopList.front().wmomsg();
	  				mySynop+="\n\n";
	  				ccx++;
	  				bccx=true;
				}else{
	  				newSynop=false;
				}
      	}
    	}

    	Synop::replaceCCCXXX(sSynop, ccx);

    	if(newSynop){
      	miTime createTime(miTime::nowTime());
      	ostringstream myost(mySynop);
      
      	myost << "[Created: " << createTime << "]" << endl;
      	myost << sSynop;

      	if(app.saveSynopData(TblSynop(info->wmono(), event.obstime(), 
				 				 		createTime, crc, ccx, myost.str()), 
			   					 	*con))
				LOGINFO("Synop information saved to database!");

     		saveTo(info, event.obstime(), sSynop, ccx);
      
     		if(!bccx)
				swmsg << "New synop created!" << endl;
     		else
				swmsg << "New synop created (CC" << ('A'+(ccx-1)) << ")!" << endl;

    	}else{
     		LOGINFO("DUPLICATE: wmono=" << info->wmono() << " obstime: " 
       			 << event.obstime());
      
     		swmsg << "Duplicate synop created!" << endl;
      
     		if(event.hasCallback())
				event.msg() << "DUPLICATE: wmono=" << info->wmono() << " obstime: " 
		    					<< event.obstime();
      		
    	}

    	ost.str("");

    	if(bccx)
      	ost << " (CC" << ('A'+(ccx-1)) << ")";
    	
    	LOGINFO("SYNOP "<< info->wmono() << " crc=" << crc << " oldcrc=" << oldcrc
	    		  << ost.str() << " : " << endl << sSynop << endl);
    
    	if(event.hasCallback()){
      	//If we have a callback registred. Return the synop
      	event.synop(sSynop);
      	event.isOk(true);
    	}
  	}
}

SynopWorker::EReadData 
SynopWorker::readData(dnmi::db::Connection &con,
		      			 ObsEvent             &event,
		      			 DataEntryList        &data)const
{
  	ostringstream                   ost;
  	kvDbGate                        gate(&con);
  	list<Data>                      dataList;
  	list<Data>::iterator            dit;
  	DataEntryList::CITDataEntryList it;
  	StationInfo::TLongList          stIDs;
  	StationInfo::RITLongList        itStId;
  	StationInfoPtr                  station=event.stationInfo();
  	miutil::miTime                  from(event.obstime());
  	miutil::miTime                  to(event.obstime());
  	bool                            hasObstime=false;

  	data.clear();

  	if(!station)
    	return RdERROR;

  	gate.busytimeout(120);

  	from.addHour(-25);

  	stIDs=station->stationID();
  	itStId=stIDs.rbegin();

  	if(itStId==stIDs.rend()){
    	LOGERROR("No stationid's for station <" << station->wmono() << ">!");
    	return RdNoStation;
  	}
  
  	for(;itStId!=stIDs.rend(); itStId++){
   	ost.str("");
    	ost << " where stationid=" << *itStId
			 << " AND " 
			 << " obstime>=\'"      << from.isoTime() << "\' AND "
	  		 << " obstime<=\'"      << to.isoTime()   << "\'"
			 << " order by obstime;";

    	LOGDEBUG("query: " << ost.str());

    	if(!gate.select(dataList, ost.str())){
      	LOGERROR("Cant get data from the database!\nQuerry: " << ost.str() 
	       		   << endl << "Reason: " << gate.getErrorStr());
      	return RdERROR;
    	}
    
    	bool doLogTypeidInfo=true;

    	for(dit=dataList.begin(); dit!=dataList.end(); dit++){
      	try{
				if(event.hasReceivedTypeid(dit->stationID(), 
				  									dit->typeID(),
				   								doLogTypeidInfo)){
	  				data.insert(*dit);
	  
	  				if(!hasObstime && dit->obstime()==event.obstime())
	    				hasObstime=true;
				}

				doLogTypeidInfo=false;
      	}
      	catch(DataListEntry::TimeError &e){
				LOGDEBUG("EXCEPTION(DataListEntry::TimeError): Hmmm... " <<
		 					"This should not happend!!" << endl);
      	}
    	}
  	}
  
  	it=data.begin();
  
  	if(it!=data.end()){
    	ostringstream ost;
    	ost << "First obstime: " << it->obstime() << " - ";
    	it=data.end();
    	it--;
    	ost << it->obstime();
    	LOGDEBUG(ost.str());
  	}else{
    	LOGWARN("No data in the cache for the station!");
    	return RdNoData;
  	}

  	if(!hasObstime){
    	LOGERROR("No data for the obstime: " << event.obstime());
    	return RdMissingObstime;
  	}

  	return RdOK;
}

void 
SynopWorker::loadSynopData(const DataEntryList &dl, 
			   SynopDataList       &sd, 
			   StationInfoPtr      info)const
{
  StationInfo::TLongList          types;
  StationInfo::RITLongList        itt;
  DataEntryList::CITDataEntryList it;
  DataListEntry::ITDataList       itd;
  DataListEntry::TDataList        dataList;

  sd.clear();

  types=info->typepriority();
  it=dl.begin();

  for(; it!=dl.end(); it++){
    SynopData synopData;

    itt=types.rbegin();

    for(;itt!=types.rend(); itt++){
      dataList=it->getTypeId(*itt);
      
      if(dataList.empty())
	continue;

      itd=dataList.begin();
      synopData.time(itd->obstime());
      
      for(;itd!=dataList.end(); itd++){
	//COMMENT:
	//We use only the default sensor for every parameter, ie sensor=0.
	//This may be to restrective. Shuld this be a configuration option for
	//the parameters we wish to overide tis behavior for.
	
	if(itd->sensor()==0 && itd->level()==0){
	  synopData.setData(itd->paramID(), itd->original());
	}else{
	  LOGINFO("loadSynop: sensor=" << itd->sensor() << " level=" << itd->level()
		  << " not used!");
	}
      }
    }
    
    if(!synopData.undef()){
      synopData.cleanUpSlash();
      
      try{
	sd[synopData.time()]=synopData;
      }
      catch(out_of_range &ex){
	LOGDEBUG("EXCEPTION(out_of_range): Should not happend!!!"<< endl);
      }
      catch(...){
	LOGDEBUG("EXCEPTION(Unknown): Should never happend!" << endl);
      }
    }
  }
}


bool
SynopWorker::
checkTypes(const DataEntryList  &data, 
	   	         StationInfoPtr stInfo,
	        const miutil::miTime obstime,
	              bool           &mustHaveTypes)const
{
  	StationInfo::TLongList          tids=stInfo->typepriority();
  	DataEntryList::CITDataEntryList dit=data.find(obstime);
  	StationInfo::CITLongList        it;

  	mustHaveTypes=false;

  	if(dit==data.end()){
  		LOGDEBUG("checkTypes: No data for: wmono: " << stInfo->wmono() << " obstime: " << obstime);
    	return false;
  	}

  	if(dit->obstime()!=obstime){
  		LOGDEBUG("checkTypes: No data for obstime: " << obstime <<  " wmono: " << stInfo->wmono());
    	return false;
  	}

	LOGDEBUG("checkTypes: " << *dit);

  	for(it=tids.begin();it!=tids.end(); it++){
    	if(dit->size(*it)==0)
      	break;
  	}

  	if(it==tids.end()){
   	mustHaveTypes=true;
    	return true;
  	}

  	tids=stInfo->mustHaveTypes();
  
  	ostringstream ost;
  	
  	ost << "checkTypes: mustHaveTypes:";
  	
  	for(it=tids.begin();it!=tids.end(); it++)
  		ost << " " << *it; 		
  	
  	LOGDEBUG(ost.str());
  
  	for(it=tids.begin();it!=tids.end(); it++){
    	if(dit->size(*it)==0)
      	break;
  	}

  	if(it==tids.end()){
    	mustHaveTypes=true;
  	}

  	return false;
}



void
SynopWorker::
saveTo(StationInfoPtr info, 
       const miutil::miTime &obstime, 
       const std::string &wmomsg,
       int ccx)
{
  ostringstream ost;
  ofstream      f;
  struct stat   sbuf; 
  
  if(!info->copy())
    return;

  if(stat(info->copyto().c_str(), &sbuf)<0){
    if(errno==ENOENT || errno==ENOTDIR){
      LOGERROR("copyto: <"<<info->copyto()<<"> invalid path!");
    }else if(errno==EACCES){
      LOGERROR("copyto: <"<<info->copyto()<<"> permission denied!");
    }else{
      LOGERROR("copyto: <"<<info->copyto()<<">, lstat unknown error!");
    }
    
    return;
  }

  if(!S_ISDIR(sbuf.st_mode)){
    LOGERROR("copyto: <"<<info->copyto()<<"> not a directory!");
    return;
  }

  if(ccx==0){
    ost << info->copyto() << "/" <<  info->wmono() << "-"
#ifdef USE_KVDATA
	<< setfill('0') << setw(4) << obstime.year() << setw(2) << obstime.month() << setw(2) << obstime.day() << setw(2) 
#else
	<< setfill('0') << setw(2) << obstime.day() << setw(2)
#endif
	<< obstime.hour()
	<< ".synop";
  }else{ 
    ost << info->copyto() << "/" <<  info->wmono() << "-" 
#ifdef USE_KVDATA
	<< setfill('0') << setw(4) << obstime.year() << setw(2) << obstime.month() << setw(2) << obstime.day() << setw(2) 
#else
	<< setfill('0') << setw(2) << obstime.day() << setw(2)
#endif
	<< obstime.hour() << "-" << static_cast<char>('A'+(ccx-1))
	<< ".synop";
  }

  f.open(ost.str().c_str());

  if(f.is_open()){
    LOGINFO("Writing SYNOP to file: " << ost.str());
    f << wmomsg;
    f.close();
  }else{
    LOGERROR("Cant write  SYNOP to file: " << ost.str());
  }  
}
    

bool 
SynopWorker::
checkContinuesTypes(ObsEvent &event, 
		    const DataEntryList &data)const
{
  WaitingPtr w;
  StationInfoPtr info=event.stationInfo();  

  if(event.waitingOnContinuesData()){
    //We have waited on this event in the predefined time,
    //just generate a synop for this event.
    swmsg << "Expired waiting on continues data!" << endl;
    return true;
  }

  if((event.obstime().hour()%3)!=0){
    //Just interested in synoptimes that use data from 
    //multiple hours.
       
    return true;
  }

  w=app.getWaiting(event.obstime(), info->wmono(), con);
  
  
  std::list<int> contTypes=info->continuesTypeids(app.continuesTypeID());

  if(contTypes.empty()){
    //The station has only no continues type ids.
    return true;
  }

  if(!data.hasContinuesTimes(contTypes, 4)){
    if(!w){
      miutil::miTime now(miutil::miTime::nowTime());
      now.addMin(5);

      try{
	w=WaitingPtr(new Waiting(now, event.obstime(), info, true));
      }
      catch(...){
	LOGERROR("NO MEM");
	return true;
      }
    }

    w->waitingOnContinuesData(true);

    if(!app.addWaiting(w, true, con)){
      LOGERROR("Cant add a waiting element to the Waiting database.");
      return true;
    }
      
    LOGINFO("Waiting on continues data: wmono: " << info->wmono() <<
	    " obstime: " << event.obstime() << " delay: " << w->delay());

    swmsg << "Waiting on continues data until: " << w->delay();

    return false;
  }

  return true;
}
