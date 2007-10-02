/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: QaWorkThread.h,v 1.1.2.3 2007/09/27 09:02:21 paule Exp $                                                       

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
#ifndef __QaWorkThread_h__
#define __QaWorkThread_h__


#include <kvalobs/kvStationInfo.h>
#include <string>
#include <stack>

class QaBaseApp;
class QaWorkCommand;

/**
   \brief Manage input queue (work-queue) to QaBase

   - Start checks for one station and time
*/

class QaWork
{
    QaBaseApp & app;
    std::string logpath_; ///<logpath for the html output.

  public:
    QaWork( QaBaseApp &app_, const std::string& logpath = "./log" );

    void doWork( const kvalobs::kvStationInfo & params,
                 kvalobs::kvStationInfoList & retParams,
                 dnmi::db::Connection & con,
                 const std::string & logPath );

    void process( dnmi::db::Connection & con, const QaWorkCommand & work );

    void operator() ();
};



#endif
