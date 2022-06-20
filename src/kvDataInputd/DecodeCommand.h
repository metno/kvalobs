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
#ifndef SRC_KVDATAINPUTD_DECODECOMMAND_H_
#define SRC_KVDATAINPUTD_DECODECOMMAND_H_

#include <list>
#include <string>
#include "lib/miutil/runable.h"
#include "lib/miutil/blockingqueue.h"
#include "lib/decoder/decoderbase/decoder.h"
#include "lib/decoder/decoderbase/RedirectInfo.h"

/**
 * \addtogroup kvDatainputd
 * @{
 */

/**
 * \brief This is the message that is passed to the
 * work threads.
 */

class DecodeCommand : public miutil::Runable {
  DecodeCommand();
  DecodeCommand(const DecodeCommand &);
  DecodeCommand& operator=(const DecodeCommand &);

  kvalobs::decoder::DecoderBase *decoder;
  kvalobs::decoder::DecoderBase::DecodeResult result;
  std::string msg;
  miutil::concurrent::BlockingQueuePtr<DecodeCommand> resQue;
  kvalobs::decoder::RedirectInfo *redirect_;

  friend class DataSrcApp;

  kvalobs::decoder::DecoderBase* getDecoder() {
    return decoder;
  }

  ~DecodeCommand();
  explicit DecodeCommand(kvalobs::decoder::DecoderBase *decoder_);

 public:
  kvalobs::decoder::DecoderBase::DecodeResult getResult() const {
    return result;
  }

  std::string getMsg() const {
    return msg;
  }
  void setMsg(const std::string &m) {
    msg = m;
  }

  void setMessageId( const std::string &msgid );
  void setSerialNumber( unsigned long long serialNumber );
  void setProducer( const std::string &producer );
  std::string getProducer( )const;

  kvalobs::kvStationInfoList& getInfoList();

  std::list<kvalobs::serialize::KvalobsData>& getDecodedData();

  std::list<kvalobs::serialize::KvalobsData>& getPublishData();


  bool isOk() const {
    return result == kvalobs::decoder::DecoderBase::Ok;
  }
  /**
   * The caller must delete the pointer.
   */
  kvalobs::decoder::RedirectInfo *getRedirctInfo() {
    kvalobs::decoder::RedirectInfo *ret = redirect_;
    redirect_ = 0;
    return ret;
  }

  DecodeCommand *wait(int timeoutInMilliseconds = 0);
  void signal();
  void run();
};

/** @} */

#endif  // SRC_KVDATAINPUTD_DECODECOMMAND_H_
