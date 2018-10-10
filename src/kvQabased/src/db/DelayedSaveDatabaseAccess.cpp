/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 Copyright (C) 2010 met.no

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

#include "DelayedSaveDatabaseAccess.h"

namespace db {

DelayedSaveDatabaseAccess::DelayedSaveDatabaseAccess(
    DatabaseAccess * baseImplementation)
    : FilteredDatabaseAccess(baseImplementation) {
}

DelayedSaveDatabaseAccess::~DelayedSaveDatabaseAccess() {
}

void DelayedSaveDatabaseAccess::getData(
    DataList * out, const qabase::Observation & obs,
    const qabase::DataRequirement::Parameter & parameter,
    int minuteOffset) const {
  DataList fromDb;
  FilteredDatabaseAccess::getData(&fromDb, obs, parameter, minuteOffset);

  for (DataList::iterator it = fromDb.begin(); it != fromDb.end(); ++it) {
    SavedData::const_iterator alreadySaved = savedData_.find(*it);
    if (alreadySaved == savedData_.end())
      out->push_back(*it);
    else
      out->push_back(*alreadySaved);
  }
}

void DelayedSaveDatabaseAccess::write(const DataList & data) {
  for (DataList::const_iterator it = data.begin(); it != data.end(); ++it) {
    savedData_.erase(*it);
    savedData_.insert(*it);
  }
}

void DelayedSaveDatabaseAccess::commit() {
  if (not savedData_.empty()) {
    DataList saveData(savedData_.begin(), savedData_.end());
    FilteredDatabaseAccess::write(saveData);
    FilteredDatabaseAccess::commit();
    savedData_.clear();
  } else
    FilteredDatabaseAccess::commit();
}

void DelayedSaveDatabaseAccess::rollback() {
  savedData_.clear();
  FilteredDatabaseAccess::rollback();
}

}
