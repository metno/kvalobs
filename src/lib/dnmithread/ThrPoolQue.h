/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: ThrPoolQue.h,v 1.1.2.2 2007/09/27 09:02:33 paule Exp $                                                       

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
#ifndef __ThrPoolQue_H__
#define __ThrPoolQue_H__

#include <boost/thread/thread.hpp>
#include <dnmithread/CommandQue.h>
#include <dnmithread/mtcout.h>

namespace dnmi {
namespace thread {
/**
 * \addtogroup threadutil
 * @{
 */

typedef bool (*threadHelperFunc)(CommandBase *cmd);
typedef bool (*threadKillFunc)();

/**
 * \brief A class that can be used to implement a thread pool.
 *
 * The class listen on a que and select a thread in the pool to 
 * execute the command.
 */
class ThreadPoolQue : boost::noncopyable {
  boost::thread_group thrGroup;
  CommandQue &que;
  int size;
  bool doKill;
  threadHelperFunc beforeFunc;
  threadHelperFunc afterFunc;
  threadKillFunc killFunc;
  int timeout;
  boost::mutex mutex;

 public:

  /**
   * \brief This class implements a simple threadpool.
   *
   * The command to be executed is received from a que.
   * Before each command is proccessed the beforeFunc is called, the
   * default befireFunc does nothing.
   * After a command is proccessed the afterFunc is called. The default
   * afterFunc deletes the command.  
   *
   * There can be a killFunc that can be calle if needed, see the 
   * getKillFunc method.
   *
   * When the threads in the pool is terminating the que is suspended.
   *
   * The constructor takes three parameters the number of threads in the
   * pool, the que to receive commands from and the timeout we shall
   * use to wait on the que before we call the killFunc to check if we 
   * shall terminate.
   */

  explicit ThreadPoolQue(unsigned int size_, CommandQue &q,
                         int timeoutInSec = 0);

  /**
   * You can set a function that returns true when
   * the thread shall return. ie it can be used as a
   * way to delever the status of some global signal
   * flags that hints that we want the threads in the 
   * pool to return (stop). The default action is do nothing!
   */
  threadKillFunc getKillFunc() const {
    return killFunc;
  }

  /**
   * beforeFunc get called before CommandBase.execute().
   * The default function does nothing. If you define your
   * own function remeber that it can not hold static data or
   * global data. If it does, it must be protected with a mutex.
   * The function is run in the context of the executing thread.
   */
  threadHelperFunc getBeforeFunc() const {
    return beforeFunc;
  }

  /**
   * afterFunc get called after CommandBase.execute().
   * The default function deletes the CommandBase that
   * it got from the CommandQue. If you define your
   * own function remeber that it can not hold static data or
   * global data. If it does, it must be protected with a mutex. The
   * function is run in the context of the executing thread.
   */
  threadHelperFunc getAfterFunc() const {
    return afterFunc;
  }

  int getTimeout() const {
    return timeout;
  }
  void setKillFunc(threadKillFunc func) {
    killFunc = func;
  }
  void setBeforeFunc(threadHelperFunc func) {
    beforeFunc = func;
  }
  void setAfterFunc(threadHelperFunc func) {
    afterFunc = func;
  }
  void run();
  bool killSignal();
  void join();
  bool getKillHint() const {
    return doKill;
  }
};

/** @} */
}
;

}
;
#endif
