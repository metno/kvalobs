/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: PThread.cc,v 1.4.6.2 2007/09/27 09:02:32 paule Exp $                                                       

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
#include <iostream>
#include <milog/thread/PThread.h>
#include <map>
#include <assert.h>

using namespace std;

namespace milog{
  namespace thread {
    namespace PThread{
      typedef std::map<std::string, pthread_key_t>                   TKeyMap; 
      typedef std::map<std::string, pthread_key_t>::iterator        ITKeyMap; 
      typedef std::map<std::string, pthread_key_t>::const_iterator CITKeyMap; 
      
      TKeyMap keyMap;
      Mutex   keyMapMutex;
      
      bool
      getKey(const std::string &id, 
	     pthread_key_t &key, 
	     void (*destr_function) (void *))
      {
	ScopedLock l(keyMapMutex);

	ITKeyMap it=keyMap.find(id);
	
	//cerr << "PThread::keyMap::size: " << keyMap.size() << endl;

	if(it!=keyMap.end()){
	  //err << "getKey: returns key: " << it->second << "\n";
	  key=it->second;
	  return true;
	}else{
	  pthread_key_t key_;
	  //cerr << "PThread::getKey: create key!\n";

	  if(::pthread_key_create(&key_, destr_function)==0){
	    keyMap[id]=key_;
	    //cerr << "getKey: NEW KEY, returns key: " << key_ << "\n";
	    key=key_;
	    
	    return true;
	  }else{
	    cerr << "milog::getKey: Cant create key!!!\n";
	    return false;
	    
	  }
	}
      }
      
    }
  }
}
