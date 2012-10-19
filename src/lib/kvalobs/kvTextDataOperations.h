/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvTextDataOperations.h,v 1.1.2.2 2007/09/27 09:02:30 paule Exp $                                                       

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
#ifndef KVTEXTDATAOPERATIONS_
#define KVTEXTDATAOPERATIONS_

#include "kvalobs/kvTextData.h"
#include <functional>

namespace kvalobs
{
  class kvTextDataFactory
  {
  public:
    kvTextDataFactory( int stationID, const miutil::miTime & obsTime, int typeID );
    
    explicit kvTextDataFactory( const kvTextData & data );
    
    kvTextData getData( std::string val, int paramID, const miutil::miTime & obsTime = miutil::miTime() ) const;
    
  private:
    const int stationID_;
    const int typeID_;
    const miutil::miTime obstime_;
  };

  namespace compare
  {
    typedef std::binary_function<kvTextData, kvTextData, bool> kvTextDataCompare;

    struct lt_kvTextData : kvTextDataCompare
    {
      bool operator()( const kvTextData & a, const kvTextData & b ) const;
    };

    struct kvTextData_same_obs_and_parameter : kvTextDataCompare
    {
      bool operator()( const kvTextData & a, const kvTextData & b ) const;
    };

    struct kvTextData_exactly_equal_ex_tbtime : kvTextDataCompare
    {
      bool operator()( const kvTextData & a, const kvTextData & b ) const;
    };

  }
}


#endif // KVTEXTDATAOPERATIONS_
