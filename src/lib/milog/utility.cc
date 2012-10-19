/*
  Kvalobs - Free Quality Control Software for Meteorological Observations

  $Id: mkdir.cc,v 1.1.2.3 2007/09/27 09:02:28 paule Exp $

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

#include <sstream>
#include <string>
#include <milog/milog.h>

using namespace std;
namespace milog {

bool
createGlobalLogger( const std::string &path,
                    const std::string &prefix,
                    const std::string &id,
                    milog::LogLevel ll,
                    int size,
                    int nFiles,
                    StdLayout1 *layout)
{
   try{

      if( LogManager::hasLogger(id) ) {
         return true;
      }

      if( size < 200 ) //Must be at least 200k
         size = 200;

      size *= 1024;

      if( nFiles < 1 )
         nFiles=1;
      FLogStream *logs;

      if( layout )
         logs = new FLogStream( layout, nFiles, size );
      else
         logs = new FLogStream( nFiles, size );

      std::ostringstream ost;

      ost << path;

      if( !path.empty() && path[path.length()-1] != '/' )
         ost << "/";

      ost <<  prefix << "_" << id << ".log";

      if( logs->open( ost.str() ) ){
         logs->loglevel( ll );

         if( !LogManager::createLogger( id, logs )){
            LOGERROR("Failed to create logger '" << id << "'. Logfile: '" << ost.str()<<"'.");
            delete logs;
            return false;
         }

         LOGINFO("Created logger '" << id << "'. Logfile: '" << ost.str() <<"'.");
         return true;
      }else{
         LOGERROR("Can't open the logfile <" << ost.str() << ">!");
         delete logs;
         return false;
      }
   }
   catch(...){
      LOGERROR("EXCEPTION: Can't create a logstream for logid '" << id << "'.");
      return false;
   }
}
}
