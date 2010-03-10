/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: StationInfo.h,v 1.13.2.8 2007/09/27 09:02:22 paule Exp $                                                       

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
/* $Header: /var/lib/cvs/kvalobs/src/kvsynopd/StationInfo.h,v 1.13.2.8 2007/09/27 09:02:22 paule Exp $ */

#ifndef __StationInfo_h__
#define __StationInfo_h__

#include <bitset>
#include <iostream>
#include <boost/shared_ptr.hpp>
#include <miconfparser/miconfparser.h>
#include <milog/milog.h>
#include <list>
#include <string>
#include <puTools/miTime.h>

//#include "StationInfoParse.h"


class StationInfoParse; 

/**
 * parseStationInfo, parse a stationinfo section in the
 * configuration file.
 * 
 * <pre>
 * Recognized keys:
 *   stationid [list]: stationid, may be a list of stationid if
 *                     the wmo message is created from data from
 *                     diffrent stationid.
 *   delay [list]: a list of delay specifications. A spesification
 *                 may have the following diffrent forms.
 *                 
 *                 tt:mm  - tt is an hour in the range [0,23].
 *                          mm is an minute in the range [0, 59]
 *                 Ftt:mm - tt is an hour in the range [0,23].
 *                          mm is an minute in the range [0, 59],
 *                          F - force.
 *                 SS:mm  - SS specify that  mm is for all
 *                          synoptimes, ie. 0,3,6,9,12,15,18 og 21
 *                 HH:mm  - HH specify that mm is for all hours.
 *                 FS:mm  - Force a delay for all synoptimes.
 *                 FH:mm  - Force a delay for all hours.
 *                 fS:mm  - Delay max mm minutes after the first data is 
 *                          received for all synoptimes.
 * 		           -SS    - Dont generate synop for synoptimes.
 *                 -tt    - Dont generate synop for the hour tt, where
 * 	                        tt is in the range [0,23].
 * 		
 *                 
 *                 ex: delay=("6:10", "SS:03")
 *                     This means that we shall delay with 3 minutes for all 
 *                     synoptimes except for the 6 termin that we shall delay
 *                     with 10 minutes.
 *
 *                     delay=("6:10", "SS:03", "HH:01")
 *                     This means that we shall delay all termins with 1 
 *                     minute, except for synoptimes (except the 6 termin 
 *                     that shal be delayed with 6 minutes) that shall delay
 *                     with  minutes.
 *                          
 *                     delay=("fS:03")
 *                     if the station is to receive data for multiple typeid's
 *                     delay the SYNOP production with 3 minutes after the 
 *                     first typeid is received. 
 * 
 *   precipitation [list]: Specify which parameter shall be used for
 *                         precipitation. Valid values "RA", "RR_01",
 *                         "RR_1", "RR_3", "RR_6","RR_12" og "RR_24".
 *                          
 *   typepriority [list]: A list of typeis's that is used to create a
 *                        wmo message.
 * </pre>
 */

class DelayInfo
{
  	//hour have several value:
  	// [0,23] the termin (hour) to delay.
  	//  -1 specify only synoptimes ie. 0, 3, 6, 9, 12, 15, 18, 21
  	//  -2 specify all hours
  
  	char hour_;  
  	char delay_; //minutes
  	bool force_;
  	bool *synoptimes_;  //Create synop for this times.

	void initSynopTimes(){
		
			//std::cerr << "initSynopTimes: " << int(hour_) << std::endl;
			if(!synoptimes_){
				try{
					synoptimes_=new bool[24];
				}
				catch(...){
					return;
				}
			}
		
			for(int i=0; i<24; i++)
				synoptimes_[i]=true;
	}

public:
  	enum {STIME=-1, HTIME=-2, FSTIME=-3, SKIP_SYNOP=127, UNDEF=-128};

  	DelayInfo(int hour=UNDEF)
    	:hour_(hour), delay_(0), force_(false), synoptimes_(0){
    		if(hour==SKIP_SYNOP){
    			//std::cerr << "DelayInfo::CTOR: SKIP_SYNOP!" << std::endl;
    			initSynopTimes();
    		}
    	}
  	DelayInfo(char hour, char delay, bool force)
    	: hour_(hour), delay_(delay), force_(force), synoptimes_(0){}
  	DelayInfo(const DelayInfo &d)
    	:hour_(d.hour_),delay_(d.delay_),force_(d.force_), synoptimes_(0){
    		if(d.synoptimes_){
   				initSynopTimes();
    				    		
    			if(synoptimes_){	
    				for(int i=0; i<24; i++)
    					synoptimes_[i]=d.synoptimes_[i];
    			}
    		}
    	}
    ~DelayInfo(){
    	if(synoptimes_)
    		delete synoptimes_;
    }

