/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: transactionhelper.cc,v 1.1.2.3 2007/09/27 09:02:25 paule Exp $                                                       

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
#include "transactionhelper.h"
#include "kvdb.h"

namespace dnmi {
namespace db {

TransactionBlock::TransactionBlock(dnmi::db::Connection &con_)
    : abort_(true),
      inTransaction(false),
      con(&con_) {
  try {
    con->beginTransaction();
    inTransaction = true;
  } catch (...) {
  }
}

TransactionBlock::TransactionBlock(dnmi::db::Connection *con_, dnmi::db::Connection::IsolationLevel il, bool defaultDoAbort, bool doIgnoreTransactions)
  : abort_(defaultDoAbort),
      inTransaction(false),
      con(con_),
      ignore_(doIgnoreTransactions) 
{
  if ( ignore_)
    return;

  try {
    con->beginTransaction(il);
    inTransaction = true;
  } catch (...) {
  }
}


TransactionBlock::~TransactionBlock() {
  if (ignore_)
    return;

  try {
    if (inTransaction) {
      if (abort_)
        con->rollBack();
      else
        con->endTransaction();
    }
  } catch (...) {
  }
}
}
}