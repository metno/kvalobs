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

#include "LogFileCreator.h"
#include "Exception.h"
#include <kvalobs/kvStationInfo.h>
#include <milog/milog.h>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/lexical_cast.hpp>


namespace
{
QABASE_EXCEPTION(LogFileCreationError);

#if BOOST_FILESYSTEM_VERSION >= 3
    inline std::string to_native_file(const boost::filesystem::path& path) {
        return path.native();
    }
    inline std::string to_native_dir(const boost::filesystem::path& path) {
        return path.native();
    }
#else
    inline std::string to_native_file(const boost::filesystem::path& path) {
        return path.native_file_string();
    }
    inline std::string to_native_dir(const boost::filesystem::path& path) {
        return path.native_directory_string();
    }
#endif

boost::filesystem::path getLogFileName(const kvalobs::kvStationInfo & observationToCheck)
{
  std::ostringstream name;
  int hour = observationToCheck.obstime().time_of_day().hours();
  name << "log.kl";
  if ( hour < 10 )
    name << 0;
  name << hour << '.' << observationToCheck.typeID() << ".log";
  	return name.str();
	//return "log-" + observationToCheck.obstime().isoClock() + ".log";
}

boost::filesystem::path getLogFile(const kvalobs::kvStationInfo & observationToCheck, const std::string & baseLogDir)
{
	const boost::filesystem::path base(baseLogDir);

	if ( not exists(base) and ! create_directories(base) )
		throw LogFileCreationError("Unable to create base logging folder " + base.string() );

	if ( is_directory(base) )
	{
		boost::filesystem::path logFile(base);
		logFile /= boost::lexical_cast<std::string>(observationToCheck.stationID());
		logFile /= to_iso_extended_string(observationToCheck.obstime().date());

		if ( not exists(logFile) and ! create_directories(logFile) )
			throw LogFileCreationError("Unable to create logging folder " + logFile.string() );
		logFile /= getLogFileName(observationToCheck);
		return logFile;
	}
	else
		return base;
}

void renameOldLogs(const boost::filesystem::path & logFile)
{
	if ( exists(logFile) )
	{
		std::string newName = logFile.string();
		try
		{
			std::string::size_type pos = newName.find_last_of('.');
			if ( pos == std::string::npos )
				pos = newName.size() -1;
			unsigned logVersion = boost::lexical_cast<unsigned>(newName.substr(pos +1));
			if ( logVersion >= 9 ) // max number of log files
			{
				remove(logFile);
				return;
			}
			newName = newName.substr(0, pos +1) + boost::lexical_cast<std::string>(logVersion +1);
		}
		catch ( boost::bad_lexical_cast &)
		{
			newName += ".1";
		}
		renameOldLogs(newName);
		rename(logFile, newName);
	}
}
}

namespace qabase
{

LogFileCreator::LogFileCreator(const std::string & baseDirectory) :
		baseDirectory_(baseDirectory)
{

}

LogFileCreator::~LogFileCreator()
{
}

LogFileCreator::LogStreamPtr LogFileCreator::getLogStream(const kvalobs::kvStationInfo & observationToCheck) const
{
	milog::LogContext context("scriptlog");

	LOGDEBUG("Logging to base directory " + baseDirectory_);

	if ( not baseDirectory_.empty() )
	{
		try
		{
			boost::filesystem::path logFile = getLogFile(observationToCheck, baseDirectory_);
			renameOldLogs(logFile);
			LogStreamPtr logStream(new boost::filesystem::ofstream(logFile));
			if ( not logStream->good() )
				throw LogFileCreationError("Error when opening log file: " + to_native_file(logFile));
			return logStream;
		}
		catch ( std::exception & e )
		{
			LOGERROR(e.what());
			// continue this function
		}
	}

	// If things fail...
	LOGINFO("No script logging");
	return LogStreamPtr();
}


}
