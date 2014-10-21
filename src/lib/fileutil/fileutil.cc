/*
  Kvalobs - Free Quality Control Software for Meteorological Observations

  $Id: file.h,v 1.1.2.2 2007/09/27 09:02:28 paule Exp $

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
#include "file.h"
#include "fileutil.h"
#include "miutil/strerror.h"

using namespace std;

namespace miutil {
namespace file {

using namespace dnmi::file;

namespace {
void
throwOnError( const File &f )
{
    if( ! f.ok() ) {
        if( errno == EACCES )
            throw permission_error();
        else if( errno == ENOENT || errno == ENOTDIR )
            throw path_error( errno );
        else
            throw file_error( errno, "stat failed.");
    }
}
}

file_error::
file_error( int errno_, const std::string &what__ ) : errno_( errno_ )
{
    ostringstream err;

    if( ! what__.empty() )
        err << what__ << " (" << miutil::strerror( errno_ ) << ").";
    else
        err << miutil::strerror( errno_ );
    what_ = err.str();
}

file_error::
file_error( int errno_ ):errno_( errno_ )
{
    what_ = miutil::strerror( errno_ );
}


bool
isdir( const std::string &dir )
{
    File f(dir);

    throwOnError( f );

    return f.isDir();
}

/**
 * @throw std::runtime_error
 */
bool
isfile( const std::string &file )
{
    File f(file);

    throwOnError( f );
    return f.isFile();
}
/**
 * @throw std::runtime_error
 */
bool
isRunable( const std::string &file )
{
    File f(file);
    throwOnError( f );

    if( f.isFile() && f.ownerCanExecute() )
        return true;
    else
        return false;

}

bool
isSearchable( const std::string &dir )
{
    File f(dir);
    throwOnError( f );

    if( f.isDir() && f.ownerCanExecute() )
        return true;
    else
        return false;
}

bool
permissionToWrite( const std::string &path )
{
    File f(path);

    try {
        throwOnError( f );
    } catch( const permission_error &e) {
        return false;
    }
    return f.ownerCanWrite();

}

bool
permissionToRead( const std::string &path )
{
    File f(path);

    try {
        throwOnError( f );
    } catch( const permission_error &e) {
        return false;
    }
    return f.ownerCanRead();

}

}
}


