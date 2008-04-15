/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvalobsdatabase.h,v 1.1.2.2 2007/09/27 09:02:21 paule Exp $                                                       

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
#ifndef KVALOBSDATABASE_H
#define KVALOBSDATABASE_H

#include "kvQABaseDBConnection.h"
#include <string>
#include <kvdb/kvdb.h>
#include <boost/noncopyable.hpp>

/**
 * A database for testing, looking more or less like the one in kvalobs.
 *
 * @author Vegard Bnes
 */
class KvalobsDatabase : boost::noncopyable
{
  public:
    /**
     * Create an in-memory database. The database is deleted along with this
     * object.
     *
     * @throw std::runtime_error If unable to connect to database.
     */
    KvalobsDatabase();

    /**
     * Create a database, storing data to the specified file. If the file
     * already existed when this call is made, the old file will be deleted.
     * The database will not be deleted along with this object.
     *
     * @param filename The name of the file in which to store the database.
     *
     * @throw std::runtime_error If unable to connect to database.
     */
    KvalobsDatabase( const std::string & filename );

    ~KvalobsDatabase();

    /**
     * Get a connection to the database
     */
    dnmi::db::Connection * getConnection() { return connection_; }
    
    kvQABaseDBConnection * getQaBaseConnection() { return con_; }

  private:
    dnmi::db::Connection * connection_;
    
    kvQABaseDBConnection * con_;
    
    static const std::string dbConnectString;
    static const std::string dbSetup;
    static const std::string db_memory_database_filename;

    void setup( const std::string & filename );
};

#endif
