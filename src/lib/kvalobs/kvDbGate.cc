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
#include <kvalobs/kvDbGate.h>
#include <sstream>
#include <time.h>
// #include <milog/milog.h>

using namespace std;
using namespace dnmi::db;

/* Created by met.no/IT: borge.moe@met.no
 at Wed Aug 28 13:49:02 2002 */

bool kvalobs::kvDbGate::begintransaction() {
  time_t timeout;
  time_t now;
  bool busy = true;

  time(&now);
  timeout = now + busytimeout_;

  while (timeout >= now && busy) {
    busy = false;

    try {
      con->beginTransaction();
    } catch (dnmi::db::SQLNotConnected &ex) {
      err_ = NotConnected;
      errStr_ = ex.what();
      return false;
    } catch (dnmi::db::SQLBusy &ex) {
      err_ = Busy;
      errStr_ = ex.what();
      busy = true;
      time(&now);
    } catch (dnmi::db::SQLException &ex) {
      err_ = Error;
      errStr_ = ex.what();

      return false;
    }
  }

  return !busy;
}

bool kvalobs::kvDbGate::endtransaction() {
  time_t timeout;
  time_t now;
  bool busy = true;

  time(&now);
  timeout = now + busytimeout_;

  while (timeout >= now && busy) {
    busy = false;

    try {
      con->endTransaction();
    } catch (dnmi::db::SQLNotConnected &ex) {
      err_ = NotConnected;
      errStr_ = ex.what();
      return false;
    } catch (dnmi::db::SQLBusy &ex) {
      err_ = Busy;
      errStr_ = ex.what();
      busy = true;
      time(&now);
    } catch (dnmi::db::SQLException &ex) {
      err_ = Error;
      errStr_ = ex.what();

      return false;
    }
  }

  return !busy;
}

bool kvalobs::kvDbGate::rollback() {
  time_t timeout;
  time_t now;
  bool busy = true;

  time(&now);
  timeout = now + busytimeout_;

  while (timeout >= now && busy) {
    busy = false;

    try {
      con->rollBack();
    } catch (dnmi::db::SQLNotConnected &ex) {
      err_ = NotConnected;
      errStr_ = ex.what();
      return false;
    } catch (dnmi::db::SQLBusy &ex) {
      err_ = Busy;
      errStr_ = ex.what();
      busy = true;
      time(&now);
    } catch (dnmi::db::SQLException &ex) {
      err_ = Error;
      errStr_ = ex.what();

      return false;
    }
  }

  return !busy;
}

bool kvalobs::kvDbGate::insertImpl(const std::list<kvalobs::kvDbBase *> &elem,
                                   const std::string &tblName, bool replace) {
  InsertOption action(INSERTONLY);

  if (replace)
    action = INSERT_OR_REPLACE;

  return insertImpl(elem, tblName, action);
}

bool kvalobs::kvDbGate::insertImpl(const std::list<kvalobs::kvDbBase *> &elem,
                                   const std::string &tblName,
                                   InsertOption option) {
  DataInsertTransaction::Action action(DataInsertTransaction::INSERTONLY);

  if (option == INSERT_OR_REPLACE)
    action = DataInsertTransaction::INSERT_OR_REPLACE;
  else if (option == INSERT_OR_UPDATE)
    action = DataInsertTransaction::INSERT_OR_UPDATE;

  DataInsertTransaction transaction(elem, action, tblName);

  try {
    con->perform(transaction, 3, dnmi::db::Connection::SERIALIZABLE);
    return true;
  } catch (const dnmi::db::SQLDuplicate &ex) {
    err_ = Duplicate;
    errStr_ = string("Duplicate: [") + ex.what() + string("]");
    return false;

  } catch (const dnmi::db::SQLNotConnected &ex) {
    err_ = NotConnected;
    errStr_ = string("NotConnected: [") + ex.what() + string("]");
    return false;
  } catch (const dnmi::db::SQLBusy &ex) {
    err_ = Busy;
    errStr_ = string("Busy: [") + ex.what() + string("]");
    return false;
  } catch (const dnmi::db::SQLAborted &ex) {
    err_ = Aborted;
    errStr_ = string("Aborted: [") + ex.what() + string("]");
    return false;
  } catch (const dnmi::db::SQLException &ex) {
    err_ = Error;
    errStr_ = string("Error: [") + ex.what() + string("]");
    return false;
  } catch (const std::exception &ex) {
    err_ = Error;
    errStr_ = string("Error: ") + ex.what() + string("]");
    return false;
  } catch (...) {
    err_ = UnknownError;
    errStr_ = "UnknownError. Unknown exception.";
    return false;
  }
}

