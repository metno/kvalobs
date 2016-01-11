/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: miTimeParse.h,v 1.1.2.2 2007/09/27 09:02:32 paule Exp $

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


#ifndef SRC_LIB_MIUTIL_THREADPOOL_H_
#define SRC_LIB_MIUTIL_THREADPOOL_H_


#include <chrono>  // NOLINT(build/c++11)
#include <string>
#include <thread>  // NOLINT(build/c++11)
#include <vector>
#include "miutil/blockingqueue.h"
#include "miutil/runable.h"

namespace miutil {
namespace concurrent {

class ThreadPool {
 public:
  typedef miutil::concurrent::BlockingQueuePtr<Runable> RunQueue;

 private:
  ThreadPool();
  ThreadPool(const ThreadPool&);
  ThreadPool& operator=(const ThreadPool &rhs);

  typedef std::vector<std::thread> Pool;
  mutable std::mutex mutex;
  int size;
  RunQueue runQueue;
  Pool pool;

  void init_(int poolSize);
  void setName_(const std::string &name = std::string());

 protected:
  std::string name;

 public:
  explicit ThreadPool(int size, const std::string &poolName = "");
  explicit ThreadPool(const std::string &poolName = "");
  virtual ~ThreadPool();

  void init(int poolSize);
  void setName(const std::string &name = std::string());
  unsigned int poolSize() const;
  std::string getName() const {
    return name;
  }
  virtual void afterExecute(miutil::Runable *r);
  virtual void beforeExecute(miutil::Runable *r);
  void execute(miutil::Runable *r);
  bool execute(miutil::Runable *r,
               const std::chrono::high_resolution_clock::duration &timeout);
  void shutdown();

  bool remove(Runable *task);
  unsigned int waitForTermination(
      const std::chrono::high_resolution_clock::duration &timeout =
          std::chrono::high_resolution_clock::duration::max());
  void log(const std::string &logMsg);
};

}  // namespace concurrent
}  // namespace miutil

#endif  // SRC_LIB_MIUTIL_THREADPOOL_H_
