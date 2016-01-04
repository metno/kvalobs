/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: kvSynopDecoder.h,v 1.10.2.2 2007/09/27 09:02:18 paule Exp $                                                       

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
#ifndef _kvSynopDecoder_h
#define _kvSynopDecoder_h

#include "synop.h"

#include <kvalobs/kvStation.h>
#include <kvalobs/kvData.h>
#include <kvalobs/kvRejectdecode.h>
#include <puTools/miTime.h>
#include <list>
#include <map>
#include <string>

/** Created by met.no/FoU/PU: j.schulze@met.no
 at Thu Jan 23 13:24:08 2003 */

/** kvalobs-specific synop-decoder.
 Reads a synop as a string and return a list<kvData>.

 needs to be initialised with the kvalobs station list
 to map the wmo-number to a valid kvalobs-station-number
 */

class kvSynopDecoder {
 private:

  synop obs;  ///< the resulting observation

  miutil::miTime ref;                 ///< referencetime  
  bool continuous;          ///< if true -> ref = now (default)

  miutil::miTime createObsTime();
  int findStationid();

  std::map<int, int> synopRegister;
  std::map<std::string, int> tempoRegister;
  std::map<std::string, int> shipRegister;

  int temporaryRegister(int);
  int temporaryRegister(std::string);

  bool needTmp;                    ///</ need a temporary station?
  int lastTempoIndex;             ///</ the newest index
  kvalobs::kvStation tstat;     ///</ unknown SHIP temporary station

  int early;                       ///</ time check ( 2 early? in min)
  int late;                        ///</ time check ( 2 late ? in min)

  std::string rejectComment;
  bool hshsInMeter;

  char checkObservationTime(miutil::miTime tbt, miutil::miTime obt);
  void correct_h_VV_N(std::list<kvalobs::kvData> &data) const;

 public:
  std::string msgid;  //Set by findStationid to identify the msg when writing errorlog.

  static miutil::miTime createObsTime(int day, int hour,
                                      const miutil::miTime &refTime);
  static miutil::miTime firstDayNextMonth(const miutil::miTime &mi);
  static miutil::miTime lastDayThisMonth(const miutil::miTime &mi);

  void setHshsInMeter(bool hshsInMeter_) {
    hshsInMeter = hshsInMeter_;
  }
  bool initialise(const std::list<kvalobs::kvStation>&, int e = 20, int l = 30);
  void setReferenceTime(const miutil::miTime& r);  ///< set ref, continuous = false
  void autoReferenceTime();                      ///< continuous = true; ref=now

  bool decode(const std::string &, std::list<kvalobs::kvData>&);  ///< false for rejected
  kvalobs::kvRejectdecode rejected(const std::string &decoder = "");  ///< use in case of repel
  bool tmpStation(kvalobs::kvStation&);                     ///< true if any ...

};

#endif