std::string kvalobs::kvDbGate::esc(const std::string &src) const {
  return con->esc(src);
}

bool kvalobs::kvDbGate::insert(const kvalobs::kvDbBase &elem,
                               const std::string &tblName, bool replace) {
  time_t timeout;
  time_t now;
  bool busy = true;
  ostringstream ost;

  err_ = NoError;

  if (!elem.exists()) {
    err_ = InvalidObject;
    errStr_ = "InvalidObject: The kvDbBase object for table <" + tblName +
              "> is invalid!";
    return false;
  }

  ost << "INSERT INTO " << tblName << " VALUES" << elem.toSend() << ";";

  // LOGDEBUG("INSERT: " << endl << "[" << ost.str() << "]" << endl);

  for (int i = 0; i < 2; i++) {
    time(&now);
    timeout = now + busytimeout_;
    busy = true;

    while (timeout >= now && busy) {
      busy = false;

      try {
        con->exec(ost.str());
        return true;
      } catch (dnmi::db::SQLDuplicate &ex) {
        if (replace) {
          time(&now);
          timeout = now + busytimeout_;
          busy = true;

          while (timeout >= now && busy) {
            busy = false;

            try {
              ostringstream o;
              o << "DELETE FROM " << tblName << " " << elem.uniqueKey();
              // LOGDEBUG("[" << o.str() << "]");
              con->exec(o.str());
            } catch (dnmi::db::SQLBusy &ex) {
              err_ = Busy;
              errStr_ = string("Busy, Duplicate (replace): [") + ex.what() +
                        string("]");
              time(&now);
              busy = true;
            } catch (dnmi::db::SQLNotConnected &ex) {
              err_ = NotConnected;
              errStr_ = string("NotConnected, Duplicate (replace): [") +
                        ex.what() + string("]");
              return false;
            } catch (dnmi::db::SQLAborted &ex) {
              err_ = Aborted;
              errStr_ = string("Aborted, Duplicate (replace): [") + ex.what() +
                        string("]");
              return false;
            } catch (dnmi::db::SQLException &ex) {
              err_ = Error;
              errStr_ = string("Error, Duplicate (replace) : [") + ex.what() +
                        string("]");
              return false;
            } catch (...) {
              err_ = UnknownError;
              errStr_ = "UnknownError, Duplicate (replace): Unknown exception.";
              return false;
            }
          }

          if (busy)
            return false;
        } else {
          err_ = Duplicate;
          errStr_ = "Duplicate, Duplicate (no replace): Duplicate key!";
          return false;
        }
      } catch (dnmi::db::SQLNotConnected &ex) {
        err_ = NotConnected;
        errStr_ = string("NotConnected: [") + ex.what() + string("]");
        return false;
      } catch (dnmi::db::SQLBusy &ex) {
        err_ = Busy;
        errStr_ = string("Busy: [") + ex.what() + string("]");
        time(&now);
        busy = true;
      } catch (dnmi::db::SQLAborted &ex) {
        err_ = Aborted;
        errStr_ = string("Aborted: [") + ex.what() + string("]");
        return false;
      } catch (dnmi::db::SQLException &ex) {
        err_ = Error;
        errStr_ = string("Error: ") + ex.what() + string("]");
        return false;
      } catch (...) {
        err_ = UnknownError;
        errStr_ = "UnknownError. Unknown exception.";
        return false;
      }
    }
  }

  return !busy;
}

