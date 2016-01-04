/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: kvDbGate.cc,v 1.14.2.4 2007/09/27 09:02:30 paule Exp $

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

#include <kvalobs/DataInsertTransaction.h>

using namespace dnmi::db;
using namespace std;

#define INSERT_SAVEPOINT "INSERT_SAVEPOINT"
#define EX_SQLException     0
#define EX_SQLNotSupported  1
#define EX_SQLDuplicate     2
#define EX_SQLNotConnected  3
#define EX_SQLBusy          4
#define EX_SQLAborted       5
#define EX_ModeAction       6
#define EX_Exception        7

namespace kvalobs {

void DataInsertTransaction::reThrow() {
  switch (savedExcetion) {
    case EX_SQLException:
      throw SQLException(lastError, errorCode);
    case EX_SQLNotSupported:
      throw SQLNotSupported(lastError, errorCode);
    case EX_SQLDuplicate:
      throw SQLDuplicate(lastError, errorCode);
    case EX_SQLNotConnected:
      throw SQLNotConnected(lastError, errorCode);
    case EX_SQLBusy:
      throw SQLBusy(lastError, errorCode);
    case EX_SQLAborted:
      throw SQLAborted(lastError, errorCode);
    case EX_ModeAction:
      throw ModeException(index, lastError);
    case EX_Exception:
      //fall through to the default action.
    default:
      throw std::logic_error(lastError);
  }
}

DataInsertTransaction::DataInsertTransaction(
    const std::list<kvalobs::kvDbBase*> &elem, Action action_,
    const std::string &tblName_)
    : deleteElems(false),
      elems(&elem),
      tblName(tblName_),
      action(action_) {

}

DataInsertTransaction::~DataInsertTransaction() {
  if (deleteElems)
    delete elems;
}

bool DataInsertTransaction::insert(kvalobs::kvDbBase *data) {
  string savepoint(INSERT_SAVEPOINT);
  ostringstream ost;

  try {
    conection->createSavepoint(savepoint);
    ost << "INSERT INTO " << (tblName.empty() ? data->tableName() : tblName)
        << " VALUES" << data->toSend() << ";";

    conection->exec(ost.str());
    conection->releaseSavepoint(savepoint);
    return true;
  } catch (SQLDuplicate &ex) {
    conection->rollbackToSavepoint(savepoint);
    if (action == INSERTONLY) {
      ostringstream ost;
      ost << "INSERTONLY (duplicate) into '"
          << (tblName.empty() ? data->tableName() : tblName) << "' "
          << "with uniqueKey '" << data->uniqueKey() << "' failed.";
      fastExit = true;
      throw ModeException(index, ost.str());
    }
    return false;
  } catch (const std::exception &ex) {
    conection->rollbackToSavepoint(savepoint);
    throw;
  } catch (...) {
    conection->rollbackToSavepoint(savepoint);
    throw;
  }
}

void DataInsertTransaction::update(kvalobs::kvDbBase *data) {
  ostringstream ost;

  ost << "UPDATE " << (tblName.empty() ? data->tableName() : tblName) << " "
      << data->toUpdate();
  conection->exec(ost.str());
}

void DataInsertTransaction::replace(kvalobs::kvDbBase *data) {
  ostringstream ost;
  ost << "DELETE FROM " << (tblName.empty() ? data->tableName() : tblName)
      << " " << data->uniqueKey() << ";" << "INSERT INTO "
      << (tblName.empty() ? data->tableName() : tblName) << " VALUES"
      << data->toSend() << ";";
  conection->exec(ost.str());
}

bool DataInsertTransaction::run() {
  std::list<kvalobs::kvDbBase*>::const_iterator it;

  for (it = elems->begin(), index = 0; it != elems->end(); ++it, ++index) {
    if (!*it || !(*it)->exists()) {
      fastExit = true;

      if (*it) {
        ostringstream ost;
        ost << "InvalidObject: The kvDbBase object for table <"
            << (tblName.empty() ? (*it)->tableName() : tblName)
            << "> is invalid!";
        lastError = ost.str();
      } else
        lastError = "DataInsertTransaction::run: NULL pointer to a kvDbBase!";

      throw logic_error(lastError);
    }

    if (!insert(*it)) {
      if (action == INSERT_OR_UPDATE) {
        update(*it);
      } else {
        replace(*it);
      }
    }
  }

  return true;
}

void DataInsertTransaction::onAbort(const std::string &driverid,
                                    const std::string &errorMessage,
                                    const std::string &errorCode) {
}

void DataInsertTransaction::onSuccess() {
}

void DataInsertTransaction::onRetry() {
  if (fastExit) {
    reThrow();
  }
}

void DataInsertTransaction::onMaxRetry(const std::string &lastError) {
  reThrow();
}

bool DataInsertTransaction::operator()(dnmi::db::Connection *con) {
  index = 0;
  fastExit = false;
  lastError.erase();
  savedExcetion = -1;
  errorCode.erase();
  conection = con;

  if (elems->empty())
    return true;

  try {
    return run();
  } catch (const SQLNotSupported &ex) {
    savedExcetion = EX_SQLNotSupported;
    lastError = ex.what();
    errorCode = ex.errorCode();
    throw;
  } catch (const SQLDuplicate &ex) {
    savedExcetion = EX_SQLDuplicate;
    lastError = ex.what();
    errorCode = ex.errorCode();
    throw;
  } catch (const SQLNotConnected &ex) {
    savedExcetion = EX_SQLNotConnected;
    lastError = ex.what();
    errorCode = ex.errorCode();
    throw;
  } catch (const SQLBusy &ex) {
    savedExcetion = EX_SQLBusy;
    lastError = ex.what();
    errorCode = ex.errorCode();
    throw;
  } catch (const SQLAborted &ex) {
    savedExcetion = EX_SQLAborted;
    lastError = ex.what();
    errorCode = ex.errorCode();
    throw;
  } catch (const ModeException &ex) {
    savedExcetion = EX_ModeAction;
    lastError = ex.what();
    throw;
  } catch (const SQLException &ex) {
    savedExcetion = EX_SQLException;
    lastError = ex.what();
    errorCode = ex.errorCode();
    throw;
  } catch (const exception &ex) {
    savedExcetion = EX_Exception;
    lastError = ex.what();
    throw;
  } catch (...) {
    savedExcetion = EX_Exception;
    lastError = "DataInsertTransaction::opeartor() UNKNOWN exception!";
    throw logic_error(lastError);
  }
}

}

