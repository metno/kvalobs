/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvObsPgm.h,v 1.1.2.2 2007/09/27 09:02:30 paule Exp $                                                       

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
#ifndef _kvObsPgm_h
#define _kvObsPgm_h 

#include <kvalobs/kvDbBase.h> 

/* autogenerated c++ header file from kv databasescript */


namespace kvalobs {

  /**
   * \addtogroup  dbinterface
   *
   * @{
   */  


  /**
   * \brief Interface to the table obs_pgm in the kvalobs database.
   */

class kvObsPgm : public kvDbBase {
private: 
  int stationid_;
  int paramid_;
  int level_;
  int nr_sensor_;
  int typeid_;
  bool collector_;
  bool kl00_;
  bool kl01_;
  bool kl02_;
  bool kl03_;
  bool kl04_;
  bool kl05_;
  bool kl06_;
  bool kl07_;
  bool kl08_;
  bool kl09_;
  bool kl10_;
  bool kl11_;
  bool kl12_;
  bool kl13_;
  bool kl14_;
  bool kl15_;
  bool kl16_;
  bool kl17_;
  bool kl18_;
  bool kl19_;
  bool kl20_;
  bool kl21_;
  bool kl22_;
  bool kl23_;
  bool mon_;
  bool tue_;
  bool wed_;
  bool thu_;
  bool fri_;
  bool sat_;
  bool sun_;
  miutil::miTime fromtime_;

public:

  kvObsPgm() {};
  kvObsPgm(const dnmi::db::DRow& r) {set(r);} 
  kvObsPgm( int stationid,
            int paramid,
	    int level,
            int nr_sensor,
	    int typ,
            bool collector,
            bool kl00,
            bool kl01,
            bool kl02,
            bool kl03,
            bool kl04,
            bool kl05,
            bool kl06,
            bool kl07,
            bool kl08,
            bool kl09,
            bool kl10,
            bool kl11,
            bool kl12,
            bool kl13,
            bool kl14,
            bool kl15,
            bool kl16,
            bool kl17,
            bool kl18,
            bool kl19,
            bool kl20,
            bool kl21,
            bool kl22,
            bool kl23,
            bool mon,
            bool tue,
            bool wed,
            bool thu,
            bool fri,
            bool sat,
            bool sun,
            const miutil::miTime& fromtime )
  { set(stationid, paramid, level, nr_sensor, typ, collector, kl00, kl01, kl02, kl03, kl04, kl05, kl06, kl07, kl08, kl09, kl10, kl11, kl12, kl13, kl14, kl15, kl16, kl17, kl18, kl19, kl20, kl21, kl22, kl23, mon, tue, wed, thu, fri, sat, sun, fromtime);}

  bool set( int stationid,
            int paramid,
            int level,
            int nr_sensor,
	    int typ,
            bool collector,
            bool kl00,
            bool kl01,
            bool kl02,
            bool kl03,
            bool kl04,
            bool kl05,
            bool kl06,
            bool kl07,
            bool kl08,
            bool kl09,
            bool kl10,
            bool kl11,
            bool kl12,
            bool kl13,
            bool kl14,
            bool kl15,
            bool kl16,
            bool kl17,
            bool kl18,
            bool kl19,
            bool kl20,
            bool kl21,
            bool kl22,
            bool kl23,
            bool mon,
            bool tue,
            bool wed,
            bool thu,
            bool fri,
            bool sat,
            bool sun,
            const miutil::miTime& fromtime );

  bool set(const dnmi::db::DRow&);
  char* tableName() const {return "obs_pgm";}
  miutil::miString toSend() const;
  miutil::miString uniqueKey()const;

  int stationID()           const {return stationid_; }
  int paramID()             const {return paramid_; }
  int level()               const {return level_; }       
  int nr_sensor()           const {return nr_sensor_; }
  int typeID()              const {return typeid_; }
  bool collector()          const {return collector_; }
  bool kl00()               const {return kl00_; }
  bool kl01()               const {return kl01_; }
  bool kl02()               const {return kl02_; }
  bool kl03()               const {return kl03_; }
  bool kl04()               const {return kl04_; }
  bool kl05()               const {return kl05_; }
  bool kl06()               const {return kl06_; }
  bool kl07()               const {return kl07_; }
  bool kl08()               const {return kl08_; }
  bool kl09()               const {return kl09_; }
  bool kl10()               const {return kl10_; }
  bool kl11()               const {return kl11_; }
  bool kl12()               const {return kl12_; }
  bool kl13()               const {return kl13_; }
  bool kl14()               const {return kl14_; }
  bool kl15()               const {return kl15_; }
  bool kl16()               const {return kl16_; }
  bool kl17()               const {return kl17_; }
  bool kl18()               const {return kl18_; }
  bool kl19()               const {return kl19_; }
  bool kl20()               const {return kl20_; }
  bool kl21()               const {return kl21_; }
  bool kl22()               const {return kl22_; }
  bool kl23()               const {return kl23_; }
  bool mon()                const {return mon_; }
  bool tue()                const {return tue_; }
  bool wed()                const {return wed_; }
  bool thu()                const {return thu_; }
  bool fri()                const {return fri_; }
  bool sat()                const {return sat_; }
  bool sun()                const {return sun_; }
  miutil::miTime fromtime() const {return fromtime_; }

  bool isOn(const miutil::miTime&) const;
  };
/** @} */ 
};

#endif










