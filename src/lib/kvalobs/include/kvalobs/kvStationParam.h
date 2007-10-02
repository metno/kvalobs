/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvStationParam.h,v 1.1.2.2 2007/09/27 09:02:30 paule Exp $                                                       

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
#ifndef _kvStationParam_h
#define _kvStationParam_h

#include <kvalobs/kvDbBase.h>

/* Created by DNMI/FoU/PU: j.schulze@met.no
   at Mon Aug 26 2002 */
/* Edited by T.Reite 21. mai 2003*/

namespace kvalobs {

    /**
   * \addtogroup  dbinterface
   *
   * @{
   */  


  /**
   * \brief Interface to the table station_param in the kvalobs database.
   */

  class kvStationParam : public kvDbBase {
  private:
    int stationid_;
    int paramid_;
    int level_;
    int sensor_;
    int fromday_;
    int today_;
    int hour_;
    miutil::miString qcx_;
    miutil::miString metadata_;
    miutil::miString descMetadata_;
    miutil::miTime fromtime_;

  public:
    kvStationParam() {}
    kvStationParam(const dnmi::db::DRow& r) {set(r);}
    kvStationParam(int st, int pa, int lev, int sen, int fr, int to, int hour, const miutil::miString& qc, 
		   const miutil::miString& md, const miutil::miString& dm, 
		   const miutil::miTime& fromtime) 
    {set(st,pa,lev,sen,fr,to,hour,qc,md,dm,fromtime);}
    
    bool set(const dnmi::db::DRow&);
    bool set(int, int, int, int, int, int, int, const miutil::miString&, 
	     const miutil::miString&, const miutil::miString&, const miutil::miTime&);


    char* tableName() const {return "station_param";}
    miutil::miString toSend() const;
    miutil::miString uniqueKey()const;    

    int stationID()                 const { return stationid_;   }
    int paramID()                   const { return paramid_;     }
    int level()                     const { return level_;       }
    int sensor()                    const { return sensor_;      }
    int fromday()                   const { return fromday_;     }
    int today()                     const { return today_;       }
    int hour()                      const { return hour_;        }
    miutil::miString qcx()          const { return qcx_;         }
    miutil::miString metadata()     const { return metadata_;    }
    miutil::miString descMetadata() const { return descMetadata_;}
    miutil::miTime fromtime()       const { return fromtime_;    }


    miutil::miDate fromDAY(int y=-1) const 
    { return  julianDayThatYear(fromday_,y); }

    miutil::miDate toDAY(int y=-1) const 
    { return  julianDayThatYear(today_,y); }
  };

  /** @} */
}

#endif


