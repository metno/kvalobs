/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: StationInfoParse.cc,v 1.12.2.6 2007/09/27 09:02:22 paule Exp $                                                       

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
#include <sstream>
#include <stdio.h>
#include <milog/milog.h>
#include "StationInfo.h"
#include "StationInfoParse.h"

using namespace std;
using namespace miutil::conf;


bool
StationInfoParse::DefaultVal::
valid()const{
  	if(copyto.empty() || owner.empty() || list.empty() || delay.empty()){
    	ostringstream ost;
    	ost << "Missing key: ";

    	if(copyto.empty())
      		ost << "copyto";
    	else if (owner.empty())
      		ost << "owner";
    	else if(list.empty())
      		ost << "list";
    	else 
      		ost << "delay";

    	LOGERROR(ost.str() << "!" << endl);
    	return false;
  	}
  	
  	return true;
}
 

bool 
StationInfoParse::parse(miutil::conf::ConfSection *conf,
						std::list<StationInfoPtr> &stationList)
{
  	ConfSection *wmoDefault;
  	ConfSection *wmoSec;
  	string      wmono;
  	int         iWmono;
  	StationInfo *info;

  	stationList.clear();

  	wmoDefault=conf->getSection("wmo_default");
  
  	if(!wmoDefault){
   		LOGFATAL("Missing section <wmo_default> in the configuration file!");
    	return false;
  	}

  	if(!doWmoDefault(wmoDefault)){
    	LOGFATAL("Fatal errors in  <wmo_default> in the configuration file!");
    	return false;
  	}
  
  	list<string> sect=conf->getSubSections();
  	string::size_type i;
  
  	LOGDEBUG("Sections: (" << sect.size() << ")" << endl << 
	   		 "wmo_default: " << endl << *wmoDefault << endl);

  	for(list<string>::iterator it=sect.begin(); 
      	it!=sect.end(); it++){
    	i=it->find("wmo_");
    
    	if(i==string::npos)
      		continue;

    	wmono=it->substr(i+4);

    	if(wmono=="default")
      		continue;

    	wmoSec=conf->getSection(*it);
    
    	if(!wmoSec){
      		LOGERROR("FATAL: Missing wmoSec for <" << *it << ">!" << endl);
      		continue;
    	}
   
    	iWmono=atoi(wmono.c_str());

    	info=parseSection(wmoSec, iWmono);

    	if(!info){
      		LOGERROR("Cant parse section <" << *it << ">!");
      		continue;
    	}

    	stationList.push_back(StationInfoPtr(info));
  	}
  
  	return true;
}

