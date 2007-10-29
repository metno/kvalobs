/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: KvObsData.h,v 1.3.2.3 2007/09/27 09:02:45 paule Exp $                                                       

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
#ifndef __kvservice__KvObsData_h__
#define __kvservice__KvObsData_h__

#include <list>
#include <kvskel/kvServiceCommon.hh>
#include <kvalobs/kvData.h>
#include <kvalobs/kvTextData.h>

namespace kvservice
{
  /**
   * \addtogroup kvcpp2
   * @{
   */

  /**
   * \brief A collection of observations, as kvData and kvTextData
   * objects, for a single station.
   */
  class KvObsData
  {
  public:

    typedef std::list<kvalobs::kvData>     kvDataList;
    typedef std::list<kvalobs::kvTextData> kvTextDataList;

    /**
     * \brief Object will contain no data.
     */
    KvObsData();

    /**
     * \brief The resulting object will contain the data data
     * represented by the given object.
     *
     * \param obsData The CORBA data object, normally returned by
     * kvalobs, containing the observations this object is to
     * represent.
     */
    KvObsData( const CKvalObs::CService::ObsData &obsData );

    virtual ~KvObsData( );

    virtual void operator=( const CKvalObs::CService::ObsData &obsData );

    virtual operator kvDataList&() { return dataList_; }

    virtual operator kvTextDataList&() { return textDataList_; }

    /**
     * \brief Get this collection of data's stationID.
     */
    inline int stationid() const { return stationid_; }

    /**
     * \brief Get the list of KvData elements.
     **/ 
    inline kvDataList &dataList() { return dataList_; }

    /**
     * \brief Get the list of KvTextData elements.
     */
    inline kvTextDataList &textDataList() { return textDataList_; }

    /**
     * \brief clear all internal lists, that is: make this object
     * represent no station, and contain no data.
     */
    virtual void clear();
              
  private:
    int stationid_;
    kvDataList     dataList_;
    kvTextDataList textDataList_;
  };
}

#endif // __kvservice__KvObsData_h__
