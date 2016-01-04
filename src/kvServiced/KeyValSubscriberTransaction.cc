/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: kvSubscriberCollection.cc,v 1.2.6.3 2007/09/27 09:02:39 paule Exp $

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

#include <boost/algorithm/string.hpp>
#include "KeyValSubscriberTransaction.h"

using namespace std;
using namespace kvalobs;

bool KeyValSubscriberTransaction::KeyValSubscriberTransaction::insert(
    const std::string &val) {
  ostringstream q;
  q << "INSERT INTO key_val VALUES(" << SubScriberID << ",'" << subscriberid
    << "', '" << val << "')";
  err << "query: <" << q.str() << ">. ";
  con->exec(q.str());  //Throws on error
  return true;
}

bool KeyValSubscriberTransaction::update(const std::string &val) {
  ostringstream q;
  q << "UPDATE key_val" << " SET val='" << val << "'" << " WHERE package="
    << SubScriberID << " AND key='" << subscriberid << "'";
  err << "query: <" << q.str() << ">. ";
  con->exec(q.str());  //Throws on error
  return true;
}

bool KeyValSubscriberTransaction::get(std::list<kvalobs::kvKeyVal> &vals,
                                      const std::string &subid) {
  vals.clear();
  ostringstream q;
  q << "SELECT * FROM key_val" << " WHERE package=" << SubScriberID;

  if (!subid.empty())
    q << " AND key='" << subid << "'";
  ;

  err << "query: <" << q.str() << ">. ";
  dnmi::db::Result *res = con->execQuery(q.str());  //Throws on error

  if (!res)
    return true;

  while (res->hasNext()) {
    vals.push_back(kvKeyVal(res->next()));
  }

  return true;
}

bool KeyValSubscriberTransaction::insertOrUpdate() {
  if (subscriberid.empty()) {
    err << "Subscriberid cant be empty!";
    return false;
  }

  get(keyVals, subscriberid);

  if (keyVals.empty())
    return insert(content);
  else
    return update(content);
}

bool KeyValSubscriberTransaction::updateLastCall() {
  const std::string prefix("Last call: ");

  if (subscriberid.empty()) {
    err << "Subscriberid cant be empty!";
    return false;
  }
  if (lastCall.undef()) {
    err << "Invalid timestamp!";
    return false;
  }

  get(keyVals, subscriberid);

  if (keyVals.empty()) {
    err << "No subscribers with id <" << subscriberid << ">!";
    return false;
  }

  string val(keyVals.begin()->val());
  string::size_type i = val.find(prefix);
  string::size_type ii;

  if (i == string::npos) {
    err << "Invalid format. Expecting <" << prefix << ">!";
    return false;
  }
  i += prefix.length();
  ii = val.find("\n", i);
  if (ii == string::npos) {
    err << "Invalid format. Expecting eol after <" << prefix << ">!";
    return false;
  }

  val.replace(i, ii - i, lastCall.isoTime());
  return update(val);
}

bool KeyValSubscriberTransaction::removeSubscriber() {
  ostringstream q;
  q << "DELETE FROM key_val" << " WHERE package=" << SubScriberID;

  if (!subscriberid.empty())
    q << " AND key='" << subscriberid << "'";
  ;

  err << "query: <" << q.str() << ">. ";
  con->exec(q.str());  //Throws on error
  return true;
}

KeyValSubscriberTransaction::KeyValSubscriberTransaction(
    const std::string &subid, const miutil::miTime &lastCall_)
    : action(UPDATE_LAST_CALL),
      SubScriberID("'KvServiceSubcriberIDS'"),
      subscriberid(subid),
      lastCall(lastCall_),
      con(0),
      ok(false) {
}

KeyValSubscriberTransaction::KeyValSubscriberTransaction(
    const std::string &subid, const std::string &content_)
    : action(INSERT_OR_UPDATE),
      SubScriberID("'KvServiceSubcriberIDS'"),
      subscriberid(subid),
      content(content_),
      con(0),
      ok(false) {
}

KeyValSubscriberTransaction::KeyValSubscriberTransaction(
    const std::string &subid, Action act)
    : action(act),
      SubScriberID("'KvServiceSubcriberIDS'"),
      subscriberid(subid),
      con(0),
      ok(false) {
}

KeyValSubscriberTransaction::~KeyValSubscriberTransaction() {
}

bool KeyValSubscriberTransaction::operator()(dnmi::db::Connection *conection) {
  ok = false;
  con = conection;
  switch (action) {
    case INSERT_OR_UPDATE:
      err << "<INSERT_OR_UPDATE> ";
      ok = insertOrUpdate();
      break;
    case UPDATE_LAST_CALL:
      err << "<UPDATE_LAST_CALL> ";
      ok = updateLastCall();
      break;
    case DELETE_SUBSCRIBER:
      err << "<DELETE_SUBSCRIBER> ";
      ok = removeSubscriber();
      break;
    case GET_SUBSCRIBER:
      err << "<GET_SUBSCRIBER> ";
      ok = get(keyVals, subscriberid);
      break;
  }
  return true;
}

void KeyValSubscriberTransaction::onAbort(const std::string &driverid,
                                          const std::string &errorMessage,
                                          const std::string &errorCode) {
}

void KeyValSubscriberTransaction::onSuccess() {
  msg = "KeyValTransaction:OK " + err.str();
}

void KeyValSubscriberTransaction::onRetry() {
  err.str("");
}

void KeyValSubscriberTransaction::onMaxRetry(const std::string &lastError) {
  msg = "KeyValTransaction:FAILED " + err.str();
}