  	DelayInfo& operator=(const DelayInfo &rhs){
    	if(this!=&rhs){
      		hour_=rhs.hour_;
      		delay_=rhs.delay_;
      		force_=rhs.force_;
      
      		if(rhs.synoptimes_){
    			if(!synoptimes_)
    				initSynopTimes();
    				    		
    			if(synoptimes_){	
    				for(int i=0; i<24; i++)
    					synoptimes_[i]=rhs.synoptimes_[i];
    			}
    		}else if(synoptimes_){
    			delete synoptimes_;
    			synoptimes_=0;
      		}
    			
    	}
    	return *this;
  	}
  
  
  	bool operator==(const DelayInfo &di){
    	return equal(di);
  	}

  	bool operator==(const DelayInfo &di)const{
    	return equal(di);
  	}
  

  	bool equal(const DelayInfo &di)const{
			if(this==&di)
        		return true;
       
       		if(hour_==di.hour_   &&
	     	   delay_==di.delay_ &&
	     	   force_==di.force_){
	   
	   			if(synoptimes_ || di.synoptimes_){
	   				if(synoptimes_ && di.synoptimes_){
	     	   			for(int i=0; i<24; i++){
    						if(synoptimes_[i]!=di.synoptimes_[i])
    							return false;
	     	   			}
	   				}else{
	   					return false;
	   				}
	   			}
	     	   	   	   	
	    		return true;
	    	}else{
	    		return false;
	    	}
  	}

	bool skipSynopSpec()const{ return synoptimes_!=0;}
  	bool undef()const { return hour_==UNDEF;}
  	int  hour()const{ return static_cast<int>(hour_); }
  	int  delay()const{ return static_cast<int>(delay_);}
  	bool force()const{ return force_;}
  
  	//Shall we generate synop for this hour
  	bool synopForThisHour(int hour)const{
			if(!synoptimes_)
				return true;

  			if(hour<0 || hour>23)
  				return false;
  		
  			return synoptimes_[hour];
  	}
  
  	void synopForThisHour(int hour, bool flag){
  		//Must be a SKIP_SYNOP spec.
  		if(synoptimes_){
  			if(hour<0 || hour>23)
  				return;
  				  			
  			synoptimes_[hour]=flag;
  		}
  	}
  	
  	
  	friend std::ostream& operator<<(std::ostream& ost,
									  const DelayInfo& sd);
};

class StationInfo
{
 public:
 
 	/**
 	 * Type is a class to hold information about one typeid 
 	 * and for witch hours it shall be used and if it is a 
 	 * must have type, ie we cant create a SYNOP wuithout it.
 	 * 
 	 * We use a bitset to hold the time information, bit 0-23 is
 	 * for hour 00-23. Bit 24 is the must have information.
 	 *   
 	 */
 	class Type{
 			long typeid_;
 			std::bitset<25> hours;
 		
 		public:
 			Type():typeid_(0){
 				hours.set();
 				hours.set(24, false);
 			}
 	
 			Type(long t):typeid_(t){
 				hours.set();
 				hours.set(24, false);
 			}
 	
 			
 			Type(const Type &t):typeid_(t.typeid_), hours(t.hours){}

	  		Type& operator=(const Type &rhs){
	  			if(this!=&rhs){
	  				typeid_=rhs.typeid_;
	  				hours==rhs.hours;
	  			}
	  			
	  			return *this;
	  		}
 			
 			
 			bool operator==(const Type &t){
    			return equal(t);
  			}

  			bool operator==(const Type &t)const{
    			return equal(t);
  			}
  

  			bool equal(const Type &t)const{
				if(this!=&t){
					if(typeid_!=t.typeid_)
						return false;
						
					if(hours!=t.hours)
						return false;
						
					return true;
				}
				
				return false;
  			}	       
 			 			
 			long typeID()const{ return typeid_;}
 			bool hour(int h)const { return hours.test(h);}
 			void hour(int h, bool value){ hours.set(h, value);}
 			bool mustHaveType()const{ return hours.test(24);}
 			void mustHaveType(bool v){ hours.set(24, v);}
 			
 			friend std::ostream& operator<<(std::ostream& ost,
				  const StationInfo::Type& sd);
 	};
 	
 
 	typedef std::list<Type>                            TTypeList;
  	typedef std::list<Type>::iterator                 ITTypeList;
  	typedef std::list<Type>::const_iterator          CITTypeList;
  	typedef std::list<Type>::reverse_iterator        RITTypeList;
  	typedef std::list<Type>::const_reverse_iterator CRITTypeList;
 				
 
  typedef std::list<long>                   TLongList;
  typedef std::list<long>::iterator        ITLongList;
  typedef std::list<long>::const_iterator CITLongList;
  typedef std::list<long>::reverse_iterator        RITLongList;
  typedef std::list<long>::const_reverse_iterator CRITLongList;

  typedef std::list<std::string>                   TStringList;
  typedef std::list<std::string>::iterator        ITStringList;
  typedef std::list<std::string>::const_iterator CITStringList;

  typedef std::list<DelayInfo>                   TDelayList;
  typedef std::list<DelayInfo>::iterator        ITDelayList;
  typedef std::list<DelayInfo>::const_iterator CITDelayList;
  
 private:
  friend class StationInfoParse;

