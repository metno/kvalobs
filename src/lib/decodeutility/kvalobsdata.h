/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvalobsdata.h,v 1.1.2.3 2007/09/27 09:02:27 paule Exp $                                                       

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
#ifndef KVALOBSDATA_H
#define KVALOBSDATA_H

#include "sorteddata.h"
#include <string>
#include <map>
#include <list>
#include <utility>
#include <kvalobs/kvDataFlag.h>
#include <puTools/miTime>

namespace kvalobs
{
class kvData;
class kvTextData;

namespace serialize
{

/**
 * The content of a serialized message
 *
 * @author Vegard B�nes
 */
class KvalobsData
{
  public:
    KvalobsData();

    KvalobsData( const std::list<kvData> & data, const std::list<kvTextData> & tdata );

    ~KvalobsData();

    bool empty() const;
    size_t size() const;

    void insert( const kvalobs::kvData & d );
    void insert( const kvalobs::kvTextData & d );
    template<typename InputIterator>
    void insert( InputIterator begin, InputIterator end )
    {
      for ( ; begin != end; ++ begin )
        insert( * begin );
    }

    void getData( std::list<kvalobs::kvData> & out, const miutil::miTime & tbtime = miutil::miTime() ) const;
    void getData( std::list<kvalobs::kvTextData> & out, const miutil::miTime & tbtime = miutil::miTime()  ) const;
    void getData( std::list<kvalobs::kvData> & out1, std::list<kvalobs::kvTextData> & out2 ,
                  const miutil::miTime & tbtime = miutil::miTime() ) const
    {
      getData( out1 );
      getData( out2 );
    }

    bool overwrite() const
    {
      return overwrite_;
    }
    void overwrite( bool doit )
    {
      overwrite_ = doit;
    }

    void invalidate( bool doit, int station, int typeID, const miutil::miTime & obstime );
    bool isInvalidate( int station, int typeID, const miutil::miTime & obstime ) const;

    struct InvalidateSpec
    {
      int station;
      int typeID;
      miutil::miTime obstime;
      InvalidateSpec( int st, int ty, miutil::miTime ot )
          : station( st ), typeID( ty ), obstime( ot )
      {}
    };
    void getInvalidate( std::list<InvalidateSpec> & invSpec );

    /**
     * Const access to data holder
     */
    const internal::Observations & obs() const
    {
      return obs_;
    }

  private:
    bool overwrite_;
    internal::Observations obs_;
};

}

}

#endif
