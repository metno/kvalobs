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

#include "AgregatorConfiguration.h"
#include <kvalobs/kvPath.h>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <string>
#include <iostream>

using namespace boost::program_options;

void AgregatorConfiguration::setup_()
{
	options_description mode("Working mode");

	mode.add_options()
			("back-production,b", value<std::string>(), "Produce data according to the given specification. Format for specification is '2008-04-08T06:00:00,5', which means produce data valid for period 2008-04-08T06:00:00 - 2008-04-08T11:00:00. Daemon mode will not be entered if this option is given.")
			("daemon-mode,d", "Enter daemon mode, even if overridden by the --back-production option.")
			("stations,s", value<std::vector<std::string> >(), "Only process stations from the given comma-separated list.")
			("parameter,p", value<std::string>(), "Only process parameters from the given comma-separated list.")
	;
	availableOptions_.add(mode);

	options_description database("Database");
	bool repopulate;
	database.add_options()
		("proxy-database-name", value<std::string>(), "Use the given file as proxy database. If the file does not exist, it will be created")
		("repopulate,r", "Repopulate internal agragator database on startup")
	;
	availableOptions_.add(database);

	options_description general("General");
	general.add_options()
			("help", "Get help message")
			("version",	"Display version information")
	;
	availableOptions_.add(general);
}


AgregatorConfiguration::AgregatorConfiguration() :
		messageStream_(std::cout), errorStream_(std::clog), availableOptions_("Available options")
{
	setup_();
}

AgregatorConfiguration::AgregatorConfiguration(std::ostream & messageStream, std::ostream & errorStream) :
		messageStream_(messageStream), errorStream_(errorStream), availableOptions_("Available options")
{
	setup_();
}

AgregatorConfiguration::~AgregatorConfiguration()
{
}

namespace
{
template<class Iterator>
void createIntList(Iterator out, const std::string & csv)
{
	using namespace boost;

	split_iterator<std::string::const_iterator> it = make_split_iterator(csv, first_finder(","));
	for ( ; it != split_iterator<std::string::const_iterator>(); ++ it )
	{
		* out = lexical_cast<int>(*it);
		++ out;
	}
}

template<class Iterator>
void getOptionList(Iterator out, const std::string & option, const variables_map & givenOptions)
{
	if ( givenOptions.count(option) )
	{
		typedef std::vector<std::string> StringList;
		const StringList & stList = givenOptions[option].as<StringList>();
		for ( StringList::const_iterator it = stList.begin(); it != stList.end(); ++ it )
			createIntList(out, * it);
	}
}
}

AgregatorConfiguration::ParseResult AgregatorConfiguration::parse(int & argc, char ** argv)
{
	try
	{

	//	parsed_options parsed =
	//			command_line_parser(argc, argv).options(availableOptions_).allow_unregistered().run();
	//	store(parsed, givenOptions_);
		store(parse_command_line(argc, argv, availableOptions_), givenOptions_);
		notify(givenOptions_);

		if ( givenOptions_.count("help") )
		{
			help(messageStream_);
			return Exit_Success;
		}

		getOptionList(std::back_inserter(stations_), "stations", givenOptions_);
		getOptionList(std::back_inserter(parameters_), "parameter", givenOptions_);

		return No_Action;
	}
	catch ( std::exception & e )
	{
		errorStream_ << "Error when parsing command line options: " << e.what() << std::endl;
		return Exit_Failure;
	}
}

std::ostream & AgregatorConfiguration::version(std::ostream & s) const
{
	return s << "kvAgregated (kvalobs) " << VERSION << std::endl;
}


std::ostream & AgregatorConfiguration::help(std::ostream & s) const
{
	return version(s) << "\nData agregation daemon for kvalobs.\n\n" << availableOptions_ << std::endl;
}


bool AgregatorConfiguration::backProduction() const
{
	return givenOptions_.count("back-production");
}

std::string AgregatorConfiguration::backProductionSpec() const
{
	const variable_value & opt = givenOptions_["back-production"];
	if ( opt.empty() )
		return std::string();
	return opt.as<std::string>();
}

bool AgregatorConfiguration::daemonMode() const
{
	return givenOptions_.count("daemon-mode");
}

std::string AgregatorConfiguration::proxyDatabaseName() const
{
	const variable_value & opt = givenOptions_["back-production"];
	if ( opt.empty() )
		return kvPath("localstatedir")+ "/agregate/database.sqlite";
	return opt.as<std::string>();
}


bool AgregatorConfiguration::repopulateDatabase() const
{
	return givenOptions_["repopulate"].as<bool>();
}
