/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: sms12.h,v 1.2.2.2 2007/09/27 09:02:24 paule Exp $

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

#ifndef __kvalobs_decoder_comobsdecoder_sms13_h__
#define __kvalobs_decoder_comobsdecoder_sms13_h__

#include <ostream>
#include <miutil/commastring.h>
#include "smsbase.h"

namespace kvalobs {
namespace decoder {
namespace comobsdecoder {

/**
 * \addtogroup comobsdecode
 *
 * @{
 */

class Sms13: public SmsBase {
    Sms13( const Sms13 & );
    Sms13& operator=( const Sms13 & );

public:
    Sms13( const ParamList &paramList,
            kvalobs::decoder::comobsdecoder::ComObsDecoder &decoder );
    ~Sms13();

    kvalobs::decodeutil::DecodedData*
    decode( long stationid, int smscode, const SmsMelding::MeldingList &obs,
            std::string &returnMessage,
            std::list<kvalobs::kvRejectdecode> &rejected, bool &hasRejected );

};

}
}
}

#endif
