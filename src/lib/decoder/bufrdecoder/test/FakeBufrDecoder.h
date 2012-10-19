/*
  Kvalobs - Free Quality Control Software for Meteorological Observations

  $Id: comobsentry.cc,v 1.1.2.1 2007/09/27 09:02:24 paule Exp $

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

#ifndef __FAKEBUFRDECODER_H__
#define __FAKEBUFRDECODER_H__

#include <decoder/decoderbase/decoder.h>
#include "../bufrdecoder.h"



class FakeBufrDecoder : public kvalobs::decoder::bufr::BufrDecoder
{
   std::string saSdEm;
public:
   FakeBufrDecoder( dnmi::db::Connection     &con,
                    const ParamList        &params,
                    const std::list<kvalobs::kvTypes> &typeList,
                    const miutil::miString &obsType,
                    const miutil::miString &obs,
                    int                    decoderId=-1);

   long getStationid( int wmono )const;
   bool getEarlyLateObs( int &early, int &late )const;

   void setSaSdEm(const std::string saSdEm_ ){ saSdEm = saSdEm_; }
   std::string getMetaSaSdEm( int stationid, int typeid_, const miutil::miTime &obstime );

   virtual miutil::miString name()const;

   virtual kvalobs::decoder::DecoderBase::DecodeResult execute(miutil::miString &msg);

};


#endif
