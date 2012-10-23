/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: synopdecoder.h,v 1.7.6.3 2007/09/27 09:02:18 paule Exp $                                                       

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
#ifndef __kvalobs_decoder_bufrdecoder_h__
#define __kvalobs_decoder_bufrdecoder_h__

#include <boost/shared_ptr.hpp>
#include <kvalobs/kvData.h>
#include <list>
#include <kvalobs/kvStation.h>
#include <decoderbase/decoder.h>
#include "BufrDecodeKvResult.h"

namespace kvalobs{
namespace decoder{
namespace bufr{

typedef boost::mutex::scoped_lock    Lock;


class DecodeKvResult : public BufrDecodeKvResult
{
public:
   bool saveData( BufrDecoder *decoder );
};


class BufrDecoder : public DecoderBase{
   BufrDecoder();
   BufrDecoder(const BufrDecoder &);
   BufrDecoder& operator=(const BufrDecoder &);

   static miutil::miTime lastStationCheck;
   static boost::mutex   mutex;
   boost::shared_ptr< std::list<kvalobs::kvStation> > stationList;
   int earlyobs;
   int lateobs;

   long getStationId(miutil::std::string &msg);
   bool initialize();


public:
   BufrDecoder( dnmi::db::Connection     &con,
                const ParamList        &params,
                const std::list<kvalobs::kvTypes> &typeList,
                const miutil::std::string &obsType,
                const miutil::std::string &obs,
                int                    decoderId=-1);
   virtual ~BufrDecoder();

   bool saveData(std::list<kvalobs::kvData> &data,
                   bool &rejected,
                   std::string &rejectedMessage);

   void splitBufr( const std::string &bufr, std::list<std::string> &bufrs );

   long getStationid( int wmono )const;
   std::string getFormat()const;

   bool getEarlyLateObs( int &early, int &late )const;


   virtual miutil::std::string name()const;

   virtual DecodeResult execute(miutil::std::string &msg);
};
}
}
}

#endif
