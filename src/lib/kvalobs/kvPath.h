/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvPath.h,v 1.1.2.2 2007/09/27 09:02:30 paule Exp $                                                       

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
#ifndef __kvPath_h__
#define __kvPath_h__

#include <string>

/**
 * kvPath returns the path to the named directory.
 * 
 * name may be one of:
 * 
 *  - prefix
 *  - pkglibdir     , eg $prefix/kvalobs              
 *  - sysconfdir    , eg $perfix/etc
 *  - libdir        , eg $prefix/lib
 *  - bindir        , eg $prefix/bin
 *  - datadir       , eg $prefix/share 
 *  - localstatedir , eg $prefix/var
 *  - logdir        , eg $localstatedir/log/kvalobs
 *  - rundir        , eg $localstatedir/run/kvalobs
 * 
 * @return The path to the 'name'. An empty string if name is not recogniced.
 * 
 */
std::string kvPath( const std::string &name, const std::string &system="kvalobs" );

#endif
