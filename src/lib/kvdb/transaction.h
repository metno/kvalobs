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

#ifndef __dnmi_db_transaction_h__
#define __dnmi_db_transaction_h__

namespace dnmi {
namespace db {
class Connection;

class Transaction{
public:
   virtual bool operator()(dnmi::db::Connection *conection);

   ///Called if the transaction is aborted.
   virtual void onAbort();

   ///Called if the operator() returns true;
   virtual void onSuccess();

   ///Called if the operator() returns false;
   virtual void onFailure();

   /**
    * Called before each retry of the transaction.
    * Do any cleanup before a new transaction is begin.
    */
   virtual void onRetry();

   /**
    * Called when max retry to perform the transaction is reached.
    * Default action is to throw SQLException
    * @throw SQLException
    */
   virtual void onMaxRetry( const std::string &lastError );
};


}
}


#endif
