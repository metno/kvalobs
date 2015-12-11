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

#include <config.h>
#include "Configuration.h"
#include <kvalobs/kvStationInfo.h>
#include <kvalobs/kvPath.h>
#include <kvsubscribe/DataSubscriber.h>
#include <miutil/timeconvert.h>
#include <boost/program_options.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <iostream>
#include <cstdlib>


using namespace boost::program_options;

namespace {
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
}

namespace qabase
{
namespace
{
boost::filesystem::path defaultConfigFile()
{
	using namespace boost::filesystem;

#ifdef SYSCONFDIR
	const path homeDir(SYSCONFDIR);
	const path potentialConfigFile(homeDir / "kvQabased.conf" );
	if ( exists(potentialConfigFile) )
		return potentialConfigFile;
#endif
	return path();
}

kvalobs::kvStationInfo * getStationInfo(const variables_map & vm)
{
	int optionCount = 0;
	optionCount += vm.count("station");
	optionCount += vm.count("obstime");
	optionCount += vm.count("typeid");

	if ( optionCount == 0 )
		return 0;
	if ( optionCount < 3 )
		throw std::runtime_error("Missing station spec");

	int station = vm["station"].as<int>();
	std::string obstime = vm["obstime"].as<std::string>();
	int type = vm["typeid"].as<int>();

	return new kvalobs::kvStationInfo(station, boost::posix_time::time_from_string_nothrow(obstime), type);
}

void parse(const boost::filesystem::path & configFile, variables_map & vm, const options_description & configFileOptions)
{
	using namespace boost::filesystem;

	if ( not exists(configFile) )
		throw std::runtime_error(to_native_file(configFile) + ": no such file or directory");
	if ( is_directory(configFile) )
		throw std::runtime_error(to_native_dir(configFile) + " is a directory");

	boost::filesystem::ifstream s(configFile);
	store(parse_config_file(s, configFileOptions), vm);
	notify(vm);
}
}

Configuration::Configuration(int & argc, char ** argv) :
		runNormally_(true), observationToCheck_(0), logLevel_(milog::DEBUG), port_(0)
{
	const char * USER = std::getenv("USER");
	const std::string databaseUser = USER ? USER : "kvalobs";

	options_description commandLine("Command-line options");

	options_description observation("Single observation");
	observation.add_options()
			("station,s", value<int>(), "Check the given station")
			("obstime,o", value<std::string>(), "Check the given obstime")
			("typeid,t", value<int>(), "Check the given typeid")
			("qcx", value<std::string>(), "Only run the given check. This also means that control flags will not be reset before checks are run")
			("model-source", value<std::string>(& modelDataName_)->default_value("yr"), "Get model data from the given source (from model table in database");

	options_description logging("Logging");
	logging.add_options()
			("runloglevel", value<std::string>()->default_value("info"), "Set loglevel (debug_all, debug, info, warn, error or fatal")
			("runlogfile", value<std::string>(& runLogFile_)->default_value(kvalobs::kvPath(kvalobs::logdir) + "/kvQabased.log"), "Set file name for run log")
			("logdir", value<std::string>(& baseLogDir_)->default_value(kvalobs::kvPath(kvalobs::logdir) + "/checks/"), "Use the given directory as base directory for script logs");

	options_description database("Database");
	database.add_options()
			("database,d", value<std::string>(&databaseName_)->default_value("kvalobs"), "Name of database to connect to")
			("host,h", value<std::string>(&host_), "Hostname of database")
			("port,p", value<int>(&port_), "Port of database")
			("user,U", value<std::string>(&user_)->default_value(databaseUser), "Database user");

	options_description kafka("Kafka Connection");
	kafka.add_options()
			("brokers", value<std::string>(&kafkaBrokers_)->default_value("localhost"), "List of kafka brokers to connect to, comma-separated")
			("domain", value<std::string>(&kafkaDomain_)->default_value("test"), "Kvalobs domain to use in kafka queues, to separate your instance from others that use the same kafka servers");


	options_description generic("Generic");
	generic.add_options()
			("config", value<std::string>(), "Read configuration from the given file")
			("version",	"Produce version information")
			("help", "Produce help message");

	commandLine.add(observation).add(logging).add(database).add(kafka).add(generic);

	parsed_options parsed =
	    command_line_parser(argc, argv).
	    options(commandLine).
	    //allow_unregistered().
	    run();
	variables_map vm;
	store(parsed, vm);
	notify(vm);

	if (vm.count("version"))
	{
		version(std::cout);
		runNormally_ = false;
		return;
	}
	if (vm.count("help"))
	{
		help(std::cout, commandLine);
		runNormally_ = false;
		return;
	}

	options_description configFileOptions;
	configFileOptions.add(logging).add(database).add(kafka);

	if (vm.count("config"))
		parse(vm["config"].as<std::string> (), vm, configFileOptions);

	boost::filesystem::path defaultConfig = defaultConfigFile();
	if (not defaultConfig.empty())
		parse(defaultConfig, vm, configFileOptions);

	if ( vm.count("qcx") )
	{
		std::string qcxFilter = vm["qcx"].as<std::string>();
		qcxFilter_.push_back(qcxFilter);
	}

	if ( vm.count("runloglevel") )
	{
		std::string wantedLevel = vm["runloglevel"].as<std::string>();
		if ( wantedLevel == "debug_all" )
			logLevel_ = milog::DEBUG1;
		else if ( wantedLevel == "debug" )
			logLevel_ = milog::DEBUG;
		else if (wantedLevel == "info")
			logLevel_ = milog::INFO;
		else if (wantedLevel == "warn")
			logLevel_ = milog::WARN;
		else if (wantedLevel == "error")
			logLevel_ = milog::ERROR;
		else if (wantedLevel == "fatal")
			logLevel_ = milog::FATAL;
		else
			throw std::runtime_error(wantedLevel + " is not a valid loglevel");
	}
	//milog::Logger::logger().logLevel(logLevel_);

	observationToCheck_ = getStationInfo(vm);
}

Configuration::~Configuration()
{
	delete observationToCheck_;
}

std::string Configuration::databaseConnectString() const
{
	std::ostringstream dbConnect;
	dbConnect << "dbname=" << databaseName_;
	if ( not host_.empty() )
		dbConnect << " host=" << host_;
	if ( port_ )
		dbConnect << " port=" << port_;
	dbConnect << " user=" << user_;
	return dbConnect.str();
}


namespace
{
void processDataErrors(const std::string & data,
        const std::string & errorMessage)
{
    LOGERROR("Unable to send data: " + errorMessage + "  Data: <" + data + ">");
}
}

std::shared_ptr<kvalobs::subscribe::KafkaProducer> Configuration::kafkaProducer() const
{
	LOGINFO("Creating kafka connection on " << kafkaBrokers_ << ", using topic " << kvalobs::subscribe::DataSubscriber::topic(kafkaDomain_));

	return std::make_shared<kvalobs::subscribe::KafkaProducer>(
	//kvalobs::subscribe::KafkaProducer dataSender(
			kvalobs::subscribe::DataSubscriber::topic(kafkaDomain_),
			kafkaBrokers_,
			processDataErrors);
}

std::ostream & Configuration::version(std::ostream & s) const
{
	return s << "qabase (Kvalobs) " << PACKAGE_VERSION << std::endl;
}

std::ostream & Configuration::help(std::ostream & s, const boost::program_options::options_description & options) const
{
	version(s) << "\n"
			"Kvalobs check runner. This program may either run in daemon mode continuously\n"
			"checking all incoming observations to kvalobs. As an alternative, you may\n"
			"specify a single station, obstime and typeid to check. Daemon mode will not be\n"
			"entered if you do this.\n"
			"\n";

#ifdef QABASE_NO_SAVE
	s << "This is a testing version which will not write any check results back to the\n"
			"database.\n\n";
#endif

	boost::filesystem::path defaultConfig = defaultConfigFile();
	if (not defaultConfig.empty())
		s << "Additional configuration is read from <"
				<< to_native_file(defaultConfig) << ">\n\n";

	s << options << std::endl;

	return s;
}

}
