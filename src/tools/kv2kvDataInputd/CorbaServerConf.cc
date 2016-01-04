#include <sstream>
#include <boost/algorithm/string.hpp>
#include "CorbaServerConf.h"

using namespace std;

CorbaServerConf::CorbaServerConf(const CorbaServerConf &cns)
    : name(cns.name),
      ns(cns.ns) {
}

CorbaServerConf&
CorbaServerConf::operator=(const CorbaServerConf &rhs) {
  if (this != &rhs) {
    name = rhs.name;
    ns = rhs.ns;
  }

  return *this;
}

void CorbaServerConf::clean() {
  name.erase();
  ns = CorbaHelper::ServiceHost();
}

CorbaServerConf CorbaServerConf::decode(const std::string &confString,
                                        const std::string &defaultNameserver) {
  CorbaServerConf conf;

  if (!conf.decodeConfspec(confString, defaultNameserver))
    throw logic_error("Invalid server conf <" + confString + ">.");

  return conf;
}

bool CorbaServerConf::decodeConfspec(const std::string &confString,
                                     const std::string &defaultNameserver) {
  string nameServer(defaultNameserver);
  ostringstream ost;
  name = confString;

  string::size_type i = name.find("@");

  if (i != string::npos) {
    nameServer = name.substr(i + 1);
    name.erase(i);
  }

  boost::trim(name);
  boost::trim_right_if(name, boost::is_any_of("/"));

  if (!ns.decode(nameServer, 2809) || name.empty()) {
    clean();
    return false;
  }

  return true;
}

std::ostream&
operator<<(std::ostream &out, const CorbaServerConf &sf) {
  out << sf.name << "@" << sf.ns.toString();
  return out;
}

