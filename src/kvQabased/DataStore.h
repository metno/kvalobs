#ifndef DATASTORE_H_
#define DATASTORE_H_

#include <kvalobs/kvDataOperations.h>
#include <string>
#include <list>
#include <vector>
#include <functional>


/**
 * A storage of the base data for a test run. 
 */
class DataStore
{
public:

  typedef std::vector<DataStore> DataList;
  
  /**
   * Factory method for creating several DataStore objects. Will create and 
   * populate a DataStore for each (station,type,obstime,sensor,level) 
   * combination in the given kvData list.
   */
  static void getStores(DataList & out, const std::list<kvalobs::kvData> & data);

  DataStore();

  ~DataStore();

  /// @todo rename me
  void push_back(const kvalobs::kvData & d)
  {
    data_.push_back(d);
  }
  
  /// @todo remove me
  kvalobs::kvData & operator [] (const std::vector<kvalobs::kvData>::size_type index) const 
  {
    return data_[index];
  }

//  /// @todo remove me
//  kvalobs::kvData & operator [] (const std::vector<kvalobs::kvData>::size_type index) 
//  {
//    return data_[index];
//  }

  /**
   * Get a data object for the given paramid. If the paramid does not exist,
   * get an empty kvData object. In that case, the new data will be inserted 
   * into the container.
   */
//  kvalobs::kvData & operator[](int parameter);
  
  typedef std::vector<kvalobs::kvData>::iterator iterator;
  iterator begin() const { return data_.begin(); }
  iterator end() const { return data_.end(); } 

  typedef iterator const_iterator;
  
//  typedef std::vector<kvalobs::kvData>::const_iterator const_iterator;
//  const_iterator begin() const { return (const_iterator) data_.begin(); }
//  const_iterator end() const { return (const_iterator) data_.end(); }


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
    return stationID_;
  }
  int typeID() const
  {
    return typeID_;
  }
  const miutil::miTime & obstime() const
  {
    return obstime_;
  }
  int sensor() const
  {
    return sensor_;
  }
  int level() const
  {
    return level_;
  }

private:

  DataStore(const kvalobs::kvData & type);

  int stationID_;
  int typeID_;
  miutil::miTime obstime_;
  int sensor_;
  int level_;


  /// @throws std::logic_error if data does not match DataStore Specs
  void insert_(const kvalobs::kvData & d);

  typedef std::vector<kvalobs::kvData> Store;
  mutable Store data_;
};


namespace std
{
  template<>
  struct less<DataStore> : binary_function<DataStore,DataStore,bool>
  {
    bool operator() (const DataStore & a, const DataStore & b) const
    {
      if ( a.stationID() != b.stationID() )
        return a.stationID() < b.stationID();
      return a.obstime() < b.obstime();      
    }
  };
}

// functors:

class DataStoreMatches : public std::unary_function<DataStore, bool>
{
  int stationID_;
  const miutil::miTime & obstime_;
public:
  DataStoreMatches(int stationID, const miutil::miTime & obstime)
  : stationID_(stationID), obstime_(obstime)
  {}
  bool operator () (const DataStore & ds) const
  {
    return ds.stationID() == stationID_ && ds.obstime() == obstime_;
  }
};


#endif /*DATASTORE_H_*/