StationInfo* 
StationInfoParse::parseSection(miutil::conf::ConfSection *stationConf, 
			       			   int wmono)
{
 	const char *keywords[]={"stationid", "delay", "precipitation",
		    		  "typepriority", "list",
		    		  "copy", "copyto", "owner", "loglevel", 0};

  	list<std::string>           keys;
  	list<std::string>::iterator it;
  	int                         i;
  	StationInfo                 *st=0;
  	ValElementList              value;
  	bool                        ok;

  	keys=stationConf->getKeys();

  	for(it=keys.begin(); it!=keys.end(); it++){
    	for(i=0; keywords[i]; i++){
      		if(keywords[i]==*it)
				break;
    	}
    
    	if(!keywords[i]){
      		LOGERROR("Unknown key <" << *it << "> in WMO section <" << wmono 
	      			 << ">!" << endl);
    	}
  	}
  
  	try{
    	st=new StationInfo();
  	}
  	catch(...){
    	LOGFATAL("NOMEM" << endl);
    	return 0;
  	}
  
  	st->wmono_=wmono;

  	for(i=0; keywords[i]; i++){
    	value=stationConf->getValue(keywords[i]);
      
    	if(value.empty()){
      		if(keywords[i]=="precipitation"){
				LOGDEBUG6("NO VALUE: for key <" << keywords[i] << "> in WMO section <" 
						  << wmono << ">! Using default value!" << endl);
				st->precipitation_=defVal.precipitation;
      		}else if(keywords[i]=="list"){
				LOGDEBUG6("NO VALUE: for key <" << keywords[i] << "> in WMO section <" 
		 		<< wmono << ">! Using default value!" << endl);
				st->list_=defVal.list;
      		}else if(keywords[i]=="copy"){
				LOGDEBUG6("NO VALUE: for key <" << keywords[i] << "> in WMO section <" 
						  << wmono << ">! Using default value!" << endl);
				st->copy_=defVal.copy;
      		}else if(keywords[i]=="copyto"){
				LOGDEBUG6("NO VALUE: for key <" << keywords[i] << "> in WMO section <" 
						  << wmono << ">! Using default value!" << endl);
				st->copyto_=defVal.copyto;
      		}else if(keywords[i]=="owner"){
				LOGDEBUG6("NO VALUE: for key <" << keywords[i] << "> in WMO section <" 
						  << wmono << ">! Using default value!" << endl);
				st->owner_=defVal.owner;
      		}else if(keywords[i]=="delay"){
				LOGDEBUG6("NO VALUE: for key <" <<keywords[i] << "> in WMO section <" 
		 				  << wmono << ">! Using default value!");
				st->delayList_=defVal.delay;
      		}else if(keywords[i]=="loglevel"){
				LOGDEBUG6("NO VALUE: for key <" <<keywords[i] << "> in WMO section <" 
		 				  << wmono << ">! Using default value!");
				st->loglevel_=defVal.loglevel;
      		}else{
				LOGDEBUG6("NO VALUE: for key <" << keywords[i] << "> in WMO section <" 
						  << wmono << ">! And no default value!" << endl);
				delete st;
				return 0;
      		}
    	}else{
      		ok=true;

      		if(keywords[i]=="stationid"){
				ok=doStationid(keywords[i], value, *st);
      		}else if(keywords[i]=="delay"){
				ok=doDelay(keywords[i], value, *st);
      		}else if(keywords[i]=="precipitation"){
				ok=doPrecip(keywords[i], value, *st);
      		}else if(keywords[i]=="typepriority"){
				ok=doTypePri(keywords[i], value, *st);
      		}else if(keywords[i]=="owner"){
				st->owner_=doDefOwner(value, st->wmono());
				ok=!st->owner_.empty();
      		}else if(keywords[i]=="list"){
				st->list_=doDefList(value, st->wmono());
				ok=!st->list_.empty();
     		}else if(keywords[i]=="copy"){
				st->copy_=doDefCopy(value, st->wmono());
      		}else if(keywords[i]=="copyto"){
				st->copyto_=doDefCopyto(value, st->wmono());
      		}else if(keywords[i]=="loglevel"){
				st->loglevel_=doDefLogLevel(value, st->wmono());
      		}

      		if(!ok){
				delete st;
				return 0;
      		}
    	}
  	}
  
  	if(st->stationid_.empty()){
    	LOGERROR("MISSING KEY <stationid> in WMO section <" << wmono 
	    		 << ">!" << endl);
    	delete st;
    	return 0;
  	}

  	if(st->typepriority_.empty()){
    	LOGERROR("MISSING KEY <typepriority> in WMO section <" << wmono 
	 		     << ">!" << endl);
    	delete st;
    	return 0;
  	}

  	return st;
}


