/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: GenerateZero.h,v 1.1.2.5 2007/09/27 09:02:15 paule Exp $                                                       

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
#ifndef __agregator__GenerateZero_h__
#define __agregator__GenerateZero_h__

#include <vector>
#include <puTools/miTime>

namespace kvalobs 
{
  class kvObsPgm;
}

namespace agregator
{
  class rr_1;

  /**
   * Performs agregation from rr_01 to RR_1 once every 24 hours, in its own 
   * thread.
   */
  class GenerateZero
  {
  public:
    GenerateZero( rr_1 & ag );
    ~GenerateZero();
    
    /**
     * \brief Main loop for running in a thread.
     * 
     * Run in an eternal loop, waiting until it's time to agregate, or the 
     * server is shutting down. The actual agregation work is done in the 
     * generate() method.
     */
    void operator()();
    
    /**
     * \brief Agregate all RR_01 data for the last 24 hours.
     */
    void generate();
    
    /**
     * \return the value this class will set for a fake observation
     */
    static float obsVal() { return -1; }

    /**
     * \brief Time of day when generation of RR_1 will start.
     */
    static const miutil::miClock genClock;
    
    /**
     * \brief Dependency injection. 
     * 
     * This should not be used for anything but testing. 
     */
     

    
  private:
    void fillObsPgm();
    
    rr_1 & ag;
    
    miutil::miTime nextGen;
    
    miutil::miTime nextGetObsPgm;
    static const miutil::miClock getObsPgmClock;
    
    struct StData 
    {
      int station;
      int type;
      int sensor;
      int lvl;
      
      StData( const kvalobs::kvObsPgm & p );
      StData( int station, int type, int sensor = '0', int level = 0 );
      
      bool operator<( const StData &d ) const;
      bool operator==( const StData &d ) const;
    };

    typedef std::vector<StData> StDataContainer;
    StDataContainer stations;
    
    // Unit test must be able to access StDataContainer stations. 
    friend class GenerateZeroTest;
  };
}

#endif // __agregator__GenerateZero_h__
