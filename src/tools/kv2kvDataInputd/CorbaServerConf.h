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

#ifndef __CORBASERVERCONF_H__
#define __CORBASERVERCONF_H__

#include <stdexcept>
#include <string>
#include <iostream>
#include <corbahelper/corbaApp.h>

struct CorbaServerConf {
   std::string confName;
   std::string name;
   CorbaHelper::ServiceHost ns;

   CorbaServerConf(){}
   CorbaServerConf( const CorbaServerConf &nsc );

   CorbaServerConf& operator=( const CorbaServerConf &rhs );

   void clean();

   /**
    * confString is on the form name@corbanameserver:port
    *
    * Where name is the 'name' of the kvalobs server to receive
    * data. 'corbanameserver' is the CORBA nameservice to look
    * up the 'name'. Port is the CORBA nameservice port.
    *
    * The nameserver part is optional. If it is not given
    * the default nameserver is used.
    *
    * @return true on success and false otherwise.
    */
   bool decodeConfspec( const std::string &confString, const std::string &defaultNameserver );

   /**
    * confString is on the form name@corbanameserver:port
    *
    * Where name is the 'name' of the kvalobs server to receive
    * data. 'corbanameserver' is the CORBA nameservice to look
    * up the 'name'. Port is the CORBA nameservice port.
    *
    * The nameserver part is optional. If it is not given
    * the default nameserver is used.
    * @throw std::logic_error on failure.
    */

   static CorbaServerConf decode( const std::string &confString, const std::string &defaultNameserver );

   friend std::ostream& operator<<( std::ostream &out, const CorbaServerConf &sf );
};

std::ostream& operator<<( std::ostream &out, const CorbaServerConf &sf );



#endif
