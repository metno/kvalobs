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

#include "KvalobsCheckScript.h"
#include "DataStore.h"
#include "populateScript.h"
#include "ScriptResultIdentifier.h"
#include <scriptrunner/Script.h>
#include <milog/milog.h>

namespace qabase {
namespace {
scriptrunner::language::Interpreter::Ptr getInterpreter(int kvalobsCode) {
  if (kvalobsCode == 1)
    return scriptrunner::language::Interpreter::get("perl");
  return scriptrunner::language::Interpreter::Ptr();
}


void logObsData(const qabase::DataStore &datastore, std::ostream &scriptLog) {
  const qabase::Observation & observation = datastore.observation();
  auto obsdata=datastore.getObsData();

  scriptLog << "DEBUG begin, obsdata types\n";
  scriptLog << "observation_stationid: " << observation.stationID() << "\n"
    << "observation_typeid: " << observation.typeID() << "\n"
    << "types defined:\n";

  for( auto param : obsdata) {
    if(!param.second.empty()) {
      scriptLog << "  " << param.first.baseName() << ": haveType=" << (param.second.begin()->typeID() != observation.typeID())
        << " typeid=" << param.second.begin()->typeID() << "\n"; 
    }
  }
  scriptLog << "DEBUG end\n";
}

}

KvalobsCheckScript::KvalobsCheckScript(const db::DatabaseAccess & database,
                                       const qabase::Observation & obs,
                                       const kvalobs::kvChecks & check,
                                       std::ostream * scriptLog)
    : scriptLog_(scriptLog) {
  try {
    kvalobs::kvAlgorithms algorithm = database.getAlgorithm(check.checkname());

    if (scriptLog_) {
      (*scriptLog_) << "CHECK:\t" << check.qcx() << " - " << check.checkname()
                    << "\n\n" << "Abstract signature: " << algorithm.signature()
                    << "\n" << "Concrete signature: " << check.checksignature()
                    << "\n\n\n" 
                    << "Observation: stationid=" << obs.stationID() 
                    << ", typeid=" << obs.typeID() << ", obstime=" << obs.obstime() << "\n\n";
     }

    CheckSignature abstractSignature(algorithm.signature(), obs.stationID());
    CheckSignature concreteSignature(check.checksignature(), obs.stationID());

    store_.reset(
        new DataStore(database, obs, check.qcx(), abstractSignature,
                      concreteSignature, scriptLog_));
    script_.reset(
        new scriptrunner::Script(algorithm.script(),
                                 getInterpreter(algorithm.language())));

    //DEBUG begin
    //logObsData(*store_,*scriptLog_);
    //DEBUG end

    addDataToScript(*script_, *store_);
  } catch (NoErrorLogException & e) {
    // Missing model data is not logged as an error
    if (scriptLog_)
      (*scriptLog_) << "NOTE: Unable run check: " << e.what() << "\n\n";
    terminateLogEntry_();
    throw;
  } catch (std::exception & e) {
    if (scriptLog_)
      (*scriptLog_) << "ERROR: Unable to generate check: " << e.what()
                    << "\n\n";
    terminateLogEntry_();
    throw;
  }
}

KvalobsCheckScript::~KvalobsCheckScript() {
  terminateLogEntry_();
}

std::string KvalobsCheckScript::str() const {
  return script_->str();
}

void KvalobsCheckScript::run(db::DatabaseAccess::DataList * modificationsOut) {
  if (scriptLog_)
    (*scriptLog_) << "SCRIPT:\n\n" << str() << '\n';

  scriptrunner::RunResult result = script_->run();

  if (!result) {
    if (scriptLog_)
      (*scriptLog_) << "ERROR when running script\n\n";
    throw std::runtime_error("Error when running script");
  }

  const scriptrunner::RunResult::RunReturn & ret = result.returnValues();

  if (scriptLog_) {
    (*scriptLog_) << "\nRESULT:\n\n";
    for (scriptrunner::RunResult::RunReturn::const_iterator it = ret.begin();
        it != ret.end(); ++it)
      (*scriptLog_) << it->first << " = " << it->second << '\n';
  }

  for (scriptrunner::RunResult::RunReturn::const_iterator it = ret.begin();
      it != ret.end(); ++it) {
    ScriptResultIdentifier result(it->first);
    store_->apply(result, it->second);
  }

  store_->getModified(std::back_inserter(*modificationsOut));

  if (scriptLog_ and not modificationsOut->empty()) {
    (*scriptLog_) << "Modified data:";
    for (db::DatabaseAccess::DataList::const_iterator it = modificationsOut
        ->begin(); it != modificationsOut->end(); ++it)
      (*scriptLog_) << '\n' << *it;
    (*scriptLog_) << std::endl;
  }
}

void KvalobsCheckScript::terminateLogEntry_() const {
  if (scriptLog_) {
    for (int i = 0; i < 79; ++i)
      (*scriptLog_) << '-';
    (*scriptLog_) << '\n' << std::endl;
  }
}

}
