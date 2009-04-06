/*
  Kvalobs - Free Quality Control Software for Meteorological Observations

  $Id: kvRejectdecode.h,v 1.1.2.2 2007/09/27 09:02:30 paule Exp $

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
#ifndef _kvRejectdecode_h
#define _kvRejectdecode_h

#include <kvalobs/kvDbBase.h>

/* Created by DNMI/FoU/PU: j.schulze@dnmi.no
   at Tue Aug 27 08:46:16 2002 */

namespace kvalobs {
  /**
   * \addtogroup  dbinterface
   *
   * @{
   */


  /**
   * \brief Interface to the table rejectdecode in the kvalobs database.
   */

  class kvRejectdecode  : public kvDbBase {
  private:
    miutil::miString message_;
    miutil::miTime   tbtime_;
    miutil::miString decoder_;
    miutil::miString comment_;

  public:
    kvRejectdecode() {}
    kvRejectdecode(const dnmi::db::DRow& r) { set(r);}
    kvRejectdecode(const miutil::miString &me,
		   const miutil::miTime   &tb,
		   const miutil::miString &decode,
		   const miutil::miString &comment )
    {set(me,tb,decode,comment);}

    bool set(const dnmi::db::DRow&);
    bool set(const miutil::miString &message,
	     const miutil::miTime   &tbtime,
	     const miutil::miString &decoder,
	     const miutil::miString &comment);


    const char* tableName() const {return "rejectdecode";}
    miutil::miString toSend() const;
    miutil::miString uniqueKey()const;

    miutil::miString message() const {return message_;}
    miutil::miTime   tbtime()  const {return tbtime_; }
    miutil::miString decoder() const {return decoder_;}
    miutil::miString comment() const {return comment_;}

  };

  /** @} */
}
#endif
