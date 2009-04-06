/*
  Kvalobs - Free Quality Control Software for Meteorological Observations

  $Id: kvQcxInfo.h,v 1.1.2.2 2007/09/27 09:02:30 paule Exp $

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
#ifndef _kvQcxInfo_h
#define _kvQcxInfo_h

#include <kvalobs/kvDbBase.h>

/* Edited by T.Reite 30. januar 2003 */

namespace kvalobs {
  /**
   * \addtogroup  dbinterface
   *
   * @{
   */


  /**
   * \brief Interface to the table qcx_info in the kvalobs database.
   */


class kvQcxInfo : public kvDbBase {
private:
  miutil::miString medium_qcx_;
  miutil::miString main_qcx_;
  int              controlpart_;
  miutil::miString comment_;

public:

  kvQcxInfo() {};
  kvQcxInfo( const dnmi::db::DRow& r) {set(r);}
  kvQcxInfo( const miutil::miString& medium_qcx,
	     const miutil::miString& main_qcx,
             int controlpart,
	     const miutil::miString& comment
	     )
      { set( medium_qcx, main_qcx, controlpart, comment);}
  ~kvQcxInfo();

  bool set( const miutil::miString& medium_qcx,
	    const miutil::miString& main_qcx,
            int controlpart,
	    const miutil::miString& comment);


  bool set(const dnmi::db::DRow&);
  const char* tableName() const {return "qcx_info";}
  miutil::miString toSend() const;
  miutil::miString uniqueKey()const;

  miutil::miString medium_qcx() const {return medium_qcx_; }
  miutil::miString main_qcx() const {return main_qcx_; }
  int controlpart()          const {return controlpart_; }
  miutil::miString comment() const {return comment_; }
  };

/** @} */
}

#endif


