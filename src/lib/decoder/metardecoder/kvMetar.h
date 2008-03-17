/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvMetar.h,v 1.2.2.2 2007/09/27 09:02:37 paule Exp $                                                       

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
#ifndef _kvMetar_h
#define _kvMetar_h

#include <puTools/miString>
#include <puTools/miTime>
#include "kvMetarElements.h"

extern "C" {
#include <metar.h>
}

#include <list>

/// Created by met.no/FoU/PU: j.schulze@met.no
/// at Tue Apr  1 11:23:47 2003

/// Class to hold a TIPS-KVALOBS metar
/// syntax check of the original METAR is done in 
/// the TIPS c-library kvMetar is a kvalobs 
/// compatible wrapper around that 

class kvMetar {
private:
  int  unsafe;                         ///< copy pointer trick
  metar* m;                            ///< TIPS metar structure
  
  miutil::miString raw;                ///< the orig. message
  miutil::miString unrecognised;       ///< unknown metar

  std::list<kmet::obsbuf> obs;         ///< all data storage 
  std::list<kmet::txtbuf> txt;         ///< all text storage 

  void safe_create();                  
  void copyMembers(const kvMetar&);
  void clear();
  void setUnrecognised();
  void setMetarTokens();
  void setToken(const int par,float value,int lvl=0);
  void setToken(const int par,miutil::miString value);
  void setWind( const int par,float value, metarWindUnit unit);
  const int dir2int(const metarDirection);
  const int amount2int(const metarCloudAmount);
public:
  kvMetar() {safe_create();}
  kvMetar(const kvMetar&); 
  ~kvMetar();

  /// wwTabInit has to be done once (with a file weather.tab)
  /// to initialise legal weather elements -
  /// the result is a static structure - accessable for all
  /// kvMetars
  void wwTabInit(const miutil::miString&);

  miutil::miString        Raw()          const { return raw;         }
  miutil::miString        Unrecognised() const { return unrecognised;}
  std::list<kmet::obsbuf> Data()         const { return obs;         } 
  std::list<kmet::txtbuf> Text()         const { return txt;         } 


  bool hasUnrecognised() { return !unrecognised.empty();}

  bool decode(const miutil::miString&);
  
  miutil::miString ICAOID();
  miutil::miTime   createObsTime(miutil::miTime);

};

#endif
