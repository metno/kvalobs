/*
  Kvalobs - Free Quality Control Software for Meteorological Observations

  $Id: decoder.h,v 1.1.2.2 2007/09/27 09:02:27 paule Exp $

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

#include <kvdb/kvdb.h>
#include <kvdb/transaction.h>

namespace dnmi {
namespace db {

bool
Transaction::
operator()(dnmi::db::Connection *conection)
{
   throw SQLException( "Transaction: operator()( dnmi::db::Connection *conection ) NOT IMPLEMENTED!");
}

///Called if the transaction is aborted.
void
Transaction::
onAbort( const std::string &driverid,
         const std::string &errorMessage,
         const std::string &errorCode )
{
}

void
Transaction::
onSuccess()
{
}

void
Transaction::
onFailure()
{
}

void
Transaction::
onRetry()
{
}

void
Transaction::
onMaxRetry( const std::string &lastError )
{
   std::string msg("Max retry reached.");

   if( ! lastError.empty() )
      msg=lastError;

   throw SQLException( lastError );
}

}
}

