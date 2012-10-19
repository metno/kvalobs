#ifndef CHECKCREATOR_H_
#define CHECKCREATOR_H_

#include "kvQABaseMeteodata.h"
#include "kvQABaseScriptManager.h"
#include "kvQABaseMetadata.h"
#include <kvalobs/kvChecks.h>
#include <map>
#include <string>
#include <list>
#include <vector>

namespace kvalobs
{
class kvStationInfo;
}

class kvQABaseScriptManager;
class kvQABaseMetadata;
class kvQABaseDBConnection;

/**
 * Creates a check script.
 */
class CheckCreator
{
public:
	CheckCreator(kvQABaseMeteodata & meteod,
			const kvalobs::kvStationInfo & station, kvQABaseDBConnection & con);
	~CheckCreator();

	class Script;

	typedef std::vector<Script> ScriptList;

	void getScripts(ScriptList & out, const kvalobs::kvChecks & check);

	const std::list<kvalobs::kvChecks> & getChecks() const
	{
		return checklist_;
	}
	const kvObsPgmList & oprogramlist() const
	{
		return oprogramlist_;
	}

private:
	std::string getPerlScript(const kvalobs::kvChecks & check);
	std::string getMeteoData(const kvalobs::kvChecks & check);
	std::string getMetaData(const kvalobs::kvChecks & check);

	const kvalobs::kvStationInfo & station_;
	kvObsPgmList oprogramlist_;
	std::list<kvalobs::kvChecks> checklist_;
	kvQABaseMeteodata & meteod_; // Meteorological data manager
	kvQABaseMetadata metad;
	kvQABaseScriptManager sman;
};

class CheckCreator::Script
{
public:
	Script(const std::string & perlScript, const std::string & meteoData, const std::string & metaData, const std::string & qcx);

	typedef	std::map<std::string, double> return_value;

	/**
	 * Run the script
	 */
	return_value run() const;

	std::string str() const;

	const std::string & perlScript() const { return perlScript_; }
	const std::string & meteoData() const { return meteoData_; }
	const std::string & metaData() const { return metaData_; }
	const std::string & qcx() const { return qcx_; }

private:
	std::string perlScript_;
	std::string meteoData_;
	std::string metaData_;
	std::string qcx_;
};



#endif /*CHECKCREATOR_H_*/
