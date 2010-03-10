/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvCronString.h,v 1.1.2.3 2007/09/27 09:02:21 paule Exp $                                                       

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
#ifndef _kvCronString_h
#define _kvCronString_h


/* Created by met.no/FoU/PU: a.christoffersen@met.no
   at Wed May 14 17:12:03 2003 */

#include <string>
#include <puTools/miString.h>
#include <puTools/miTime.h>
#include <vector>

namespace kvalobs {

  /**
     \brief Cron-like time string 

     Class CronString interpretes a classic crontab timestring like:

     minutes hours days months years
     - comma-separated list of times
     - * as wildcard
     
     Examples:
     - * * * * *     : at all times (minute-resolution)
     - 5,10 * * * *  : 5 and 10 minutes past all hours of the year
     - * * 20 * *    : every minute on the 20th day of each month
     - 0 3 15 6 2004 : exactly 3 AM on 15th June 2004

     use active(miTime) to check if a particular time gives a match with the cronstring
    
   */

  class CronString {
  private:
    miutil::miString str_;
    bool isempty;
  
    /// to skip using years - set NT to 4 - etc.
    enum { NT = 5 };
  
    /**
      numbers from string:
      0: minutes
      1: hours
      2: days
      3: months
      4: years    
    */
    std::vector<int> numbers[NT];
    
    void unpackString();

  public:
    CronString() : isempty(true) {}
    CronString(const std::string& str);

    void str(const std::string& str);
    std::string str() const {return str_;}

    bool active(const miutil::miTime& t);
  
  };

}

#endif
