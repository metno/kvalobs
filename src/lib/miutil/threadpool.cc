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


#include <atomic>
#include <iostream>
#include <sstream>
#include "lib/miutil/msleep.h"
#include "lib/miutil/threadpool.h"

namespace miutil {
namespace concurrent {

namespace {

volatile std::atomic_uint poolCounter(0);

void runner(ThreadPool *pool, Runable *run) {
  try {
    if (!run)
      return;
    pool->beforeExecute(run);
    run->run();
    pool->afterExecute(run);
  } catch (const std::exception &ex) {
    pool->log(ex.what());
  } catch (...) {
    pool->log("ThreadPool: Unexpected unknown exception ... ");
  }
}

void runnerProc(ThreadPool *pool, ThreadPool::RunQueue *queue) {
  bool stop = false;

  while (!stop) {
    try {
      Runable *toRun = queue->get();
      runner(pool, toRun);
    } catch (const miutil::concurrent::QueueSuspended &suspended) {
      stop = true;
    }
  }

  //  Drain the queue
  try {
    for (Runable *toRun = queue->getAfterSuspend(); toRun;
        toRun = queue->getAfterSuspend())
      runner(pool, toRun);
  } catch (const QueueIllegalState &ex) {
    pool->log("Unexpected QueueIllegalState");
  }
}
}  // namespace

ThreadPool::ThreadPool(int poolSize, const std::string &poolName)
    : size(poolSize),
      name(poolName) {
  setName_(poolName);
  init_(size);
}

ThreadPool::ThreadPool(const std::string &poolName)
    : size(0) {
  setName_(poolName);
}

ThreadPool::~ThreadPool() {
}

void ThreadPool::setName_(const std::string &name_) {
  if (name_.empty()) {
    std::ostringstream ost;
    ost << "pool-" << poolCounter.fetch_add(1);
    name = ost.str();
  }
  name = name_;
}

void ThreadPool::init_(int size_) {
  int n = size_ - pool.size();

  if (n > 0) {
    size = size_;
    runQueue.resize(2 * size);

    for (int i = 0; i < n; ++i)
      pool.push_back(std::thread(runnerProc, this, &runQueue));
  }
}

void ThreadPool::setName(const std::string &name_) {
  std::lock_guard<std::mutex> lock(mutex);
  setName_(name_);
}

void ThreadPool::init(int size_) {
  std::lock_guard<std::mutex> lock(mutex);
  init_(size_);
}

unsigned int ThreadPool::poolSize() const {
  std::lock_guard<std::mutex> lock(mutex);
  return pool.size();
}

void ThreadPool::afterExecute(miutil::Runable *r) {
  delete r;
}

void ThreadPool::beforeExecute(miutil::Runable *r) {
}

void ThreadPool::execute(miutil::Runable *r) {
  runQueue.add(r);
}

bool ThreadPool::execute(
    miutil::Runable *r,
    const std::chrono::high_resolution_clock::duration &timeout) {
  return runQueue.timedAdd(r, timeout);
}

void ThreadPool::shutdown() {
  runQueue.suspend();
}

bool ThreadPool::remove(Runable *task) {
  return runQueue.remove(task);
}
unsigned int ThreadPool::waitForTermination(
    const std::chrono::high_resolution_clock::duration &timeout) {
  typedef std::chrono::high_resolution_clock hr;
  hr::time_point now = hr::now();
  std::chrono::high_resolution_clock::time_point deadLine = now + timeout;
  runQueue.suspend();

  while (true) {
    Pool::iterator it = pool.begin();
    while (it != pool.end())
      if (it->joinable()) {
        it->join();
        it = pool.erase(it);
      } else {
        ++it;
      }

    now = hr::now();

    if (now > deadLine)
      return pool.size();
    else if (pool.size() == 0)
      return 0;

    miutil::msleep(10);
  }
}

void ThreadPool::log(const std::string &logMsg) {
  std::clog << "ThreadPool <" << name << ">: " << logMsg << std::endl;
}
}  // namespace concurrent
}  // namespace miutil
