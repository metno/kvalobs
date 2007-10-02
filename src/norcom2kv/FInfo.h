/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: FInfo.h,v 1.3.6.5 2007/09/27 09:02:37 paule Exp $                                                       

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
#ifndef __FInfo_h__
#define __FInfo_h__

#include <time.h>
#include <map>
#include <string>
#include <exception>
#include <unistd.h>
#include "File.h"

class FInfo{
  	File          file_;
  	time_t        mtime_;
  	long          offset_;
  	unsigned int crc_;
  	bool          collected_;
  	bool          seen_; //Have the file been seen before with the collected
                      //flag set to false.
  	std::string  fcopy;

 	public:

  	class StatException : public std::exception{
    	std::string reason;
  	public:
    	explicit StatException(const std::string &reason_)
      		: reason(reason_){}
    
    	virtual ~StatException()throw(){};
    
    	const char *what()const throw(){ return reason.c_str();}
  	};
      

  	FInfo():
    	mtime_(0), offset_(0), crc_(0), collected_(false), seen_(false){}

  	FInfo(const File &f, long offset=0, 
		  unsigned int           crc=0, 
		  bool              collected=false,
		  bool                   seen=false):
    	file_(f), 
    	mtime_(f.mtime()),
    	offset_(offset), 
    	crc_(crc),
    	collected_(collected),
    	seen_(seen){}
 

  	FInfo(const FInfo& f):
    	file_(f.file_), offset_(f.offset_), crc_(f.crc_), 
    	collected_(f.collected_), seen_(f.seen_){}
  
  	FInfo& operator=(const FInfo &rhs){
      	if(this!=&rhs){
			file_     =rhs.file_;
			mtime_    =rhs.mtime_;
			offset_   =rhs.offset_;
			crc_      =rhs.crc_;
			collected_=rhs.collected_;
			seen_     =rhs.seen_;
      	}

      	return *this;
    }
  

  	bool   ok() const { return file_.ok();}
  
  	bool   seen()const{ return seen_;}
  	void   seen(bool flag){ seen_=flag;}
  	void   incSeen(){   seen_++;}

  	bool   collected()const{ return collected_;}
  	void   collected(bool c){ collected_=c;}

  	std::string copy()const{ return fcopy;}
  	void  copy(const std::string &copy_){ fcopy=copy_; }
  	void  removecopy(bool removefile){ 
    			if(!fcopy.empty()){
    				 if(removefile)
		   				unlink(fcopy.c_str());
		   			fcopy.erase();
		  		}
          }
          
  	bool   toBeCollected(){ return !collected_ && seen_ && !fcopy.empty();}
  
  	time_t mtime()const { return mtime_;}
  
  	/** Do a stat request on the file!
   	 * 
   	 * \exception StatException if the file could not be stated.
   	 */
 	 void   mtimeNow() { 
                 if(!file_.reStat()) 
		  			 throw StatException("StatExcetion");
		 		
		 		 mtime_=file_.mtime();
	     	}

  	long   offset()const{return offset_;}
  	void   offset(long o){ offset_=o;}
  
  	unsigned int crc()const { return crc_;}
  	void          crc(unsigned int c){ crc_=c;}

  	std::string  name()const{ return file_.name();}
  	std::string  basepart()const { return file_.basepart();}
  	std::string  namepart()const { return file_.namepart();}
};

typedef std::map<std::string, FInfo>                   FInfoList;  
typedef std::map<std::string, FInfo>::iterator        IFInfoList;  
typedef std::map<std::string, FInfo>::const_iterator CIFInfoList;  




#endif
