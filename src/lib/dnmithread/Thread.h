/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: Thread.h,v 1.1.2.2 2007/09/27 09:02:33 paule Exp $                                                       

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
#ifndef __dnmi_thread_Thread_h__
#define __dnmi_thread_Thread_h__

//#define DNMI_THREAD_DEBUG

#include <limits.h>
#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>

#ifdef DNMI_THREAD_DEBUG
#include <dnmithread/mtcout.h>
#endif

namespace dnmi {
namespace thread {
/**
 * \addtogroup threadutil
 * @{
 */

/**
 * \brief A base class that can be used for new threads.
 */

class Runable {
 public:

  /**
   * @brief The function to run in a thread.
   *
   * This function must be implemeneted by a class that
   * is to be used as a thread.
   */
  virtual int run()=0;
};

template<typename T>
class Thread {
  boost::shared_ptr<bool> join_;
  boost::shared_ptr<boost::thread> thread_;
  boost::shared_ptr<T> t_;
  boost::shared_ptr<int> ret_;

#ifdef DNMI_THREAD_DEBUG
  boost::shared_ptr<int> count_;
#endif

 public:
  Thread(const Thread &s)
      : join_(s.join_),
        thread_(s.thread_),
        t_(s.t_),
        ret_(s.ret_)
#ifdef DNMI_THREAD_DEBUG
  , count_(s.count_)
#endif
  {
#ifdef DNMI_THREAD_DEBUG
    (*count_)++;
    CERR("Thread:: CTOR (copy) count=" << *count_<<
        " use_count: " << join_.use_count() << ", " <<
        thread_.use_count() << ", " << count_.use_count() <<std::endl);
#endif
  }

  Thread(T *t = 0)
      : join_(new bool(false)),
        t_(t),
        ret_(new int)
#ifdef DNMI_THREAD_DEBUG
  , count_(new int(1))
#endif
  {
#ifdef DNMI_THREAD_DEBUG
    CERR("Thread:: CTOR (def) count=" << *count_<<
        " use_count: " << join_.use_count() << ", " <<
        thread_.use_count() << ", " << count_.use_count() <<std::endl);
#endif
  }

  virtual ~Thread() {
#ifdef DNMI_THREAD_DEBUG
    (*count_)--;
    CERR("Thread:: DTOR count=" << *count_<<
        " use_count: " << join_.use_count() << ", " <<
        thread_.use_count() << ", " << count_.use_count() <<std::endl);
#endif
  }

  Thread& operator=(const Thread &rhs) {
    if (&rhs == this)
      return *this;

    join_ = rhs.join_;
    thread_ = rhs.thread_;
    t_ = rhs.t_;
    ;
    ret_ = rhs.ret_;

#ifdef DNMI_THREAD_DEBUG
    count_=rhs.count_;
#endif
    return *this;
  }

  /**
   * @brief PART OF THE PRIVATE IMPLEMENTATION
   *        DONT OVERIDE!!!!!
   */
  void operator()() {
#ifdef DNMI_THREAD_DEBUG
    CERR("Thread::operator() count=" << *count_<<
        " use_count: " << join_.use_count() << ", " <<
        thread_.use_count() << ", " << count_.use_count() <<std::endl);
#endif
    try {
      *ret_ = t_->run();
    } catch (...) {
      *ret_ = INT_MIN;
      //Dont let uncaught exception leak out!
    }

    *join_ = true;
  }

  /**
   * @brief Join this thread if it is terminated!
   *
   * @param[out] ret The return value from Runable::run.
   * @return true The thread is terminated and joined!
   *         false The thread is running and is not joined.
   */
  bool join(int &ret, bool wait = false) {
    ret = INT_MAX;
    if (wait) {
      thread_->join();
      ret = *ret_;
#ifdef DNMI_THREAD_DEBUG
      CERR("Thread::Join: use_count: " << join_.use_count() << ", " <<
          thread_.use_count() << ", " << count_.use_count() <<std::endl);
#endif
      return true;
    }

    if (!*join_)
      return false;

    thread_->join();
#ifdef DNMI_THREAD_DEBUG
    CERR("Thread::join: use_count: " << join_.use_count() << ", " <<
        thread_.use_count() << ", " << count_.use_count() <<std::endl);
#endif
    ret = *ret_;
    return true;
  }

  bool join(bool wait = false) {
    int dummy;

    return join(dummy, wait);
  }

  /**
   * @brief start the thread. Ie, start the run function as in
   *        its own thread.
   *
   * @return true on success and false if it was not posible
   *         to start a new thread.
   */
  bool start() {
    boost::thread *th;

    if (t_.get() == 0)
      return false;

    if (thread_.get() != 0)
      return false;

    try {
      th = new boost::thread(*this);
    } catch (...) {
      return false;
    }
#ifdef DNMI_THREAD_DEBUG
    CERR("Thread::start count=" << *count_<<std::endl);
#endif
    thread_.reset(th);
    return true;
  }

  T* get() {
    return t_.get();
  }

};
}
;
}
;

#endif
