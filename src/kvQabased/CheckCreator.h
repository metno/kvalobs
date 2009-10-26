#ifndef CHECKCREATOR_H_
#define CHECKCREATOR_H_

#include "kvQABaseMeteodata.h"
#include "kvQABaseScriptManager.h"
#include "kvQABaseMetadata.h"
#include <kvalobs/kvChecks.h>
#include <string>
#include <list>
#include <vector>
#ifdef USE_PYTHON
#include "kvQABaseTypes.h"
#endif

namespace kvalobs {
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
  CheckCreator( kvQABaseMeteodata & meteod, const kvalobs::kvStationInfo & station, kvQABaseDBConnection & con );
  ~CheckCreator();
  
  typedef std::vector<std::string> ScriptList;
  
  void getScripts(ScriptList & out, const kvalobs::kvChecks & check );
  
  const std::list<kvalobs::kvChecks> & getChecks() const { return checklist_; }
  const kvObsPgmList & oprogramlist() const { return oprogramlist_; }
  
#ifdef USE_PYTHON
  /* extension for python */
  std::string getPythonScript( const kvalobs::kvChecks & check );
  
  /* these two new methods take a general approach when returning data,
   instead of a string. The return a boolean or throws an exception depending of the
   severity of the error
  */
  bool getMeteoData( const kvalobs::kvChecks & check, std::list<kvQABase::script_var>& data );
  bool getMetaData( const kvalobs::kvChecks & check, std::list<kvQABase::script_var>& data );
#endif
  
private:
  std::string getPerlScript( const kvalobs::kvChecks & check );
  std::string getMeteoData( const kvalobs::kvChecks & check );
  std::string getMetaData( const kvalobs::kvChecks & check );
  
  const kvalobs::kvStationInfo & station_;
  kvObsPgmList oprogramlist_;
  std::list<kvalobs::kvChecks> checklist_;
  kvQABaseMeteodata & meteod_; // Meteorological data manager
  kvQABaseMetadata metad;
  kvQABaseScriptManager sman;
};

#endif /*CHECKCREATOR_H_*/
