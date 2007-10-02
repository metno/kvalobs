/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvQABaseControl.h,v 1.12.2.2 2007/09/27 09:02:37 paule Exp $                                                       

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
#ifndef _kvQABaseControl_h
#define _kvQABaseControl_h


/* Created by DNMI/FoU/PU: a.christoffersen@dnmi.no
   at Wed May  8 08:12:37 2002 
*/


/**
   \brief QABase flowcontrol
   
   Controls the dataflow in module QABase
*/

#include <puTools/miString>
#include <puTools/miTime>
#include <kvalobs/kvStationInfo.h>
#include <milog/milog.h>

class kvQABaseControl {
private:
  /// Path to logfile(s)
  std::string logpath_;
  /// The next time the static tables should be updated.
  miutil::miTime updateStaticTime; 

protected:
  milog::HtmlStream*
  openHTMLStream(long stationid, const miutil::miTime &obstime);
  

public:
  kvQABaseControl();
  kvQABaseControl(const std::string& logp);
  ~kvQABaseControl();
  
  /**
    Set logpath (before first runchecks)
  */
  void logpath(const std::string& logp){logpath_= logp;}
  
  /**
    main routine: runChecks
    do all relevant checks for a station (currently all available checks)
    - input: kvStationInfo with station, observation time and parameters
    - input: database connection

    - run relevant checks for station
    - update kvStationInfo

    return-value != 0 means trouble
  */
  int runChecks(kvalobs::kvStationInfo& stationinfo,
		dnmi::db::Connection *con);
  
};

#endif
