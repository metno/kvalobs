/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: dummysqldb.cc,v 1.1.2.2 2007/09/27 09:02:26 paule Exp $                                                       

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
#include "dummysqldb.h"

using namespace std;

dnmi::db::drivers::DummyDriver::DummyDriver() {
}

dnmi::db::drivers::DummyDriver::~DummyDriver() {
}

dnmi::db::Connection*
dnmi::db::drivers::DummyDriver::createConnection(const std::string &connect) {
  return new DummyConnection(this->name());
  //throw SQLNotSupported("createConnection: Not supported!");
}

bool dnmi::db::drivers::DummyDriver::releaseConnection(Connection *connect) {
  delete connect;
  return true;
  //throw SQLNotSupported("releaseConnection: Not supported!");
}

dnmi::db::drivers::DummyConnection::DummyConnection()
    : Connection("") {
}

dnmi::db::drivers::DummyConnection::~DummyConnection() {
}

dnmi::db::drivers::DummyConnection::DummyConnection(const std::string &id)
    : Connection(id) {
}

bool dnmi::db::drivers::DummyConnection::isConnected() {
  return false;
}

bool dnmi::db::drivers::DummyConnection::tryReconnect() {
  throw SQLNotSupported("tryReconnection: Not supported!");
}

void dnmi::db::drivers::DummyConnection::beginTransaction() {
  throw SQLNotSupported("beginTransaction: Not supported!");
}

void dnmi::db::drivers::DummyConnection::endTransaction() {
  throw SQLNotSupported("endTransaction: Not supported!");
}

void dnmi::db::drivers::DummyConnection::rollBack() {
  throw SQLNotSupported("rollBack: Not supported!");
}

void dnmi::db::drivers::DummyConnection::exec(const std::string &query) {
  throw SQLNotSupported("exec: Not supported!");
}

dnmi::db::Result*
dnmi::db::drivers::DummyConnection::execQuery(const std::string &query) {
  throw SQLNotSupported("execQuery: Not supported!");
}

std::string dnmi::db::drivers::DummyConnection::lastError() const {
  throw SQLNotSupported("lastError: Not supported!");
}

std::string dnmi::db::drivers::DummyConnection::esc(
    const std::string &stringToEscape) const {
  return stringToEscape;
}

dnmi::db::drivers::DummyResult::~DummyResult() {
}

int dnmi::db::drivers::DummyResult::fields() const {
  throw SQLNotSupported();
}

std::string dnmi::db::drivers::DummyResult::fieldName(int index) const {
  throw SQLNotSupported();
}

int dnmi::db::drivers::DummyResult::fieldIndex(
    const std::string &fieldName) const {
  throw SQLNotSupported();
}

dnmi::db::FieldType dnmi::db::drivers::DummyResult::fieldType(int index) const {
  throw SQLNotSupported();
}

dnmi::db::FieldType dnmi::db::drivers::DummyResult::fieldType(
    const std::string &fieldName) const {
  throw SQLNotSupported();
}

int dnmi::db::drivers::DummyResult::fieldSize(int index) const {
  throw SQLNotSupported();
}

int dnmi::db::drivers::DummyResult::fieldSize(
    const std::string &fieldName) const {
  throw SQLNotSupported();
}

int dnmi::db::drivers::DummyResult::size() const {
  throw SQLNotSupported();
}

bool dnmi::db::drivers::DummyResult::hasNext() const {
  throw SQLNotSupported();
}

void dnmi::db::drivers::DummyResult::nextImpl() {
  throw SQLNotSupported();
}
