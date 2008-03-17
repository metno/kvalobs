/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvStationInfo.h,v 1.1.2.2 2007/09/27 09:02:30 paule Exp $                                                       

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
#ifndef __kvStationInfo_h__
#define __kvStationInfo_h__

#include <iostream>
#include <list>
#include <puTools/miTime>
#include <kvalobs/kvDataFlag.h>
#include <kvalobs/kvData.h>




namespace kvalobs{
  /**
   * \addtogroup kvinternalhelpers
   * @{
   */

  /**
   * \brief This class is used as an interface to the CORBA interface
   * CKvalObs::StationInfo.
   *
   * The class plays an importen role for the data flow in the 
   * kvalobs system.  
   */
  class kvStationInfo{
  private:
    long            stationid_;
    miutil::miTime  obstime_;
    int             typeid_;


  public:
    /**
     * \brief Initialize the object.
     *
     * \param stationid The kvalobs stationid.
     * \param obstime The observation time for this observation.
     * \param typeId The typeid to the observation.
     */
    kvStationInfo(long                 stationid, 
		  const miutil::miTime &obstime, 
		  int                  typeId)
      : stationid_(stationid), obstime_(obstime), typeid_(typeId)    
      {
      }
    
    
    kvStationInfo(const kvStationInfo &info);
    kvStationInfo &operator=(const kvStationInfo &info);
    
    ///The stationid for this observation
    long           stationID()const{ return stationid_;}

    ///The observation time for this observation
    miutil::miTime obstime()const  { return obstime_;}

    ///The typeid for this observation
    int            typeID()const   { return typeid_;}


    friend std::ostream& operator<<(std::ostream& os, 
				      const kvStationInfo &c);

  };
  
  typedef std::list<kvStationInfo>                 kvStationInfoList;
  typedef std::list<kvStationInfo>::iterator       IkvStationInfoList;
  typedef std::list<kvStationInfo>::const_iterator CIkvStationInfoList;

  /** @} */
};


#endif
