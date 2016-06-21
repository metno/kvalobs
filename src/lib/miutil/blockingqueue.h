/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: readfile.h,v 1.1.2.2 2007/09/27 09:02:28 paule Exp $

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

#ifndef SRC_LIB_MIUTIL_BLOCKINGQUEUE_H_
#define SRC_LIB_MIUTIL_BLOCKINGQUEUE_H_

#include <chrono>
#include <condition_variable>
#include <deque>
#include <exception>
#include <limits>
#include <mutex>
#include "miutil/exceptionSpec.h"

namespace miutil {
namespace concurrent {

using std::chrono::milliseconds;
using std::cv_status;

EXCEPTION_SPEC_BASE(BlockingQueueException);
EXCEPTION_SPEC(BlockingQueueException, QueueSuspended, "QueueSuspended");
EXCEPTION_SPEC(BlockingQueueException, QueueTimeout, "QueueTimeout");
EXCEPTION_SPEC(BlockingQueueException, QueueIllegalState, "QueueIllegalState");

template<typename T>
class BlockingQueue {
 protected:
  typedef std::lock_guard<std::mutex> Lock;
  typedef std::deque<T> Queue;
  mutable std::mutex mutex;
  std::condition_variable_any cond;
  Queue dataQueue;
  bool suspended;
  unsigned int maxSize;

 public:
  BlockingQueue()
      : suspended(false),
        maxSize(std::numeric_limits<unsigned int>::max()) {
  }

  explicit BlockingQueue(unsigned int maxQueSize)
      : suspended(false),
        maxSize(maxQueSize) {
  }

  BlockingQueue(const BlockingQueue &que) {
    Lock lock(que.mutex);
    suspended = que.suspended;
    maxSize = que.maxSize;
    dataQueue = que.dataQueue;
  }

  void resize(unsigned int newSize = std::numeric_limits<unsigned int>::max()) {
    Lock lock(mutex);
    maxSize = newSize;
  }

  /**
   * If the maxQue size is specified,
   * wait until there is available space to
   * push the value.
   *
   * @throws QueueAuspended
   */
  void add(const T &value) {
    Lock lock(mutex);

    cond.wait(lock, [this] {return !dataQueue.empty() || suspended;});

    if (suspended)
      throw QueueSuspended();

    dataQueue.push_back(value);
    cond.notify_one();
  }

  /**
   * @throws QueueAuspended, QueueTimeout
   */
  bool timedAdd(const T &value,
                const std::chrono::high_resolution_clock::duration &timeout,
                bool throwTimeout = false) {
    Lock lock(mutex);

    if (!cond.wait_for(
        lock, timeout,
        [this] {return dataQueue.size() < maxSize || suspended;})) {
      if (throwTimeout)
        throw QueueTimeout();
      else
        return false;
    }

    if (suspended)
      throw QueueSuspended();

    dataQueue.push_back(value);
    cond.notify_one();

    return true;
  }

  /**
   * Wait until data is available.
   * @throws QueueAuspended
   */
  void get(T *value) {
    Lock lock(mutex);

    cond.wait(lock, [this] {return !dataQueue.empty() || suspended;});

    if (suspended)
      throw QueueSuspended();

    *value = dataQueue.front();
    dataQueue.pop_front();

    if (maxSize > 0)
      cond.notify_one();
  }

  /**
   * @throws QueueAuspended, QueueTimeout
   */
  bool timedGet(const std::chrono::high_resolution_clock::duration &timeout,
                T *value, bool throwTimeout = false) {
    Lock lock(mutex);

    if (!cond.wait_for(lock, timeout,
                       [this] {return dataQueue.empty() || suspended;})) {
      if (throwTimeout)
        throw QueueTimeout();
      else
        return false;
    }

    if (suspended)
      throw QueueSuspended();

    *value = dataQueue.front();
    dataQueue.pop_front();
    cond.notify_one();

    return true;
  }

  /**
   * @throws QueueIllegalState, if the queue is NOT suspended.
   */
  bool fetchAfterSuspend(T *value) {
    Lock lock(mutex);

    if (!suspended)
      throw QueueIllegalState();

    if (dataQueue.empty())
      return false;

    *value = dataQueue.front();
    dataQueue.pop_front();

    return true;
  }

  /**
   * @throws QueueAuspended
   */
  bool tryGet(T *value) {
    Lock lock(mutex);

    if (dataQueue.empty()) {
      if (suspended)
        throw QueueSuspended();
      else
        return false;
    }

    *value = dataQueue.front();
    dataQueue.pop_front();

    cond.notify_one();

    return true;
  }

  bool empty() const {
    Lock lock(mutex);
    return dataQueue.empty();
  }

  void suspend(bool clearQue = false) {
    Lock lock(mutex);

    if (suspended)
      return;

    suspended = true;

    if (clearQue)
      dataQueue.clear();

    cond.notify_all();
  }