bool 
StationInfoParse::
doWmoDefault(miutil::conf::ConfSection *stationConf)
{
  	list<std::string>           keys;
  	list<std::string>::iterator it;
  	ValElementList              value;

  	keys=stationConf->getKeys();

  	for(it=keys.begin(); it!=keys.end(); it++){
    	value=stationConf->getValue(*it);
      
    	if(value.empty()){
      		LOGWARN("NOVALUE: for key <" << *it << "> in <wmo_default> section!"); 
      		continue;
    	}

	    if(*it=="list"){
	      defVal.list=doDefList(value, 0);
	    }else if(*it=="owner"){
	      defVal.owner=doDefOwner(value, 0);
	    }else if(*it=="precipitation"){
	      defVal.precipitation=doDefPrecip(value, 0);
	    }else if(*it=="copy"){
	      defVal.copy=doDefCopy(value, 0);
	    }else if(*it=="copyto"){
	      defVal.copyto=doDefCopyto(value, 0);
	    }else if(*it=="delay"){
	      defVal.delay=doDefDelay(value, 0);
	    }else if(*it=="loglevel"){
	      defVal.loglevel=doDefLogLevel(value, 0);
	    }else{
	      LOGWARN("UNKNOWN KEY: in <wmo_default> section!");
	    }
  	}

  	if(defVal.delay.empty()){
    	defVal.delay.push_back(DelayInfo(DelayInfo::FSTIME, 5, false));
    	LOGINFO("Setting default delay to: " << defVal.delay.front() << endl);
  	}
  
  	return defVal.valid();
}


milog::LogLevel 
StationInfoParse::
doDefLogLevel(miutil::conf::ValElementList &vl,
	    	  int wmono)
{
  	int val;
  	IValElementList it=vl.begin();

  	if(it==vl.end())
    	return milog::INFO;
  
  	if(it->type()!=INT){
    	LOGERROR("INVALID TYPE: key <loglevel>, expecting INT."); 
    	return milog::INFO;
  	}

  	val=it->valAsInt();

  	if(val>=milog::NOTSET)
    	val=milog::INFO;

  	return static_cast<milog::LogLevel>(val);
}


std::string
StationInfoParse::
doDefList(miutil::conf::ValElementList &vl, int wmono)
{
  	IValElementList it=vl.begin();
  
  	ostringstream ost;
  
  	if(wmono>0)
    	ost << wmono;
  	else
    	ost << "default";

	if(it==vl.end())
    	return string();
  
  	if(it->type()!=STRING){
    	LOGERROR("INVALID TYPE: key <list>, expecting STRING. In WMO section <" <<
	     		 ost.str() << ">!"); 
    	return string();
  	}

  	string val=it->valAsString();
  
  	if(val.size()!=2){
    	LOGERROR("INVALID FORMAT: key <list> must have a size of two (2) char!" <<
	    		 " In WMO section <" << ost.str() << ">!");
    	return string();
  	}
  
  	return val;
}
 
std::string
StationInfoParse::
doDefOwner(miutil::conf::ValElementList &vl, int wmono)
{
  	IValElementList it=vl.begin();

  	if(it==vl.end())
		return string();
  
  	if(it->type()!=STRING){
    	LOGERROR("INVALID TYPE: key <owner>, expecting STRING."); 
    	return string();
  	}

  	string val=it->valAsString();
  
  	if(val.size()!=4){
    	LOGERROR("INVALID FORMAT: key <owner> must have a size of four (4) char!");
    	return string();
  	}
  
  	return val;
}

std::list<std::string>
StationInfoParse::
doDefPrecip(miutil::conf::ValElementList &vl, int wmono)
{
  	const char *validval[]={"RA", "RR", "RRRtr", "RR_01", "RR_1",
	   		 		   "RR_3", "RR_6","RR_12","RR_24",0};
  	int i;
  	ostringstream ost;
  	std::list<std::string> RR;
  	std::list<std::string>::iterator itRR;

  	if(wmono>0)
    	ost << wmono;
  	else
    	ost << "default";
  

  	IValElementList it=vl.begin();

  	for( ; it!=vl.end(); it++, i++){
    	if(it->type()!=STRING){
      		LOGERROR("INVALID TYPE: key <precipitation> element number: " 
	        		 << i << " in WMO section <" << ost.str() << ">!" << endl);
      		continue;
    	}
    
    	for(i=0; validval[i]; i++){
      		if(it->valAsString()==validval[i])
				break;
    	}
    
    	if(!validval[i]){
     		LOGERROR("INVALID VALUE: key <precipitation> element number: " 
	      			 << i << " in WMO section <" << ost.str() <<">!" << endl);
      		continue;
    	}
    
    	itRR=RR.begin();
      
    	for( ; itRR!=RR.end(); itRR++){
      		if(*itRR==it->valAsString())
				break;
    	}
      
    	if(itRR==RR.end())
      		RR.push_back(it->valAsString());
    
  	}
 
 	return RR;
}

