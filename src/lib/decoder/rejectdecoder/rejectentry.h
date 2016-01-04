/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: rejectentry.h,v 1.1.2.2 2007/09/27 09:02:33 paule Exp $                                                       

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
#ifndef __rejectentry_h__
#define __rejectentry_h__

#ifdef __cplusplus
#include <decoderbase/decoder.h>
#include <kvalobs/kvTypes.h>
#include <list>

/**
 * \defgroup rejectdecode RejectDecoder is a decoder to receive
 *           messages that is rejected before they comes to kvalobs. They 
 *           are rejected by systems that delivers data to kvalobs. They
 *           can use this decoder to deliver the rejected message to kvalobs,
 *          so it can be put in the rejectdecode table. 
 * \ingroup rejectdecode
 *
 * @{ 
 */

extern "C" {
kvalobs::decoder::DecoderBase*
decoderFactory(dnmi::db::Connection &con_, const ParamList &params,
               const std::list<kvalobs::kvTypes> &typeList, int decoderId_,
               const std::string &observationType_,
               const std::string &observation_);

void releaseDecoder(kvalobs::decoder::DecoderBase* decoder);
void setKvConf(kvalobs::decoder::DecoderBase* decoder,
               miutil::conf::ConfSection *theKvConf);

std::list<std::string> getObsTypes();
}

/** @} */
#endif

#endif
