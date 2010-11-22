#include "CheckCreator.h"
#include "kvQABaseScriptManager.h"
#include "kvQABaseMetadata.h"
#include "kvQABaseDBConnection.h"
#include "perl/kvPerlParser.h"
#include <kvalobs/kvChecks.h>
#include <kvalobs/kvStationInfo.h>
#include <sstream>
#include <stdexcept>

using namespace std;
using namespace kvalobs;

CheckCreator::CheckCreator(kvQABaseMeteodata & meteod,
		const kvStationInfo & station, kvQABaseDBConnection & con) :
	station_(station), meteod_(meteod), metad(con), sman(con)
{
	if (!con.getObsPgm(station.stationID(), station.obstime(), oprogramlist_))
		throw std::runtime_error("Error when accesssing obs_pgm from database");

	if (!con.getChecks(station_.stationID(), station_.obstime(), checklist_))
		throw std::runtime_error("Error reading checks from database");

	//  meteod_.resetFlags(station);
}

CheckCreator::~CheckCreator()
{
}

void CheckCreator::getScripts(CheckCreator::ScriptList & out,
		const kvChecks & check)
{
	string perlScript = getPerlScript(check);
	string meteoData = getMeteoData(check);
	string metaData = getMetaData(check);

	// WARNING: create strings first, and in this order!
	// there is some dependency on something here that I have never quite understood.. :/
	Script script(perlScript, meteoData, metaData, check.qcx());

	out.push_back(script);
}

string CheckCreator::getPerlScript(const kvalobs::kvChecks & check)
{
	bool sig_ok; // signature ok
	if (!sman.findAlgo(check.checkname(), check.checksignature(),
			station_.stationID(), sig_ok))
		throw runtime_error(
				"CheckCreator::runChecks failed in ScriptManager.findAlgo\nAlgorithm not identified!");
	else if (!sig_ok)
		throw runtime_error(
				"CheckCreator::runChecks failed in ScriptManager.findAlgo\nBAD signature(s)!");

	string ret;

	if (!sman.getScript(ret))
		throw runtime_error(
				"CheckCreator::runChecks failed in ScriptManager.getScript");

	return ret;
}

string CheckCreator::getMeteoData(const kvalobs::kvChecks & check)
{
	return meteod_.data_asPerl(sman, oprogramlist_);
}

string CheckCreator::getMetaData(const kvalobs::kvChecks & check)
{
	string ret;
	if (!metad.data_asPerl(station_.stationID(), check.qcx(),
			station_.obstime(), sman, ret))
		throw std::runtime_error(
				"CheckCreator::runChecks failed in metad.data_asPerl");
	return ret;
}

CheckCreator::Script::Script(const std::string & perlScript, const std::string & meteoData, const std::string & metaData, const std::string & qcx) :
		perlScript_(perlScript), meteoData_(meteoData), metaData_(metaData), qcx_(qcx)
{}

CheckCreator::Script::return_value CheckCreator::Script::run() const
{
	kvPerlParser parser; // the perlinterpreter
	return_value retvalues;
	if ( ! parser.runScript(str(), retvalues) )
		throw std::runtime_error("CheckRunner::runCheck failed in parser.runScript");
	return retvalues;
}


std::string CheckCreator::Script::str() const
{
	ostringstream checkstr; // final check string
	checkstr << "#==========================================\n"
			<< "# KVALOBS check-script\n" << "# type: " << qcx() << "\n"
			<< "#==========================================\n\n"
			<< "use strict;\n";

	checkstr << metaData() << meteoData() << perlScript();

	return checkstr.str();
}