#include "CheckCreator.h"
#ifdef USE_PYTHON
#include "kvPythonInterpreter.h"
#endif
#include "kvQABaseScriptManager.h"
#include "kvQABaseMetadata.h"
#include "kvQABaseDBConnection.h"
#include <kvalobs/kvChecks.h>
#include <kvalobs/kvStationInfo.h>
#include <sstream>
#include <stdexcept>

using namespace std;
using namespace kvalobs;


CheckCreator::CheckCreator(kvQABaseMeteodata & meteod, const kvStationInfo & station, kvQABaseDBConnection & con)
	: station_(station), meteod_(meteod), metad(con), sman(con)
{
  if ( ! con.getObsPgm( station.stationID(), station.obstime(), oprogramlist_ ) )
    throw std::runtime_error( "Error when accesssing obs_pgm from database" );
  
  if ( ! con.getChecks( station_.stationID(), station_.obstime(), checklist_ ) )
    throw std::runtime_error( "Error reading checks from database" );
  
//  meteod_.resetFlags(station);
}

CheckCreator::~CheckCreator()
{
}

void CheckCreator::getScripts(CheckCreator::ScriptList & out, const kvChecks & check)
{
  ostringstream checkstr; // final check string
#ifdef USE_PYTHON
  if (check.language() == PERL)
  {
#endif
  checkstr << "#==========================================\n"
  << "# KVALOBS check-script\n"
  << "# type: " << check.qcx() << "\n"
  << "#==========================================\n\n"
  << "use strict;\n";

  string perlScript = getPerlScript( check );
  string meteoData = getMeteoData( check );
  string metaData = getMetaData( check );

  checkstr << metaData << meteoData << perlScript;
#ifdef USE_PYTHON
  }
  else
  {
	  string pythonScript = getPythonScript(check);
	  checkstr << pythonScript;
  }
#endif
  out.push_back(checkstr.str());
}

string CheckCreator::getPerlScript( const kvalobs::kvChecks & check )
{
  bool sig_ok; // signature ok
  if ( ! sman.findAlgo( check.checkname(), check.checksignature(), station_.stationID(), sig_ok ) )
    throw runtime_error( "CheckCreator::getPerlScript failed in ScriptManager.findAlgo\nAlgorithm not identified!" );
  else if ( ! sig_ok )
    throw runtime_error( "CheckCreator::getPerlScript failed in ScriptManager.findAlgo\nBAD signature(s)!" );
  
  string ret;
  
  if ( ! sman.getScript( ret ) )
    throw runtime_error( "CheckCreator::getPerlScript failed in ScriptManager.getScript" );

  return ret;
}
#ifdef USE_PYTHON
/* the PYTHON script */
string CheckCreator::getPythonScript( const kvalobs::kvChecks & check )
{
  bool sig_ok; // signature ok
  string ret;
  if ( ! sman.findAlgo( check.checkname(), check.checksignature(), station_.stationID(), sig_ok ) )
    throw runtime_error( "CheckCreator::getPythonScript failed in ScriptManager.findAlgo\nAlgorithm not identified!" );
  else if ( ! sig_ok )
    throw runtime_error( "CheckCreator::getPythonScript failed in ScriptManager.findAlgo\nBAD signature(s)!" );
  
  // The PYTHON interpreter
  kvPythonInterpreter * interpret = kvPythonInterpreter::getInterpreter();
  // optimizing for performance reasons...
  if( interpret->is_registered(check.checkname()))
	  return ret;
  if ( ! sman.getScript( ret ) )
    throw runtime_error( "CheckCreator::getPythonScript failed in ScriptManager.getScript" );
  
  interpret->register_script(check.checkname(),ret);
  return ret;
}
#endif

string CheckCreator::getMeteoData( const kvalobs::kvChecks & check )
{
  return meteod_.data_asPerl( sman, oprogramlist_ );
}

#ifdef USE_PYTHON
/* The python verion, what to do with data */
bool CheckCreator::getMeteoData( const kvalobs::kvChecks & check,
							   std::list<kvQABase::script_var>& data)
{
  return meteod_.data_asRaw( sman, oprogramlist_, data );
}
#endif

string CheckCreator::getMetaData( const kvalobs::kvChecks & check )
{
  string ret;
  if ( ! metad.data_asPerl( station_.stationID(), check.qcx(), station_.obstime(), sman, ret ) )
    throw std::runtime_error( "CheckCreator::runChecks failed in metad.data_asPerl" );
  return ret;
}

#ifdef USE_PYTHON
/* The python version, what to do with data ? */

bool CheckCreator::getMetaData( const kvalobs::kvChecks & check,
							  std::list<kvQABase::script_var>& data)
{
  if ( ! metad.data_asRaw( station_.stationID(), check.qcx(), station_.obstime(), sman, data ) )
    throw std::runtime_error( "CheckCreator::getMetaData failed in metad.data_asRaw" );
  return true;
}
#endif
