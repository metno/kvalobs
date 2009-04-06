/*
  Kvalobs - Free Quality Control Software for Meteorological Observations

  $Id: tblWaiting.h,v 1.2.2.2 2007/09/27 09:02:23 paule Exp $

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
#ifndef __kvsynop_tblWaiting_h__
#define __kvsynop_tblWaiting_h__

#include <kvalobs/kvDbBase.h>

class TblWaiting : public kvalobs::kvDbBase {
private:
  int              wmono_;
  miutil::miTime   obstime_;
  miutil::miTime   delaytime_;

  void createSortIndex();

 public:
  TblWaiting() {clean();}
  TblWaiting(const TblWaiting &waiting){ set(waiting);}
  TblWaiting(const dnmi::db::DRow &r){set(r);}
  TblWaiting(int                  wmono,
	     const miutil::miTime &obstime,
	     const miutil::miTime &delaytime)
  { set(wmono, obstime, delaytime);}

  bool set(int                  wmono,
	   const miutil::miTime &obtime,
	   const miutil::miTime &delaytime);

  bool set(const dnmi::db::DRow&);
  bool set(const TblWaiting &waiting);

  void clean();

  const char*            tableName() const {return "waiting";}
  miutil::miString toSend()    const;
  miutil::miString toUpdate()  const;
  miutil::miString uniqueKey() const;

  int              wmono()    const { return wmono_;    }
  miutil::miTime   obstime()  const { return obstime_;  }
  miutil::miTime   delaytime()const { return delaytime_;}

  void delaytime(const miutil::miTime &t){ delaytime_=t;}
};

#endif
