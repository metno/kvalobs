/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: PThread.h,v 1.1.2.2 2007/09/27 09:02:31 paule Exp $                                                       

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
#ifndef __milog_thread_PThread_h__
#define __milog_thread_PThread_h__

#include <stdio.h>
#include <string>
#include <pthread.h>

namespace milog {
  namespace thread {

    namespace  PThread{     
      bool getKey(const std::string &id, 
		  pthread_key_t &key, 
		  void (*destr_function) (void *));
    }
    
    class Mutex{
      Mutex(const Mutex &);
      Mutex& operator=(const Mutex &);
 
       pthread_mutex_t m;

    public:
      Mutex(){
	pthread_mutex_init(&m, 0);
      }
      
      ~Mutex(){
	pthread_mutex_destroy(&m);
      }
      
      void lock(){
	pthread_mutex_lock(&m);
      }
      
      void unlock(){
	pthread_mutex_unlock(&m);
      }
    };
    
    class ScopedLock{
      Mutex &m;
      
    public:
      ScopedLock(Mutex &m_):m(m_){
	m.lock();
      }
	
      ~ScopedLock(){
	m.unlock();
      }
    };
    
    template<typename T> class ThreadLocalDataHolder {
      std::string keyid;
    public:
      ThreadLocalDataHolder(const std::string &id):keyid(id){};
      ~ThreadLocalDataHolder() {
      };
      
      T* get() const {
	pthread_key_t key;
	T             *p;

	if(PThread::getKey(keyid.c_str(), key, destroy)){
	  p=static_cast<T*>(pthread_getspecific(key));
	  return p;
	}else{
	  return 0;
	}
	    
      };
      
      T* operator->() const { return get(); };
      T& operator*() const { return *get(); };
      
      T* release() {
	T             *p=get();
	
	if(p){
	  reset();
	}
	
	return p;
      };
      
      void reset(T* p=0) {
	pthread_key_t key;
	T   *old=get();
	
	if(p==old){
	  return;
	}
	
	if(old)
	  delete old;

	if(PThread::getKey(keyid.c_str(), key, destroy)){
	  pthread_setspecific(key, static_cast<void*>(p));
	}
      };
      
    private:
      static void destroy(void *p){ delete static_cast<T*>(p);}
    };
  }
}
#endif
