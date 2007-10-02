/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: obsevent.h,v 1.5.2.13 2007/09/27 09:02:23 paule Exp $                                                       

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
#ifndef __obsevent_h__
#define __obsevent_h__

#include <sstream>
#include <dnmithread/CommandQue.h>
#include <puTools/miTime>
#include <kvskel/kvsynopd.hh>
#include "StationInfo.h"
#include "Waiting.h"

class ObsEvent : public dnmi::thread::CommandBase
{
  	miutil::miTime        obstime_;
  	kvsynopd::synopcb_var ref;
  	StationInfoPtr        stInfo;
  	WaitingPtr            waiting_;
  
  	///Used to send a message back in the callback \a ref
  	std::ostringstream    msg_; 
  
  	///Used to send a synop back in the callback \a ref
  	std::string           synop_;
  
  	///Used to send status indicator back in the callback \a ref
  	bool                  isOk_;  

  	///Used to indicate that a synop posibly need to be regenerated
  	///due to changed data.
  	bool                  regenerate_;
  


  	struct IdType{
    	int sid_;
    	int tid_;

    	IdType(int sid, int tid):sid_(sid), tid_(tid){}
    	IdType(const IdType &it):sid_(it.sid_), tid_(it.tid_){}

    	IdType& operator=(const IdType &rhs){
      		if(this!=&rhs){
				sid_=rhs.sid_;
				tid_=rhs.tid_;
      		}
      		return *this;
    	}
  	};


  	std::list<IdType> typeidReceived;
 	
 	public:

  		/**
   		 * \brief Constructor to be used to signal a regenerate of a SYNOP.
   		 */
 		ObsEvent(const miutil::miTime &obstime,
	  			 StationInfoPtr stInfo_,
	  			 bool regenerate=false)
    		:obstime_(obstime),
    		 ref(kvsynopd::synopcb::_nil()),
    		 stInfo(stInfo_),
    		 isOk_(false),
    		 regenerate_(regenerate)
   		{}

  		ObsEvent(const miutil::miTime &obstime,
	   			 StationInfoPtr stInfo_,
	    		 kvsynopd::synopcb_ptr cb)
    		:obstime_(obstime),
    		 ref(cb),
    		 stInfo(stInfo_),
    		 isOk_(false),
    		 regenerate_(false)
    	{}

  		ObsEvent(WaitingPtr w)
    		:obstime_(w->obstime()),
    		 ref(kvsynopd::synopcb::_nil()),
    		 stInfo(w->info()),
    		 waiting_(w),
    		 isOk_(false),
    		 regenerate_(false)
    	{}
  
  		miutil::miTime         obstime()const{ return obstime_;}
  		StationInfoPtr     stationInfo()const{ return stInfo;}
  		WaitingPtr             waiting()const{ return waiting_;}
  		kvsynopd::synopcb_ptr callback()const 
    			{ return kvsynopd::synopcb::_duplicate(ref);}
  
  		bool hasCallback()const{ return !CORBA::is_nil(ref);}
  		bool regenerate()const { return regenerate_;}

	    ///The following tree functions is used to comunicate 
	    ///data back to a client trough the callback \a ref.
	    ///The callback is only set when we get a explicit request 
	    ///to generate a synop.

  		std::ostringstream& msg() { return msg_;}
  		void              synop(const std::string &s){ synop_=s;}
 		std::string       synop()const { return synop_;}
  		void              isOk(bool f){ isOk_=f;}
  		bool              isOk()const { return isOk_;}

  		void              addTypeidReceived(long stationid, int typeid_){
    								for(std::list<IdType>::iterator it=typeidReceived.begin();
										 it!=typeidReceived.end(); 
										 it++){
      									if(it->sid_==stationid && it->tid_==typeid_)
												return;
    								}

    								typeidReceived.push_back(IdType(stationid,typeid_));
  								}

  		void clearTypeidReceived(){
    				typeidReceived.clear();
  				}

  		int nTypeidReceived()const{
    				return typeidReceived.size();
  				}

  		bool hasReceivedTypeid(int sid, int tid, bool doLogTypeidInfo)const{
    			std::ostringstream ost;

    			if(typeidReceived.empty()){

      			if(doLogTypeidInfo){
						LOGWARN("ObsEvent: TypeidReceived empty. Accept all data!");
      			}

      			return true;
    			}
    
    			if(doLogTypeidInfo){
      			ost << "ObsEvent: hasTypeidReceived (" << sid << "/" << tid << "):";
      			for(std::list<IdType>::const_iterator it=typeidReceived.begin();
	  					 it!=typeidReceived.end(); 
	  					 it++)
						ost << " (" << it->sid_ << "/" << it->tid_ << ")";
    			}

    			for(std::list<IdType>::const_iterator it=typeidReceived.begin();
					 it!=typeidReceived.end(); 
					 it++){
      			if(it->sid_==sid && it->tid_==tid){
						if(doLogTypeidInfo){
	  						ost << " Return: TRUE!";
	  						LOGDEBUG3(ost.str());
						}

						return true;
     				}
    			}
      
    			if(doLogTypeidInfo){
     				ost << " Return FALSE!";
     				LOGDEBUG3(ost.str());
    			}
    			
    			return false;
  			}

 		bool waitingOnContinuesData()const{
    				if(waiting_ && waiting_->waitingOnContinuesData())
      				return true;

    				return false;
  				}

  		//Do nothing
  		bool       executeImpl(){ return true;};
};

#endif