bool
StationInfoParse::
doDefCopy(miutil::conf::ValElementList &vl, int wmono)
{
  	IValElementList it=vl.begin();

  	ostringstream ost;
    
  	if(wmono>0)
    	ost << wmono;
  	else
    	ost << "default";
  
  	if(it==vl.end())
		return false;
  
  	if(it->type()!=STRING){
    	LOGERROR("INVALID TYPE: key <copy>, expecting STRING." <<
	    		 " In WMO section <" << ost.str() << ">!"); 
    	return false;
  	}

  	string val=it->valAsString();
  
  	if(val=="true")
    	return true;
  	else if(val=="false")
    	return false;
  	else{
    	LOGERROR("WRONG VALUE: key <copy> valid values \"true\" or \"false\"!" <<
	    	     " In WMO section <" << ost.str() << ">!");
    	return false;
  	}
}

std::string
StationInfoParse::
doDefCopyto(miutil::conf::ValElementList &vl, int wmono)
{
  	IValElementList it=vl.begin();

  	ostringstream ost;
    
  	if(wmono>0)
    	ost << wmono;
  	else
    	ost << "default";
  
  	if(it==vl.end())
    	return string();
  
  	if(it->type()!=STRING){
    	LOGERROR("INVALID TYPE: key <copyto>, expecting STRING." <<
	    		 " In WMO section <" << ost.str() << ">!"); 
    	return string();
  	}

  	return it->valAsString();
}

