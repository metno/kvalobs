/*
  Kvalobs - Free Quality Control Software for Meteorological Observations

  $Id: kvServiceElement.h,v 1.1.2.2 2007/09/27 09:02:30 paule Exp $

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
#ifndef __kvalobs_db_kvServiceElement_h__
#define __kvalobs_db_kvServiceElement_h__

#include <kvalobs/kvDbBase.h>

/*
 * Created by DNMI/IT: borge.moe@met.no
 * at Tue Sep 20  20:15:16 2004
 */

namespace kvalobs{
  /**
   * \addtogroup  dbinterface
   *
   * @{
   */


  /**
   * \brief Interface to the table service_element in the kvalobs database.
   */


class kvServiceElement : public kvDbBase {
private:
  int            stationid_;
  miutil::miTime obstime_;
  int            typeid_;

  void createSortIndex();

public:
  kvServiceElement();
  kvServiceElement(const dnmi::db::DRow &r){set(r);}
  kvServiceElement(int                  sid,
		   const miutil::miTime &obt,
		   int                  tid)
    { set(sid, obt, tid);}

  bool set(int                  sid,
	   const miutil::miTime &obt,
	   int                  tid);

  bool set(const dnmi::db::DRow&);

  const char*            tableName() const {return "service_element";}
  miutil::miString    toSend() const;
  miutil::miString  toUpdate() const;
  miutil::miString uniqueKey() const;

  int            stationID()   const { return stationid_; }
  miutil::miTime obstime()     const { return obstime_;   }
  int            typeID()      const { return typeid_;    }
};

/** @} */

};
#endif
