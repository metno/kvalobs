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

#ifndef LOGFILECREATOR_H_
#define LOGFILECREATOR_H_

#include <string>
#include <memory>
#include <iosfwd>

namespace kvalobs
{
class kvStationInfo;
}


namespace qabase
{

/**
 * Creates script log files.
 *
 * \ingroup group_control
 */
class LogFileCreator
{
public:

	/**
	 * Initialize LogFileCreator with a base directory to use for logs.
	 *
	 * @param baseDirectory
	 * @return
	 */
	explicit LogFileCreator(const std::string & baseDirectory);
	~LogFileCreator();

	typedef std::auto_ptr<std::ostream> LogStreamPtr;

	/**
	 * Create and return a file for logging
	 *
	 * @param observationToCheck Specification of the observation. A file path
	 * is generated from this.
	 * @return A pointer to a file stream where script logs may be written, or
	 * NULL if an error occurs when opening that file.
	 */
	LogStreamPtr getLogStream(const kvalobs::kvStationInfo & observationToCheck) const;

private:
	std::string baseDirectory_;
};

}

#endif /* LOGFILECREATOR_H_ */
