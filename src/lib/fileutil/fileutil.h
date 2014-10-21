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
#ifndef __FILEUTIL_H__
#define __FILEUTIL_H__

#include <errno.h>
#include <stdexcept>

namespace miutil {
namespace file {

class file_error : public std::exception
{
    int errno_;
    std::string what_;

public:
    file_error( int errno_, const std::string &what_ );
    file_error( int errno_ );
    virtual ~file_error() throw(){}
    int getErrno() const { return errno_; }
    virtual const char* what() const throw(){ return what_.c_str(); }
};

class permission_error : public file_error
{
public:
    permission_error( const std::string &what=""):file_error( EACCES, what ){}

};

class path_error : public file_error
{
public:
    path_error( int errno_, const std::string &what=""):file_error( errno_, what ){}
};

/**
 * @throw path_error, permission_error, file_error
 */
bool isdir( const std::string &dir );
/**
 * @throw path_error, permission_error, file_error
 */
bool isfile( const std::string &dir );
/**
 * @throw path_error, permission_error, file_error
 */
bool isRunable( const std::string &file );
/**
 * @throw path_error, permission_error, file_error
 */
bool isSearchable( const std::string &dir );
/**
 * @throw path_error, file_error
 */
bool permissionToWrite( const std::string &path );
/**
 * @throw path_error, file_error
 */
bool permissionToRead( const std::string &path );

}

}



#endif /* FILEUTIL_H_ */