StationInfo::TDelayList 
StationInfoParse::
doDefDelay(miutil::conf::ValElementList &vl, int wmono)
{
  	StationInfo::TDelayList dl;
  	IValElementList it=vl.begin();
  	string::size_type i;
  	string            val;
  	string            sHH, sMM;
  	ostringstream     section;
  	bool              force=false;
  	int               hh;
  	int               mm;

  	if(wmono==0)
    	section << "default";
  	else
    	section << wmono;

  	for(int element=1 ; it!=vl.end(); it++, element++){
    	if(it->type()!=STRING){
      		LOGERROR( "INVALID TYPE: key <delay> element number: " 
					  << element << " in WMO section <" << section.str() 
					  << ">, expecting STRING!" << endl);
      		continue;
    	}
 
    	val=it->valAsString();
  
    	i=val.find(":");

    	if(i==string::npos){
    		//Is this an forced skip SYNOP definition.
    		//An skip SYNOP i started with a - character.
    		//Valid values SS - skip SYNOP for all synoptimes.
    		//A value in the range [0, 23], skip synop for 
    		//this hour.
    		if(!val.empty() && val[0]=='-'){
    			int h;
    			val.erase(0, 1);
    			    			
    			if(val=="SS"){
    				h=-1;
    			}else{
    				h=atoi(val.c_str());
    				if(h<0 || h>23){
    					LOGERROR("Invalid hour in 'Skip synop' spec: h=" << h <<
    							" Valid values [0, 23].");
    					continue;
    				}
    			}
    
    			//Look up to see if there is any SKIP_SYNOP spec
    			//Allready parsed. If so add this hour to the 
    			//spec. This means that we only have one SKIP_SYNOP spec
    			//in the DelayInfo list for the station. The SKIP_SYNOP
    			//spec ia also at the start of the list.			
    			StationInfo::ITDelayList itd=dl.begin();
	      
    			if(itd!=dl.end())
    				if(!itd->skipSynopSpec())
    					itd=dl.end();
    			   			
    			if(itd==dl.end()){
    				dl.push_front(DelayInfo(DelayInfo::SKIP_SYNOP));
    				itd=dl.begin();
    			}
    				
    			if(h==-1){
    				for(int i=0; i<24; i+=3)
		    			itd->synopForThisHour(i, false);
    			}else{
		    		itd->synopForThisHour(h, false);
    			}
    			
    			continue;
    		}
    					
				    		
      		LOGERROR("INVALID FORMAT: key <delay> element number: " 
	       			 << element << " in WMO section <" << section.str() 
	       			 <<">, format \"hh:mm\"!" << endl);
      		continue;
    	}
      
    	sHH=val.substr(0, i);
    	sMM=val.substr(i+1);

    	if(sHH.empty() || sMM.empty()){
     		LOGERROR("INVALID FORMAT: key <delay> element number: " 
	      			 << element << " in WMO section <" << section.str()
	      			 <<">, format \"hh:mm\"!" << endl);
     		continue;
    	}
  	
  		string::size_type ii;    
  		
    	for(ii=0; ii<sHH.length(); ii++)
      		if(!isdigit(sHH[ii]))
				break;
    
    	force=false;
    
    	//    cerr << "ii=" << ii << " sHH=" << sHH << " sMM=" << sMM << endl;

    	if(ii<sHH.length()){
      		if(sHH=="FS" || sHH=="SS" || sHH=="FH" || sHH=="HH"){
				if(sHH[0]=='F')
	  				force=true;

				if(sHH[1]=='S')
	  				hh=DelayInfo::STIME;
				else
	  				hh=DelayInfo::HTIME;
      		}else if(sHH=="fS"){
				hh=DelayInfo::FSTIME;
      		}else{
				LOGERROR("INVALID FORMAT: key <delay> element number: " 
		 				 << element << " in WMO section <" << section.str() 
		 				 <<">, format \"hh:mm\", where hh is a number [0,23] or the " 
		 				 << "one of the values FH, HH, FS or SS. mm is a number" 
		 				 << "[0,59]!" << endl);
				continue;
      		}
    	}else{
      		hh=atoi(sHH.c_str());

      		if(hh<0 || hh>23){
				LOGERROR("INVALID FORMAT: key <delay> element number: " 
						<< element << " in WMO section <"<< section.str() 
						<<">, format \"hh:mm\", where hh is a number [0,23] or " 
						<< "one of the values FH, HH, FS or SS. mm is a number"
						<< " [0,59]!" << endl);
				continue;
      		}
    	}

    	for(ii=0; ii<sMM.length(); ii++)
      		if(isdigit(sMM[ii]))
				break;

    	if(ii>=sMM.length()){
      		LOGERROR("INVALID FORMAT: key <delay> element number: " 
	    			 << element << " in WMO section <" << section.str() 
	    			 <<">, format \"hh:mm\", where hh is a number [0,23] or the " 
	    			 << "one of the values FH, HH, FS or SS. mm is a number [0,59]!" 
	    			 << endl);
      		continue;
    	}

    	mm=atoi(sMM.c_str());

    	if(mm<0 || mm>59){
      		LOGERROR( "INVALID FORMAT: key <delay> element number: " 
			   		  << element << " in WMO section <" << section.str() 
					  <<">, format \"hh:mm\", where hh is a number [0,23] or the " 
					  << "one of the values FH, HH, FS or SS. mm is a number"
					  << " [0,59]!" << endl);
      		continue;
    	}

    	StationInfo::ITDelayList itd=dl.begin();
      
    	for( ; itd!=dl.end(); itd++){
      		if(hh>=itd->hour())
				break;
    	}
  
    	if(itd==dl.end()){
     		dl.push_back(DelayInfo(hh, mm, force));
    	}else if(hh==itd->hour()){
      		LOGERROR("DUPLICATE ELEMENT: key <delay> element number: " 
	    			 << element << " in WMO section <" << section.str() 
	    			 <<">"  << endl);
      		continue;
    	}else{
      		dl.insert(itd, DelayInfo(hh, mm, force));
    	}
  
  	}
  
  	return dl;
}
   


