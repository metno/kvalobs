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

#ifndef DELAYEDSAVEDATABASEACCESS_H_
#define DELAYEDSAVEDATABASEACCESS_H_

#include "FilteredDatabaseAccess.h"
#include <kvalobs/kvDataOperations.h>
#include <boost/noncopyable.hpp>
#include <set>

namespace db {

/**
 * Delays the effect of write() until commit() is called. The get
 * Data function behaves as if data was stored in database.
 *
 * \ingroup group_db
 */
class DelayedSaveDatabaseAccess : public FilteredDatabaseAccess,
    boost::noncopyable {
 public:
  explicit DelayedSaveDatabaseAccess(DatabaseAccess * baseImplementation);

  virtual ~DelayedSaveDatabaseAccess();

  /**
   * Perform all previous writes
   */
  virtual void commit();

  virtual void rollback();

  virtual void getData(DataList * out, const kvalobs::kvStationInfo & si,
                       const qabase::DataRequirement::Parameter & parameter,
                       int minuteOffset) const;

  virtual void write(const DataList & data);

  virtual bool commitIsNeccessary() const {
    return true;
  }

  typedef std::set<kvalobs::kvData, kvalobs::compare::lt_kvData> SavedData;
  const SavedData & uncommitted() const {
    return savedData_;
  }

 private:
  SavedData savedData_;

};

}

#endif /* DELAYEDSAVEDATABASEACCESS_H_ */