  void resume() {
    Lock lock(mutex);

    if (!suspended)
      return;

    suspended = false;
    cond.notify_all();
  }
};

template<typename T>
class BlockingQueuePtr {
 protected:
  typedef std::unique_lock<std::mutex> Lock;
  typedef std::deque<T*> Queue;

  mutable std::mutex mutex;
  std::condition_variable cond;
  Queue dataQueue;
  bool suspended;
  unsigned int maxSize;

 public:
  BlockingQueuePtr()
      : suspended(false),
        maxSize(std::numeric_limits<unsigned int>::max()) {
  }

  explicit BlockingQueuePtr(unsigned int maxQueSize)
      : suspended(false),
        maxSize(maxQueSize) {
  }

  BlockingQueuePtr(const BlockingQueuePtr &que) {
    Lock lock(que.mutex);
    suspended = que.suspended;
    dataQueue = que.dataQueue;
    maxSize = que.maxSize;
  }

  void resize(unsigned int newSize = std::numeric_limits<unsigned int>::max()) {
    Lock lock(mutex);
    maxSize = newSize;
  }

  bool remove(T *value) {
    Lock lock(mutex);

    for (typename Queue::iterator it = dataQueue.begin(); it != dataQueue.end();
        ++it) {
      if (*it == value) {
        dataQueue.erase(it);
        cond.notify_all();
        return true;
      }
    }
    return false;
  }

  /**
   * If the maxQue size is specified,
   * wait until there is available space to
   * push the value.
   * @throws QueueAuspended
   */
  void add(T *value) {
    Lock lock(mutex);

    cond.wait(lock, [this] {return dataQueue.size() < maxSize || suspended;});

    if (suspended)
      throw QueueSuspended();

    dataQueue.push_back(value);
    cond.notify_one();
  }

  /**
   * @throws QueueSuspended, QueueTimeout
   */
  bool timedAdd(T *value,
                const std::chrono::high_resolution_clock::duration &timeout,
                bool throwTimeout = false) {
    Lock lock(mutex);

    if (suspended) {
      throw QueueSuspended();
    }

    if (!cond.wait_for(
        lock, timeout,
        [this] {return dataQueue.size() < maxSize || suspended;})) {
      if (throwTimeout)
        throw QueueTimeout();
      else
        return false;
    }

    if (suspended)
      throw QueueSuspended();

    dataQueue.push_back(value);
    cond.notify_one();

    return true;
  }

  /**
   * Wait until data is available.
   * @throws QueueAuspended
   */
  T*
  get() {
    Lock lock(mutex);

    cond.wait(lock, [this] {return !dataQueue.empty() || suspended;});

    if (suspended)
      throw QueueSuspended();

    T* res = dataQueue.front();
    dataQueue.pop_front();
    cond.notify_one();

    return res;
  }

  /**
   * @throws QueueAuspended, QueueTimeout
   */
  T*
  timedGet(const std::chrono::high_resolution_clock::duration &timeout,
           bool throwTimeout = false) {
    Lock lock(mutex);

    if (!cond.wait_for(lock, timeout,
                       [this] {return !dataQueue.empty() || suspended;})) {
      if (throwTimeout)
        throw QueueTimeout();
      else
        return nullptr;
    }

    if (suspended)
      throw QueueSuspended();

    T *res = dataQueue.front();
    dataQueue.pop_front();
    cond.notify_one();

    return res;
  }

  T*
  tryGet() {
    Lock lock(mutex);

    if (dataQueue.empty()) {
      if (suspended)
        throw QueueSuspended();
      else
        return nullptr;
    }

    T* res = dataQueue.front();
    dataQueue.pop_front();
    cond.notify_one();

    return res;
  }

  /**
   * @throws QueueIllegalState, if the queue is NOT suspended.
   */
  T*
  getAfterSuspend() {
    Lock lock(mutex);

    if (!suspended)
      throw QueueIllegalState();

    if (dataQueue.empty())
      return nullptr;

    T *res = dataQueue.front();
    dataQueue.pop_front();

    return res;
  }

  bool empty() const {
    Lock lock(mutex);
    return dataQueue.empty();
  }

  void suspend(bool clearQue = false, bool deleteElements = false) {
    Lock lock(mutex);

    if (suspended)
      return;

    suspended = true;

    if (clearQue) {
      if (deleteElements) {
        for (typename Queue::iterator it = dataQueue.begin();
            it != dataQueue.end(); ++it)
          delete *it;
      }

      dataQueue.clear();
    }

    cond.notify_all();
  }

  void resume() {
    Lock lock(mutex);

    if (!suspended)
      return;

    suspended = false;
    cond.notify_all();
  }
};
}  // namespace concurrent
}  // namespace miutil

#endif  // SRC_LIB_MIUTIL_BLOCKINGQUEUE_H_
