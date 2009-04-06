/*
  Kvalobs - Free Quality Control Software for Meteorological Observations

  $Id: kvTextData.h,v 1.1.2.2 2007/09/27 09:02:30 paule Exp $

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
#ifndef _kvTextData_h
#define _kvTextData_h

#include <kvalobs/kvDbBase.h>


namespace kvalobs {

  /**
   * \addtogroup  dbinterface
   *
   * @{
   */


  /**
   * \brief Interface to the table text_data in the kvalobs database.
   */

class kvTextData : public kvDbBase {
private:
  int              stationid_;
  miutil::miTime   obstime_;
  miutil::miString original_;
  int              paramid_;
  miutil::miTime   tbtime_;
  int              typeid_;

public:

  kvTextData() {};
  kvTextData(const dnmi::db::DRow& r) {set(r);}
  kvTextData(int                     sta,
             const miutil::miTime&   obt,
             const miutil::miString& org,
             int                     pid,
             const miutil::miTime&   tbt,
	     int                     typ )
  { set(sta, obt, org, pid, tbt, typ);}

  bool set(const dnmi::db::DRow& );
  bool set(int                     sta,
           const miutil::miTime&   obt,
           const miutil::miString& org,
           int                     pid,
           const miutil::miTime&   tbt,
           int                     typ );


  const char* tableName() const {return "text_data";}
  miutil::miString toSend() const;
  miutil::miString uniqueKey()const;

  int              stationID() const { return stationid_;}
  miutil::miTime   obstime()   const { return obstime_;  }
  miutil::miString original()  const { return original_; }
  int              paramID()   const { return paramid_;  }
  miutil::miTime   tbtime()    const { return tbtime_;   }
  int              typeID()    const { return typeid_;   }

  void typeID(int t) { typeid_=t;}
};

/** @} */
};

#endif