  StationInfo& operator=(const StationInfo &);
  
  int            wmono_;
  TLongList      stationid_;
  TTypeList      typepriority_;
//  TLongList      mustHaveTypes_;
  TStringList    precipitation_;
  TDelayList     delayList_;
  std::string    list_;
  bool           copy_;
  std::string    copyto_;
  std::string    owner_;
  miutil::miTime delayUntil_;
  //  bool           delayLogic_;
  static std::string  debugdir_;
  milog::LogLevel loglevel_;  
  bool            cacheReloaded48_;
  
  //  std::string    errorMsg;

  StationInfo();

 public:
  StationInfo(const StationInfo &);

  ~StationInfo();
  
  std::string    list()const   { return list_; }
  void           list(const std::string &l){ list_=l;}
  bool           copy()const   { return copy_; }
  void           copy(bool c)  { copy_=c;}
  std::string    copyto()const { return copyto_; }
  void           copyto(const std::string &c2){ copyto_=c2;}
  std::string    owner()const  { return owner_;}
  void           owner(const std::string &o){ owner_=o;}
  milog::LogLevel loglevel()const { return loglevel_;}


  int       wmono()const{ return wmono_;}
  TLongList stationID()const { return stationid_;}
  bool      hasStationId(long stid)const;

  static std::string debugdir() { return debugdir_;}

  TStringList precipitation()const { return precipitation_;}
  bool        hasPrecipitation()const{ return !precipitation_.empty();}

  /**
   * \brief A list of \em typeid to use data for when encoding SYNOP.
   * 
   * Typepriority plays two roles. Only data that is in this
   * list is used to create WMO message. The second use is
   * to select which Data that shall be used if there is more 
   * than one Data for one parameter. The Data is selected in the
   * order in the list. A Data with typeid preciding a another typeid
   * in the list is used before the other.
   *
   * \param hour Return the a list of typeid's that is valid for
   *             the hour \a hour. A negativ value means all typeids.
   * \return A list of \em typeid's.
   */ 
  TLongList typepriority(int hour=-1)const;


  /**
   * \brief return a list of typeid for this station that has
   *        an element in \a ctlist.
   */
  std::list<int> continuesTypeids(const std::list<int> &ctList)const;

  /**
   * \brief mustHaveTypes returns a list of \em typeid that there must be data 
   * for, before we can create a WMO message. 
   * 
   * We cant create a message if we doesnt have data from this types.
   * 
   * \param hour Return the a list of typeid's that is valid for
   *             the hour \a hour. A negativ value means all typeids.
   * \return A list of mustHaveTypes.
   */
  TLongList mustHaveTypes(int hour=-1)const;


  /**
   * \brief Do we have typeID in the list of typeriority_.
   *
   * \param hour Return the a list of typeis's that is valid for
   *             the hour \a hour. A negativ value mins all typeids.
   * \return true if \em typeID is in the list of types we shall
   *         use data from when encoding SYNOP. false othewise.
   */
  bool hasTypeId(int typeID, int hour=-1)const;
  
  /**
   *  \brief Setter function to set the delay.
   */
  void delayUntil(const miutil::miTime &delay){ delayUntil_=delay; }

  /**
   * \brief Getter function to get the delay.
   */
  miutil::miTime delayUntil()const { return delayUntil_;}
  

  /**
   * \brief Checks if we shall delay for the hour if not
   * all data is received that shall be used to create the 
   * WMO message. 
   *
   * It return true if we shall delay, and the number of
   * minutes to delay is returned in the output parameter 'minute'.
   * If we shall not delay it return false.
   *
   * \param[in] hour to check for a delay.
   * \param minute[out] Number of minutes to delay.
   * \param force[out] True if we shall allways delay, even if all
   *        data that is required.
   * \param relativToFirst[out] Is set to true on return if the delay
   *                            shall be relativ to the first typeid received
   *                            data for for a given synop time.
   * \return True if we shall delay and false otherwise.
   */
  bool delay(int hour, int &minute, bool &force, bool &relativToFirst)const;
  
  bool synopForTime(int hh)const;

  friend std::ostream& operator<<(std::ostream& ost,
				  const StationInfo& sd);

  std::string keyToString(const std::string &key);


  /**
   * \brief Compare two StationInfo to se if they has the same 
   *  configuration data.
   *
   *  The configuration data that is comapared is the information
   *  that is specified in the configuration file.
   * 
   *  \param st The StationInfo to compare against this.
   *  \return true if they are equal and false otherwise.
   */
  bool equalTo(const StationInfo &st);
  bool cacheReloaded48()const{ return cacheReloaded48_;}
  void cacheReloaded48(bool reloaded){ cacheReloaded48_=reloaded;}
};

typedef boost::shared_ptr<StationInfo> StationInfoPtr;

std::ostream& operator<<(std::ostream& ost,
			 const StationInfo& sd);

std::ostream& operator<<(std::ostream& ost,
			 const DelayInfo& sd);

std::ostream& operator<<(std::ostream& ost,
								  const StationInfo::Type& sd);
#endif
