#include "DataStore.h"
#include <algorithm>
#include <map>

using namespace std;
using namespace kvalobs;

DataStore::DataStore(const kvalobs::kvData & dummy) :
  factory_(dummy)
{
}

DataStore::~DataStore()
{
}

void DataStore::getStores(list<DataStore> & out, const vector<kvData> & data)
{
  typedef map<kvalobs::kvData, DataStore, kvalobs::compare::lt_kvData_without_paramID<> > CreateList;
  CreateList toCreate;

  for (vector<kvData>::const_iterator it = data.begin(); it != data.end(); ++it) {
    CreateList::iterator f = toCreate.insert(CreateList::value_type(*it, DataStore(*it))).first;
    f->second.insert_(*it);
  }

  for (CreateList::const_iterator category = toCreate.begin(); category != toCreate.end(); ++category)
    out.push_back(category->second);
}

DataStore::Data::Data(const kvalobs::kvData & d) :
  parameter(d.paramID()), original(d.original()), corrected(d.corrected()),
      controlinfo(d.controlinfo()), useinfo(d.useinfo()), cfailed(d.cfailed())
{
}

kvalobs::kvData DataStore::getData_(const DataStore::Data & d) const
{
  kvData r = factory_.getData(d.original, d.parameter);
  r.corrected(d.corrected);
  r.controlinfo(d.controlinfo);
  r.useinfo(d.useinfo);
  r.cfailed(d.cfailed);

  return r;
}

void DataStore::insert_(const kvalobs::kvData & d)
{
  data_.push_back(d);
}

namespace
{
  struct has_paramid
  {
    int paramid;
    has_paramid(int param) :
      paramid(param)
    {
    }
    template<typename T> bool operator()(const T & d)
    {
      return d.parameter == paramid;
    }
  };
}

const kvData DataStore::operator[](int parameter) const
{
  vector<Data>::const_iterator f = find_if(data_.begin(), data_.end(), has_paramid(parameter));
  if ( f != data_.end() )
  return getData_( * f );
  else
  return factory_.getMissing(parameter);
}
