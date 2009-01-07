/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id$                                                       

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
#include "ProcessImpl.h"
#include "Qc2Thread.h"
#include "Qc2App.h"
#include "Qc2Connection.h"
#include "ReadProgramOptions.h"
#include <sstream>
#include <milog/milog.h>
#include <kvalobs/kvDbGate.h>
#include <puTools/miTime>
#include <memory>
#include <stdexcept>

#include "CheckedDataCommandBase.h"
#include "CheckedDataHelper.h"

using namespace kvalobs;
using namespace std;
using namespace miutil;

// constructor
Qc2Work::Qc2Work( Qc2App &app_, const std::string& logpath )
    : app( app_ ), logpath_( logpath )
{
}

//operator
void Qc2Work::operator() ()
{
  LOGINFO( "Qc2Work: starting work thread!\n" );

// Establish The Connection
  ConnectionHandler connectionHandler( app ); 
  dnmi::db::Connection * con = connectionHandler.getConnection();

  if ( ! con ) {
        LOGERROR( "Could not get connection to database" );
  }

  IDLOGERROR( "html","%%%%%%%%%%%%%%%%%%%%%%%%" );
  LOGINFO( "%%%%%%%%%%%%%%%%%%%%%%%%" );

  ProcessImpl Processor( app, *con);

  while ( !app.shutdown() ) {
        ReadProgramOptions params;           
        std::vector<string> config_files;
        params.SelectConfigFiles(config_files); 
        std::vector<string>::const_iterator vit = config_files.begin();
        while ( vit != config_files.end() )  {
              params.clear(); // very important!!!!!!
              params.Parse( *vit );
              ++vit;
              if ( miutil::miTime::nowTime().min() == params.RunAtMinute &&
                     miutil::miTime::nowTime().hour() == params.RunAtHour ) {
                   try {
                      Processor.select(params);
                   }
                   catch ( dnmi::db::SQLException & ex ) {
                      IDLOGERROR( "html", "Exception: " << ex.what() << std::endl );
                   }
                   catch ( ... ) {
                      IDLOGERROR( "html", "Unknown exception: con->exec(ctbl) .....\n" );
                   }
              }
        }
        sleep(59);   //check config files every minute 
  }                  //end of app while loop
                     //59 seconds is set to avoid the thread getting trapped on a minute boundary
  LOGINFO( "Qc2Work: Thread terminating!" );
}

