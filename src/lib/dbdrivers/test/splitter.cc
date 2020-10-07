#include <iostream>
#include <list>
#include <vector>
#include <string>
#include "../../kvdb/utils.h"
using namespace std;
using namespace utils;

bool
useMaster(string& conf)
{
  const string kvdbMaster("kvdb_master");
  const string kvdbSlave("kvdb_slave");
  bool master = true;

  string::size_type i = conf.find(kvdbSlave);
  if (i != string::npos) {
    master = false;
    conf.replace(i, kvdbSlave.length(), "");
  }

  i = conf.find(kvdbMaster);
  if (i != string::npos) {
    master = true;
    conf.replace(i, kvdbMaster.length(), "");
  }
  return master;
}


bool 
getHostValue(string& conf, string *value)
{
  list<string> kvs;
  value->erase();

  if( ! findKeyVals(conf, &kvs, "=")) {
    return false;
  }

  for( auto keyval : kvs ) {
    vector<string> kv=splitstr(keyval, '=', false);

    if (kv.size()<2) {
        continue;
    }

    if( trim(kv[0])=="host") {
      *value=trim(kv[1]);
      string::size_type i=conf.find(keyval);
      if( i!=string::npos ) {
        conf.replace(i, keyval.length(), "@@HOST@@");
        return true;
      } else {
        return false;
      }
    }
  }

  return true;
}

bool
getHosts(string& conf, list<string> *hosts)
{
  string hostVal;
  hosts->clear();
  if( !getHostValue(conf, &hostVal) ) {
    return false;
  }

  for( auto h: splitstr(hostVal, ',', true) ){
    string host=trim(h);
    if( ! host.empty() ) {
      hosts->push_back(host);
    }
  }
  return true;
}

bool
getConfs(const string& conf_, bool* master, list<string> *confs)
{
  string conf(conf_);
  list<string> ll;
  list<string> hosts;  

  *master = useMaster(conf);
  confs->clear();
  
  if ( !getHosts(conf, &hosts) ){
    return false;
  }

  for( auto host : hosts ) {
    string cnf(conf);
    string::size_type i=cnf.find("@@HOST@@");
    if ( i==string::npos) {
      return false;
    }
    host = "host="+host;
    cnf.replace(i, 8, host);
    confs->push_back(cnf);
  }
  

  return true;
}

int
main(int argn, char* argv[])
{
  string conf=" user=kvproc dbname=kvalobs host  = ct58002.int.met.no,host2 port=5432"; 
  bool master; 
  cerr << "'" << conf <<"'\n\n";
  list<string> confs;
  string value;
  
  if( !getConfs(conf, &master, &confs) ) {
    cerr << "Huff, error \n";
  }

  for( auto &c : confs ){
    cerr << "'" << c << "' master: " << (master?"true":"false") << "\n";
  }
}
