/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

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

#ifndef SRC_KVDATAINPUTD_DECODEREXECUTOR_H_
#define SRC_KVDATAINPUTD_DECODEREXECUTOR_H_

#include <miutil/threadpool.h>

class DecoderExecutor : public miutil::concurrent::ThreadPool {
  std::string logid;
 public:
  explicit DecoderExecutor(int size, const std::string &logid_="thread_pool")
      : ThreadPool(size, "decoder"), logid(logid_) {
  }

  DecoderExecutor()
      : ThreadPool("decoder"), logid("thread_pool") {
  }

  void afterExecute(miutil::Runable *run);
  void log(const std::string &logMsg);
};

#endif /* SRC_KVDATAINPUTD_DECODEREXECUTOR_H_ */
