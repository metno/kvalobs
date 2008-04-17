#include "DataStore.h"
#include <algorithm>
#include <map>

using namespace std;
using namespace kvalobs;

DataStore::DataStore()
{
}

DataStore::DataStore(const kvalobs::kvData & dummy) 
	: stationID_(dummy.stationID())
	, typeID_(dummy.typeID())
	, obstime_(dummy.obstime())
	, sensor_(dummy.sensor())
	, level_(dummy.level())
{
}

DataStore::~DataStore()
{
}


// fixme!
void DataStore::getStores(DataStore::DataList & out, const list<kvData> & data)
{
  typedef map<kvalobs::kvData, DataStore, kvalobs::compare::lt_kvData_without_paramID<> > CreateList;
  CreateList toCreate;

  for (list<kvData>::const_iterator it = data.begin(); it != data.end(); ++it) {
    CreateList::iterator f = toCreate.insert(CreateList::value_type(*it, DataStore(*it))).first;
    f->second.insert_(*it);
  }

  for (CreateList::const_iterator category = toCreate.begin(); category != toCreate.end(); ++category)
    out.push_back(category->second);
}

//DataStore::Data::Data(const kvalobs::kvData & d) :
//  parameter(d.paramID()), original(d.original()), corrected(d.corrected()),
//      controlinfo(d.controlinfo()), useinfo(d.useinfo()), cfailed(d.cfailed())
//{
//}

void DataStore::insert_(const kvalobs::kvData & d)
{
  if ( d.stationID() != stationID() || 
      d.obstime() != obstime() || 
      d.typeID() != typeID() ||
      ! kvalobs::compare::eq_sensor(d.sensor(), sensor()) ||
      d.level() != level() )
    throw std::logic_error("Inserted data does not match container");
  
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

//kvData & DataStore::operator[](int parameter)
//{
//  Store::const_iterator f = find_if(data_.begin(), data_.end(), has_paramid(parameter));
//  if ( f == data_.end() ) {
//    kvalobs::kvData r = factory_.getMissing(parameter);
//    data_.push_back(r);
//    f = data.end();
//    -- f;
//  }
//  return * f;
//}
