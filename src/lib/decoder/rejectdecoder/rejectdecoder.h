/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: rejectdecoder.h,v 1.1.2.2 2007/09/27 09:02:32 paule Exp $                                                       

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
#ifndef __kvalobs_decoder_rejectdecoder_h__
#define __kvalobs_decoder_rejectdecoder_h__

#include <decoderbase/decoder.h>
#include <kvalobs/kvRejectdecode.h>

namespace kvalobs {
namespace decoder {
namespace rejectdecoder {

/**
 * \addtogroup rejectdecode
 *
 * @{
 */

typedef boost::mutex::scoped_lock Lock;

/**
 * \brief implements the interface  DecoderBase.
 *
 * This is a decoder for rejected messages.
 */
class RejectDecoder : public DecoderBase {
  RejectDecoder();
  RejectDecoder(const RejectDecoder &);
  RejectDecoder& operator=(const RejectDecoder &);

 protected:
  bool doDecode(const std::string &message, kvalobs::kvRejectdecode &reject);

 public:
  RejectDecoder(dnmi::db::Connection &con, const ParamList &params,
                const std::list<kvalobs::kvTypes> &typeList,
                const std::string &obsType, const std::string &obs,
                int decoderId = -1);

  virtual ~RejectDecoder();

  virtual std::string name() const;

  virtual DecodeResult execute(std::string &msg);
};

/** @} */
}
}
}

#endif
