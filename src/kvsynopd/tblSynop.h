/*
  Kvalobs - Free Quality Control Software for Meteorological Observations

  $Id: tblSynop.h,v 1.2.6.2 2007/09/27 09:02:23 paule Exp $

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
#ifndef __kvsynop_tblSynop_h__
#define __kvsynop_tblSynop_h__

#include <kvalobs/kvDbBase.h>

class TblSynop : public kvalobs::kvDbBase {
private:
  int              wmono_;
  miutil::miTime   obstime_;
  miutil::miTime   createtime_;
  int              crc_;
  int              ccx_;
  std::string      wmomsg_;

  void createSortIndex();

public:
  TblSynop() {clean();}
  TblSynop(const TblSynop &synop){ set(synop);}
  TblSynop(const dnmi::db::DRow &r){set(r);}
  TblSynop(int                  wmono,
	   const miutil::miTime &obtime,
	   const miutil::miTime &createtime,
	   int                  crc,
	   int                  ccx,
	   const std::string    &wmomsg)
  { set(wmono, obtime, createtime, crc, ccx, wmomsg);}

  bool set(int                  wmono,
	   const miutil::miTime &obtime,
	   const miutil::miTime &createtime,
	   int                  crc,
	   int                  ccx,
	   const std::string    &wmomsg);

  bool set(const dnmi::db::DRow&);
  bool set(const TblSynop &synop);

  TblSynop& operator=(const TblSynop &ts){
                  if(&ts!=this)
		    set(ts);
		  return *this;
             }

  void clean();

  const char* tableName()            const {return "synop";}
  miutil::miString toSend()    const;
  miutil::miString toUpdate()  const;
  miutil::miString uniqueKey() const;

  int              wmono()       const { return wmono_;     }
  miutil::miTime   obstime()     const { return obstime_;   }
  miutil::miTime   createtime()  const { return createtime_;}
  int              crc()         const { return crc_;       }
  int              ccx()         const { return ccx_;       }
  std::string      wmomsg()      const { return wmomsg_;    }

  void createtime(const miutil::miTime &t){ createtime_=t;}
  void crc(int c)                         { crc_=c;       }
  void incCcx()                           { ccx_++;       }
  void ccx(int c)                         { ccx_=c;       }
  void wmomsg(const std::string &s)       { wmomsg_=s;    }
};

#endif