bool kvalobs::kvDbGate::insert(const kvalobs::kvDbBase &elem, bool replace) {
  return insert(elem, elem.tableName(), replace);
}

bool kvalobs::kvDbGate::insert(const kvalobs::kvDbBase &elem,
                               const char *tblName, bool replace) {
  return insert(elem, string(tblName), replace);
}

bool kvalobs::kvDbGate::update(const kvalobs::kvDbBase &elem) {
  return update(elem, elem.tableName());
}

bool kvalobs::kvDbGate::update(const kvalobs::kvDbBase &elem,
                               const char *tblName) {
  return update(elem, string(tblName));
}

bool kvalobs::kvDbGate::update(const kvalobs::kvDbBase &elem,
                               const std::string &tblName) {
  ostringstream ost;
  time_t timeout;
  time_t now;
  bool busy = true;

  time(&now);
  timeout = now + busytimeout_;

  ost << "UPDATE " << tblName << " " << elem.toUpdate();

  while (timeout >= now && busy) {
    busy = false;

    try {
      con->exec(ost.str());
      return true;
    } catch (dnmi::db::SQLBusy &ex) {
      err_ = Busy;
      errStr_ = string("Busy: [") + ex.what() + string("]");
      time(&now);
      busy = true;
    } catch (dnmi::db::SQLNotConnected &ex) {
      err_ = NotConnected;
      errStr_ = string("NotConnected, Duplicate (replace): [") + ex.what() +
                string("]");
      return false;
    } catch (dnmi::db::SQLAborted &ex) {
      err_ = Aborted;
      errStr_ =
          string("Aborted, Duplicate (replace): [") + ex.what() + string("]");
      return false;
    } catch (dnmi::db::SQLException &ex) {
      err_ = Error;
      errStr_ =
          string("Error, Duplicate (replace) : [") + ex.what() + string("]");
      return false;
    } catch (...) {
      err_ = UnknownError;
      errStr_ = "UnknownError, Duplicate (replace): Unknown exception.";
      return false;
    }
  }

  return false;
}

bool kvalobs::kvDbGate::replace(const kvalobs::kvDbBase &elem) {
  return replace(elem, elem.tableName());
}

bool kvalobs::kvDbGate::replace(const kvalobs::kvDbBase &elem,
                                const char *tblName) {
  return replace(elem, string(tblName));
}

bool kvalobs::kvDbGate::replace(const kvalobs::kvDbBase &elem,
                                const std::string &tblName) {
  ostringstream del;
  ostringstream ins;
  time_t timeout;
  time_t now;
  bool busy;

  err_ = NoError;

  if (!elem.exists()) {
    err_ = InvalidObject;
    errStr_ = "The kvDbBase object for table <" + tblName + "> is invalid!";
    return false;
  }

  ins << "INSERT INTO " << tblName << " VALUES" << elem.toSend() << ";";

  del << "DELETE FROM " << tblName << " " << elem.uniqueKey();

  if (!begintransaction()) {
    return false;
  }

  time(&now);
  timeout = now + busytimeout_;
  busy = true;

  while (timeout >= now && busy) {
    busy = false;

    try {
      con->exec(del.str());
    } catch (dnmi::db::SQLBusy &ex) {
      err_ = Busy;
      errStr_ = ex.what();
      busy = true;
      time(&now);
    } catch (dnmi::db::SQLNotConnected &ex) {
      err_ = NotConnected;
      errStr_ = ex.what();
      return false;
    } catch (dnmi::db::SQLException &ex) {
      endtransaction();
      err_ = Error;
      errStr_ = ex.what();

      return false;
    }
  }

  if (busy) {
    return false;
  }

  time(&now);
  timeout = now + busytimeout_;
  busy = true;

  while (timeout >= now && busy) {
    busy = false;

    try {
      con->exec(ins.str());
    } catch (dnmi::db::SQLNotConnected &ex) {
      err_ = NotConnected;
      errStr_ = ex.what();
      return false;
    } catch (dnmi::db::SQLBusy &ex) {
      err_ = Busy;
      errStr_ = ex.what();
      busy = true;
      time(&now);
    } catch (dnmi::db::SQLException &ex) {
      rollback();
      err_ = Error;
      errStr_ = ex.what();

      return false;
    }
  }

  if (busy) {
    rollback();
    errStr_ = "Busy!";
    err_ = Busy;
    return false;
  }

  return endtransaction();
}

