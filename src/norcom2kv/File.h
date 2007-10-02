/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: File.h,v 1.2.6.3 2007/09/27 09:02:37 paule Exp $                                                       

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
#ifndef __File_Bm314_h__
#define __File_Bm314_h__

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>

class File{
  	std::string name_;
  	struct stat stat_;


 	public:
  	File(){};

  	File(const std::string &name):name_(name){
    	if(!reStat())
      		name_.erase();
  	}
    
  	File(const std::string &name, struct stat &stat):
    	name_(name), stat_(stat){
    }

  	File(const File &f):
    	name_(f.name_), stat_(f.stat_){
    }


  	File& operator=(const File &rhs){
      	if(this!=&rhs){
			name_=rhs.name_;
			stat_=rhs.stat_;
      	}

      	return *this;
    }

  	bool ok()const{ return !name_.empty();}

  	bool reStat(){ 
  		if(name_.empty())
	       	return false;
	     
	    if(stat(name_.c_str(), &stat_)<0)
		 	return false;
	    else
		 	return true;
	}
    
  	std::string basepart()const;
  	std::string namepart()const;
  
  	std::string name()const{ return name_;}
  
  	time_t      mtime()const{ return (name_.empty()?0:stat_.st_mtime);}
  	time_t      atime()const{ return (name_.empty()?0:stat_.st_atime);}
  	time_t      ctime()const{ return (name_.empty()?0:stat_.st_ctime);}
  	off_t       size()const { return (name_.empty()?0:stat_.st_size);}
  	uid_t       uid()const  { return (name_.empty()?0:stat_.st_uid);}
  	gid_t       gid()const  { return (name_.empty()?0:stat_.st_gid);}
  	mode_t      mode()const { return (name_.empty()?0:stat_.st_mode);}
  	nlink_t     nlink()const{ return (name_.empty()?0:stat_.st_nlink);}

  	bool       isFile()const    { return S_ISREG(mode());}
  	bool       isDir()const     { return S_ISDIR(mode());}
  	bool       isFifo()const    { return S_ISFIFO(mode());}
  	bool       isSymLink()const { return S_ISLNK(mode());}
  	bool       isSocket()const  { return S_ISSOCK(mode());}
};

#endif
