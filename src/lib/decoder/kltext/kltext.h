/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: dummydecoder.h,v 1.1.2.2 2007/09/27 09:02:28 paule Exp $

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
#ifndef SRC_LIB_DECODER_KLTEXT_KLTEXT_H_
#define SRC_LIB_DECODER_KLTEXT_KLTEXT_H_

#include <functional>
#include <string>
#include "boost/date_time/posix_time/ptime.hpp"
#include "lib/decoder/decoderbase/decoder.h"

namespace kvalobs {
namespace decoder {
namespace kltextdecoder {

/**
 * \addtogroup kltextdecoder
 *
 * @{
 */

typedef boost::mutex::scoped_lock Lock;

class KlText {
  KlText();
  KlText(const KlText &);
  KlText& operator=(const KlText &);

  int typeID;
  int stationID;
  std::string stationidIn;
  boost::posix_time::ptime receivedTime;
  std::string redirectedFrom;
  std::string obsType;
  std::string obs;
  boost::posix_time::ptime tbTime;  // Used for test
  std::function<int(const std::string &key, const std::string &val)> getStationId;

  void decodeObsType(const std::string &obstype);

 public:
  /**
   * getStatioId_ takes a key nationalnr, stationid or wmonr and find the stationid that identifies
   * the station.
   */
  explicit KlText(std::function<int(const std::string &key, const std::string &val)> getStationId_);

  ~KlText();

  kvalobs::decoder::DecoderBase::DecodeResult execute(const std::string &obsType_, const std::string &obs_, std::string &msg, kvalobs::kvTextData *textData);
};

/** @} */
}  // namespace kltextdecoder
}  // namespace decoder
}  // namespace kvalobs

#endif  // SRC_LIB_DECODER_KLTEXT_KLTEXT_H_