bool
StationInfoParse::
doStationid(const std::string &key,
	    	miutil::conf::ValElementList &vl, 
	    	StationInfo &st)
{
  	int i=1;
  	IValElementList it=vl.begin();

  	for( ; it!=vl.end(); it++, i++){
    	if(it->type()!=INT){
      		LOGERROR("INVALID TYPE: key <" << key << "> element number: " 
	   				<< i << endl);
    	}else{
      		StationInfo::ITLongList itl=st.stationid_.begin();
      
      		for( ; itl!=st.stationid_.end(); itl++){
				if(*itl==it->valAsInt())
	 				break;
      		}
      
      		if(itl==st.stationid_.end())
				st.stationid_.push_back(it->valAsInt());
    	}
  	}
  
  	if(st.stationid_.empty()){
    	LOGERROR("NOVALUE: key <" << key << "> has now valid values!" << endl);
    	return false;
  	}

  	return true;
}

bool
StationInfoParse::
doDelay(const std::string &key,
		miutil::conf::ValElementList &vl, StationInfo &st)
{

  	st.delayList_=doDefDelay(vl, st.wmono());

  	if(st.delayList_.empty()){
    	LOGWARN("Nol value for <delay> in WMO section " << st.wmono()
	    		<< " using default!");
    	st.delayList_=defVal.delay;
  	}

  	return true;
}


bool
StationInfoParse::
doPrecip(const std::string &key,
	 	 miutil::conf::ValElementList &vl, StationInfo &st)
{  
  	std::list<std::string> RR=doDefPrecip(vl, st.wmono());
    
  	st.precipitation_=RR;
  
  	return !RR.empty();
}

bool
StationInfoParse::
doTypePri(const std::string &key,
	  	  miutil::conf::ValElementList &vl, StationInfo &st)
{
  	bool error=false;
  	int i;
  	bool mustHaveType;
   long mytype;
   string smytype;
  	
  	IValElementList it=vl.begin();

  	for(i=0; it!=vl.end(); it++, i++){
    	mustHaveType=false;

    	if(it->type()!=INT && it->type()!=STRING){
      		LOGERROR("INVALID TYPE: key <" << key << "> element number: " 
	       			 << i << " in WMO section <" << st.wmono() << ">!" <<  endl);
      		error=true;
      		continue;
    	}

    	if(it->type()==STRING){
     		string val=it->valAsString();
     		string::size_type ii=val.find_first_of("*");
      
     		if(ii!=string::npos){
     			  mustHaveType=true;
     			  val.erase(0, ii+1);
     		}

     		ii=val.find_first_of(":");

			if(ii==string::npos){
	     		smytype=val;
			}else{
      	   smytype=val.substr(0, ii);
      	   val.erase(0, ii+1);
			}
      	   
      	   
     		if(sscanf(smytype.c_str(), "%ld", &mytype)!=1){
				LOGERROR("INVALID TYPE: key <" << key << "> element number: " 
		 				 << i << " in WMO section <" << st.wmono() 
		 				 << ">! Not an integer!"  <<  endl);
				error=true;
				continue;
     		}
    	}else{
      		mytype=it->valAsInt();
    	}

    	StationInfo::ITTypeList itl=st.typepriority_.begin();
      
    	for( ; itl!=st.typepriority_.end(); itl++){
     		if(itl->typeID()==mytype)
			break;
    	}
      
    	if(itl==st.typepriority_.end()){
  			StationInfo::Type  type(mytype);
    			
   		if(mustHaveType)
				type.mustHaveType(true);
    			
     		st.typepriority_.push_back(type);
    	}
  	}

 	
	//Check if we have at least one mustHaveType. If not set
	//all types to mustHaveTypes.
	
	mustHaveType=false;
	      
   for(StationInfo::ITTypeList itl=st.typepriority_.begin();
   	 itl!=st.typepriority_.end(); 
   	 itl++){
    	if(itl->mustHaveType()){
    		mustHaveType=true;
    		break;
    	}
   }
   
   if(!mustHaveType){
		for(StationInfo::ITTypeList itl=st.typepriority_.begin();
   	 	 itl!=st.typepriority_.end(); 
   	 	 itl++)
    		itl->mustHaveType(true);
   }
  
  	if(error)
    	return false;

  	return !st.typepriority_.empty();
}

