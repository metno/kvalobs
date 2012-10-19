#include <string>
#include <sstream>
#include <miutil/replace.h>
#include <miutil/trimstr.h>
#include "kvDataSrcList.h"

using namespace std;
using namespace miutil;

KvDataReceiver::
KvDataReceiver( const KvDataReceiver &dr ) 
	: confName( dr.confName ), dirName( dr.dirName ),
	  name( dr.name), ns( dr.ns ) 
{
}
	
KvDataReceiver& 
KvDataReceiver::
operator=( const KvDataReceiver &rhs ) 
{
	if( this != &rhs ) {
		confName = rhs.confName;
		dirName = rhs.dirName;
		name = rhs.name;
		ns = rhs.ns;
	}
	
	return *this;
}


void
KvDataReceiver::
clean()
{
	confName.erase();
	dirName.erase();
	name.erase();
   ns = CorbaHelper::ServiceHost();
}

bool 
KvDataReceiver::
decode( const std::string &confString, const std::string &defaultNameserver )
{
	string nameServer( defaultNameserver);
	string tmp;
	ostringstream ost;
	confName = confString;
	name = confString;
	
	string::size_type i = name.find("@");
	
	if( i != string::npos ) {
		nameServer = name.substr( i+1 );
		name.erase( i );
	}

	trimstr( name );
	
	if( ! ns.decode( nameServer, 2809 ) || name.empty() ) {
		clean();
		return false;
	}

	tmp = ns.host;
	
	miutil::replace( tmp, ".", "_");
	
	ost << name << "-" << tmp << "-" << ns.port;
	dirName = ost.str();
	
	return true;
}
