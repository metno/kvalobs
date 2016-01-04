/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: DummyThread.h,v 1.1.2.2 2007/09/27 09:02:31 paule Exp $                                                       

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
#ifndef __milog_thread_DummyThread_h__
#define __milog_thread_DummyThread_h__

#include <stdio.h>
#include <string>

namespace milog {
namespace thread {
//  long getThreadId(){ return 1;};

/**
 Dummy type 'int' for Mutex. Yes, this adds a bit of overhead in
 the for of extra memory, but unfortunately 'void' is illegal.
 **/
typedef int Mutex;

/**
 Dummy type 'int' defintion of ScopedLock;
 **/

typedef int ScopedLock;

template<typename T> class ThreadLocalDataHolder {
 public:
  typedef T data_type;

  inline ThreadLocalDataHolder(const std::string &dummyId)
      : _data(0) {
  }
  ;
  inline ~ThreadLocalDataHolder() {
    if (_data)
      delete _data;
  }
  ;

  inline T* get() const {
    return _data;
  }
  ;

  inline T* operator->() const {
    return get();
  }
  ;
  inline T& operator*() const {
    return *get();
  }
  ;

  inline T* release() {
    T* result = _data;
    _data = 0;

    return result;
  }
  ;

  inline void reset(T* p = NULL) {
    if (_data)
      delete _data;
    _data = p;
  }
  ;

 private:
  T* _data;
};
}
}
#endif
