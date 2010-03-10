/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvMetarDecoder.h,v 1.2.2.2 2007/09/27 09:02:37 paule Exp $                                                       

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
#ifndef _kvMetarDecoder_h
#define _kvMetarDecoder_h

#include <map>
#include <list>

#include <kvalobs/kvStation.h>
#include <kvalobs/kvData.h>
#include <kvalobs/kvRejectdecode.h>
#include <kvalobs/kvTextData.h>

#include "kvMetar.h"
#include <iostream>
#include <puTools/miString.h>

/// Created by DNMI/PU: j.schulze@met.no
/// at Wed Apr  2 08:34:13 2003 

/// kvalobs metar decoder, using selv decoding elements
/// kvMetar, which are using the TIPS metar decoding
/// internal

class kvMetarDecoder {
private:

  kvMetar          obs;                 ///< the resulting observation
  miutil::miTime   ref;                 ///< referencetime  
  bool             continuous;          ///< if true -> ref = now (default)
  miutil::miTime   tbt;
  miutil::miTime   createObsTime();
  int              findStationid();
  miutil::miString rejectComment; 

  std::map<miutil::miString,int>  icaoRegister;

public:
  bool initialise(const std::list<kvalobs::kvStation>&, const miutil::miString& wwfile="weather.tab");

  void setReferenceTime(const miutil::miTime& r); ///< set ref, continuous = false
  void autoReferenceTime();                       ///< continuous = true; ref=now

  bool decode(miutil::miString);                  ///< false for rejected

  std::list<kvalobs::kvData>     Data();
  std::list<kvalobs::kvTextData> Text();

  kvalobs::kvRejectdecode  rejected();            ///< use in case of repel

};

#endif
