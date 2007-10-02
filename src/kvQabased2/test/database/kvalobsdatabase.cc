/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvalobsdatabase.cc,v 1.1.2.3 2007/09/27 09:02:21 paule Exp $                                                       

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
#include "kvalobsdatabase.h"
#include <db/dbdrivermgr.h>
#include <stdexcept>
#include <fstream>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

using namespace std;
using namespace dnmi::db;

namespace
{
  DriverManager dm;

  string getDriverPath()
  {
    const char * kvalobs = getenv( "KVALOBS" );
    if ( ! kvalobs )
      kvalobs = ".";
    return string( kvalobs ) + "/lib/db/sqlite3driver.so";
  }

  string getConnectString()
  {
    string ret;
    if ( ! dm.loadDriver( getDriverPath(), ret ) )
      return string();
    return ret;
  }

  string getDbSetup()
  {
    ifstream f( "test/database/setupdb.sql" );
    string ret;
    getline( f, ret, char_traits<char>::to_char_type( char_traits<char>::eof() ) );
    return ret;
  }
}

const std::string KvalobsDatabase::dbConnectString = getConnectString();
const std::string KvalobsDatabase::dbSetup = getDbSetup();
const std::string KvalobsDatabase::db_memory_database_filename = ":memory:";

void KvalobsDatabase::setup( const std::string & filename )
{
  if ( dbConnectString.empty() )
    throw std::runtime_error( "Unable to connect to database driver: " + getDriverPath() );

  if ( filename != db_memory_database_filename )
    boost::filesystem::remove( filename );
    
  connection_ = dm.connect( dbConnectString, filename );
  if ( dbSetup.empty() )
    throw std::runtime_error( "Unable to find database setup file" );
  connection_->exec( dbSetup );
}


KvalobsDatabase::KvalobsDatabase()
{
  setup( db_memory_database_filename );
}


KvalobsDatabase::KvalobsDatabase( const std::string & filename )
{
  setup( filename );
}


KvalobsDatabase::~KvalobsDatabase()
{
  delete connection_;
}

