#ifndef DATASTORE_H_
#define DATASTORE_H_

#include <kvalobs/kvDataOperations.h>
#include <boost/concept_check.hpp>
#include <string>
#include <list>
#include <vector>

/**
 * A storage of the base data for a test run. 
 */
class DataStore
{
public:

  /**
   * Factory method for creating several DataStore objects. Will create and 
   * populate a DataStore for each (station,type,obstime,sensor,level) 
   * combination in the given kvData list.
   */
  static void getStores(std::list<DataStore> & out,
      const std::vector<kvalobs::kvData> & data);

  ~DataStore();

  template<typename kvDataOutputIterator> void
      getData(kvDataOutputIterator out) const;

  /**
   * Get a data object for the given paramid. If the paramid does not exist,
   * get an empty kvData object.
   * 
   * The return value is const in order not to give the impression that 
   * changes to the returned object will modify the underlying data.
   */
  const kvalobs::kvData operator[](int parameter) const;

  bool empty() const
  {
    return data_.empty();
  }

  size_t size() const
  {
    return data_.size();
  }

  // Storage characterisitcs:

  int stationID() const
  {
    return factory_.stationID();
  }
  int typeID() const
  {
    return factory_.typeID();
  }
  const miutil::miTime & obstime() const
  {
    return factory_.obstime();
  }
  int sensor() const
  {
    return factory_.sensor();
  }
  int level() const
  {
    return factory_.level();
  }

private:
  kvalobs::kvDataFactory factory_;

  DataStore(const kvalobs::kvData & type);

  struct Data
  {
    /// @throws std::logic_error if d does not match the factory's specs
    Data(const kvalobs::kvData & d);

    int parameter;
    float original;
    float corrected;
    kvalobs::kvControlInfo controlinfo;
    kvalobs::kvUseInfo useinfo;
    std::string cfailed;
  };

  kvalobs::kvData getData_(const Data & d) const;
  void insert_(const kvalobs::kvData & d);

  typedef std::vector<Data> Store;
  Store data_;
};

template<typename kvDataOutputIterator> void DataStore::getData(
    kvDataOutputIterator out) const
{
  using namespace boost;
  function_requires<OutputIteratorConcept<kvDataOutputIterator, kvalobs::kvData> >();

  for (Store::const_iterator it = data_.begin(); it != data_.end(); ++it)
    *out ++ = getData_( *it);
}

#endif /*DATASTORE_H_*/