bool kvalobs::kvDbGate::remove(const kvalobs::kvDbBase &elem) {
  return remove(elem, elem.tableName());
}

bool kvalobs::kvDbGate::remove(const kvalobs::kvDbBase &elem,
                               const char *tblName) {
  return remove(elem, string(tblName));
}

bool kvalobs::kvDbGate::remove(const kvalobs::kvDbBase &elem,
                               const std::string &tblName) {
  time_t timeout;
  time_t now;
  bool busy;

  ostringstream o;

  err_ = NoError;

  if (!elem.exists()) {
    err_ = InvalidObject;
    errStr_ = "The kvDbBase object for table <" + tblName + "> is invalid!";
    return false;
  }

  o << "DELETE FROM " << tblName << " " << elem.uniqueKey();

  time(&now);
  timeout = now + busytimeout_;
  busy = true;

  while (timeout >= now && busy) {
    busy = false;

    try {
      con->exec(o.str());
      return true;
    } catch (dnmi::db::SQLNotConnected &ex) {
      err_ = NotConnected;
      errStr_ = ex.what();
      return false;
    } catch (dnmi::db::SQLBusy &ex) {
      err_ = Busy;
      errStr_ = ex.what();
      busy = true;
      time(&now);
    } catch (dnmi::db::SQLException &ex) {
      err_ = Error;
      errStr_ = ex.what();

      return false;
    }
  }

  return false;
}

bool kvalobs::kvDbGate::remove(const std::string &query) {
  time_t timeout;
  time_t now;
  bool busy;

  ostringstream o;

  err_ = NoError;

  o << "DELETE FROM " << query;

  time(&now);
  timeout = now + busytimeout_;
  busy = true;

  while (timeout >= now && busy) {
    busy = false;

    try {
      con->exec(o.str());
      return true;
    } catch (dnmi::db::SQLNotConnected &ex) {
      err_ = NotConnected;
      errStr_ = ex.what();
      return false;
    } catch (dnmi::db::SQLBusy &ex) {
      err_ = Busy;
      errStr_ = ex.what();
      time(&now);
      busy = true;
    } catch (dnmi::db::SQLException &ex) {
      err_ = Error;
      errStr_ = ex.what();

      return false;
    }
  }

  return false;
}

bool kvalobs::kvDbGate::exec(const std::string &query) {
  time_t timeout;
  time_t now;
  bool busy;

  err_ = NoError;

  time(&now);
  timeout = now + busytimeout_;
  busy = true;

  while (timeout >= now && busy) {
    busy = false;

    try {
      con->exec(query);
      return true;
    } catch (dnmi::db::SQLNotConnected &ex) {
      err_ = NotConnected;
      errStr_ = ex.what();
      return false;
    } catch (dnmi::db::SQLBusy &ex) {
      err_ = Busy;
      errStr_ = ex.what();
      busy = true;
      time(&now);
    } catch (dnmi::db::SQLDuplicate &ex) {
      err_ = Duplicate;
      errStr_ = ex.what();
      return false;
    } catch (dnmi::db::SQLException &ex) {
      err_ = Error;
      errStr_ = ex.what();

      return false;
    }
  }

  return false;
}
