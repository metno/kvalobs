/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvTypes.h,v 1.1.2.2 2007/09/27 09:02:30 paule Exp $                                                       

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
#ifndef _kvTypes_h
#define _kvTypes_h

#include <kvalobs/kvDbBase.h>

/* Created by DNMI/FoU/PU: j.schulze@dnmi.no
   at Tue Aug 27 09:17:38 2002 */
namespace kvalobs {
  
    /**
   * \addtogroup  dbinterface
   *
   * @{
   */  


  /**
   * \brief Interface to the table types in the kvalobs database.
   */

  class kvTypes :  public kvDbBase {
  private:
    int typeid_;
    miutil::miString format_;
    int earlyobs_;
    int lateobs_;
    miutil::miString read_;
    miutil::miString obspgm_; 
    miutil::miString comment_;

  public:
    kvTypes() {}
    kvTypes(const dnmi::db::DRow& r) { set(r);}
    kvTypes( int ty, miutil::miString na, int earlyobs, int lateobs,  
	     miutil::miString read, miutil::miString obs, miutil::miString co )
    {set(ty, na, earlyobs, lateobs, read, obs, co);}
    
    bool set(const dnmi::db::DRow&);
    bool set(int, miutil::miString, int, int, miutil::miString,  miutil::miString, miutil::miString );

    miutil::miString uniqueKey()const;

    int typeID()               const {return typeid_; }
    miutil::miString format()  const {return format_;   }
    int earlyobs()             const {return earlyobs_;}
    int lateobs()              const {return lateobs_;}
    miutil::miString read()    const {return read_;}
    miutil::miString obspgm()  const {return obspgm_;}
    miutil::miString comment() const {return comment_;}
  
    
    char* tableName() const {return "types";}
    miutil::miString toSend() const;
 
  };

  /** @} */
}
#endif




