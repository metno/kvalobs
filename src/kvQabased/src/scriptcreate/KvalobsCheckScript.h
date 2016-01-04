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

#ifndef KVALOBSCHECKSCRIPT_H_
#define KVALOBSCHECKSCRIPT_H_

#include <db/DatabaseAccess.h>
#include <boost/scoped_ptr.hpp>

namespace scriptrunner {
class Script;
}

namespace qabase {
class DataStore;

/**
 * \defgroup group_scriptcreate Script creation
 *
 * Classes and functions for generating kvalobs check scripts, and parsing
 * return values from those. From a user's viewpoint, only the
 * KvalobsCheckScript class is interesting here.
 */

/**
 * Wraps a scriptrunner::Script with kvalobs-specific stuff.
 *
 * \ingroup group_scriptcreate
 */
class KvalobsCheckScript : boost::noncopyable {
 public:

  /**
   * Construct kvalobs script
   *
   * @param database The database connection to use for getting source data
   * @param obs observation to check
   * @param check The check to run
   * @param scriptLog stream to send script log, or NULL if no logging is to
   *                  be done.
   */
  KvalobsCheckScript(const db::DatabaseAccess & database,
                     const kvalobs::kvStationInfo & obs,
                     const kvalobs::kvChecks & check, std::ostream * scriptLog =
                         0);

  ~KvalobsCheckScript();

  /**
   * Get a runnable version of the given script.
   *
   * @return The script that this object represents, as a string.
   */
  std::string str() const;

  /**
   * Run the script
   *
   * @param modificationsOut Any modified data goes into this list.
   */
  void run(db::DatabaseAccess::DataList * modificationsOut);

 private:
  boost::scoped_ptr<DataStore> store_;
  boost::scoped_ptr<scriptrunner::Script> script_;

  void terminateLogEntry_() const;

  std::ostream * scriptLog_;
};

}

#endif /* KVALOBSCHECKSCRIPT_H_ */
