#include "BackProduction.h"
#include "AgregatorRunner.h"
#include "proxy/KvDataReceiver.h"
#include "proxy/KvalobsProxy.h"
#include <kvcpp/KvApp.h>
#include <milog/milog.h>
#include <boost/lexical_cast.hpp>
#include <stdexcept>

using namespace std;

BackProduction::BackProduction(kvservice::proxy::KvalobsProxy & proxy,
		const AgregatorRunner & runner,
    const miutil::miTime & from, const miutil::miTime & to) :
  proxy_(proxy), runner_(runner), from_(from), to_(to)
{
}

BackProduction::BackProduction(kvservice::proxy::KvalobsProxy & proxy,
		const AgregatorRunner & runner,
    const std::string & timeSpec) :
  proxy_(proxy), runner_(runner)
{
  const string::size_type sep = timeSpec.find_first_of(',');
  if ( sep == string::npos )
    throw std::logic_error("Invalid specification: " + timeSpec);
  
  string from = timeSpec.substr(0, sep);
  
  from_.setTime( from );
  if ( from_.undef() )
    throw std::logic_error("Invalid from specification: " + from);
  
  const string::size_type nextWord = sep +1;
  if ( nextWord == timeSpec.size() )
    throw std::logic_error("Invalid specification: " + timeSpec);
  
  const std::string to = timeSpec.substr(nextWord);
  try {
    unsigned duration = boost::lexical_cast<unsigned>(to);
    to_ = from_;
    to_.addHour(duration);
  }
  catch ( boost::bad_lexical_cast & ) {
    to_.setTime(to);
  }
  
  if ( to_.undef() )
    throw std::logic_error("Invalid to specification: " + to);
}

BackProduction::~BackProduction()
{
}

void BackProduction::operator () ()
{
  miutil::miTime f(to_);

  while ( ! runner_.stopping() && from_ < to_ ) {
    to_ = f;
    f.addHour(-1);

    processData(f, to_);

    LOGINFO( "Done processing data for time " << to_ );
  }
}

void BackProduction::processData(const miutil::miTime &from, const miutil::miTime &to)
{
	milog::LogContext context("BackProduction::processData");
	LOGINFO( "Starting processing of data from " << from << " to " << to );

	kvservice::WhichDataHelper wdh(CKvalObs::CService::All);
	miutil::miTime new_to(to);
	new_to.addSec(-1);

    const std::vector<int> & stations = proxy_.getInteresingStations();

	if (stations.empty())
		wdh.addStation(0, from, new_to);
	else
		for ( std::vector<int>::const_iterator it = stations.begin(); it != stations.end(); ++ it )
			wdh.addStation(*it, from, new_to);

	kvservice::KvDataList dataList;
	kvservice::proxy::internal::KvDataReceiver dr(dataList);
	bool result = kvservice::KvApp::kvApp->getKvData(dr, wdh);
	if (!result)
	{
		const char * err_msg = "Unable to retrieve data from kvalobs.";
		LOGERROR( err_msg );
		return;
	}

	LOGDEBUG( "Got data. Processing..." );
	proxy_.getCallbackCollection().send(dataList);

	LOGDEBUG( "Done" );
}

