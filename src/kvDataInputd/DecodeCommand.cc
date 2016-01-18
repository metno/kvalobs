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
#include "boost/thread.hpp"
#include "lib/milog/milog.h"
#include "kvDataInputd/DecodeCommand.h"

namespace kvdatainput {
namespace decodecommand {
extern boost::thread_specific_ptr<kvalobs::decoder::RedirectInfo> ptrRedirect;
}
}

DecodeCommand::~DecodeCommand() {
  if (redirect_)
    delete redirect_;
}

DecodeCommand::DecodeCommand(kvalobs::decoder::DecoderBase *decoder_)
    : decoder(decoder_),
      redirect_(0) {
}

kvalobs::kvStationInfoList&
DecodeCommand::getInfoList() {
  return decoder->getStationList();
}

std::list<kvalobs::serialize::KvalobsData>&
DecodeCommand::getDecodedData() {
  return decoder->getDecodedData();
}

void DecodeCommand::run() {
  LOGDEBUG("DecodeCommand::execute: called!\n");

  try {
    kvdatainput::decodecommand::ptrRedirect.reset(new kvalobs::decoder::RedirectInfo());
    result = decoder->execute(msg);
    redirect_ = kvdatainput::decodecommand::ptrRedirect.release();
    if (result != kvalobs::decoder::DecoderBase::Redirect) {
      delete redirect_;
      redirect_ = 0;
    }
    LOGDEBUG("DecodeCommand::execute: Redirect: " << (redirect_?"redirected":"NOT redirected")<< " Result: " << result <<"\n");
  } catch (const std::exception &ex) {
    LOGERROR("UNEXPECTED EXCEPTION from <"<< decoder->name() << ">\n Reason: " << ex.what());
    result = kvalobs::decoder::DecoderBase::Error;
  } catch (...) {
    LOGERROR("UNEXPECTED EXCEPTION from <"<< decoder->name() << ">\n Reason: Unknown");
    result = kvalobs::decoder::DecoderBase::Error;
  }

  LOGDEBUG("DecodeCommand::execute: return!\n");
}

DecodeCommand*
DecodeCommand::wait(int timeoutInSecond) {
  try {
    return resQue.timedGet(std::chrono::milliseconds(timeoutInSecond));
  } catch (const miutil::concurrent::QueueSuspended &ex) {
    LOGERROR("DecodeCommand::wait: Result que suspended.\n");
  } catch (...) {
    LOGERROR("DecodeCommand::wait: Unexpected exception!\n");
  }

  return 0;
}

void DecodeCommand::signal() {
  resQue.add(this);
}
