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

#include "HtmlLog.h"
#include <milog/milog.h>
#include <kvalobs/kvStationInfo.h>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/lexical_cast.hpp>

using namespace std;
using namespace kvalobs;
using namespace milog;
using boost::filesystem::path;


namespace
{
path logPath(const kvalobs::kvStationInfo & stinfo, const path & start_logpath)
{
	path log_dir(start_logpath);
	const path stationDirectory = boost::lexical_cast<string>(stinfo.stationID());

	const std::string obsTime = stinfo.obstime().isoDate();
	log_dir /= stationDirectory / obsTime;

	boost::filesystem::create_directories(log_dir);
	return log_dir;
}

path logfilename(const std::string & clock, int version)
{
	ostringstream filename;
	filename << "log-" << clock;
	if (version)
		filename << '_' << version;
	filename << ".html";
	return filename.str();
}

path getLogfilePath(const kvalobs::kvStationInfo & stinfo,
		const path & start_logpath)
{
	namespace fs = boost::filesystem;

	fs::path log_dir = logPath(stinfo, start_logpath);

	std::string clock = stinfo.obstime().isoClock();
	std::replace(clock.begin(), clock.end(), ':', '-');

	fs::path log_file;
	for (int i = 8; i >= 0; --i)
	{
		log_file = log_dir / logfilename(clock, i);
		if (fs::exists(log_file))
		{
			fs::path rename_to = log_dir / logfilename(clock, i + 1);
			fs::remove(rename_to);
			fs::rename(log_file, rename_to);
		}
	}
	return log_file.native_directory_string();
}
}

HtmlLog::HtmlLog(const kvalobs::kvStationInfo & stinfo, const std::string & logPath)
{
	html = new HtmlStream;

	path logfile = getLogfilePath(stinfo, logPath);

	LOGINFO( "CheckRunner::runChecks for station:" << stinfo.stationID()
			<< " and obstime:" << stinfo.obstime() << endl
			<< "Logging all activity to:" << logfile.native_file_string() << endl );

	if ( logPath.empty() )
		html->open("/dev/null");
	else
	{
		if (!html->open(logfile.native_file_string()))
		{
			LOGERROR("Failed to create logfile for the html output. Filename:\n" << logfile.native_file_string());
			html->open("/dev/null");
		}
	}

	Logger::createLogger("html", html);
	Logger::logger("html").logLevel(DEBUG);

	Logger::logger("html").debug("hallo?");

	IDLOGINFO( "html", "<h1>"
			<< "CheckRunner::runChecks for station:" << stinfo.stationID()
			<< " and obstime:" << stinfo.obstime()
			<< "</h1>" << endl );
	IDLOGINFO( "html", "<CODE><PRE>" );
}

HtmlLog::~HtmlLog()
{
	IDLOGINFO( "html", "</PRE></CODE>" );
	Logger::removeLogger("html");
}

void HtmlLog::operator () (const std::string & msg, LogType t)
{
	switch (t)
	{
	case Debug:
		IDLOGDEBUG( "html", msg << endl )
		;
		break;
	case Info:
		IDLOGINFO( "html", msg << endl )
		;
		break;
	case Warn:
		IDLOGWARN( "html", msg << endl )
		;
		break;
	case Error:
		IDLOGERROR( "html", msg << endl )
		;
		break;
	}
}
