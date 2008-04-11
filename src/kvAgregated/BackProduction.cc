#include "BackProduction.h"
#include "proxy/KvalobsProxy.h"
#include <milog/milog.h>
#include <boost/lexical_cast.hpp>
#include <stdexcept>

using namespace std;

BackProduction::BackProduction(kvservice::proxy::KvalobsProxy & proxy,
    const miutil::miTime & from, const miutil::miTime & to) :
  proxy_(proxy), from_(from), to_(to)
{
}

BackProduction::BackProduction(kvservice::proxy::KvalobsProxy & proxy,
    const std::string & timeSpec) :
  proxy_(proxy)
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

  while ( ! proxy_.stopping() && from_ < to_ ) {
    to_ = f;
    f.addHour(-1);

    proxy_.processData( f, to_ );

    LOGINFO( "Done processing data for time " << to_ );
  }
}
