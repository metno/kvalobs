#ifndef __kvDataSrcList_h__
#define __kvDataSrcList_h__

#include <string>
#include <kvskel/datasource.hh>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <list>
#include <corbahelper/corbaApp.h>

struct KvDataReceiver {
  std::string confName;
  std::string dirName;
  std::string name;
  CorbaHelper::ServiceHost ns;

  KvDataReceiver() {
  }
  KvDataReceiver(const KvDataReceiver &dr);

  KvDataReceiver& operator=(const KvDataReceiver &rhs);

  void clean();
  /**
   * confString is on the form name@corbanameserver:port
   * 
   * Where name is the 'name' of the kvalobs server to receive
   * data. 'corbanameserver' is the CORBA nameservice to look 
   * up the 'name'. Port is the CORBA nameservice port.
   * 
   * The nameserver part is optional. If it is not given
   * the default nameserver is used. 
   */
  bool decode(const std::string &confString,
              const std::string &defaultNameserver);
};

typedef std::list<KvDataReceiver> TKvDataReceiverList;

class KvDataSrc {
  KvDataReceiver dataReceiver_;
  CKvalObs::CDataSource::Data_var ref_;
  boost::posix_time::ptime dt;

 public:
  KvDataSrc(const KvDataReceiver &dataReceiver__,
            CKvalObs::CDataSource::Data_ptr ref__ =
                CKvalObs::CDataSource::Data::_nil())
      : dataReceiver_(dataReceiver__),
        ref_(ref__) {
  }

  KvDataSrc()
      : ref_(CKvalObs::CDataSource::Data::_nil()) {
  }

  ~KvDataSrc() {
  }

  KvDataSrc& operator=(const KvDataSrc &rhs) {
    if (this != &rhs) {
      dataReceiver_ = rhs.dataReceiver_;
      ref_ = CKvalObs::CDataSource::Data::_duplicate(rhs.ref_);
      dt = rhs.dt;
    }
    return *this;
  }

  bool isNull() const {
    return CORBA::is_nil(ref_);
  }
  void nextTry(const boost::posix_time::ptime &newDt) {
    dt = newDt;
  }
  boost::posix_time::ptime nextTry() const {
    return dt;
  }
  KvDataReceiver dataReceiver() const {
    return dataReceiver_;
  }
  CKvalObs::CDataSource::Data_ptr ref() const {
    return ref_;
  }
  void ref(const CKvalObs::CDataSource::Data_ptr ref__) {
    ref_ = ref__;
  }
};

typedef std::list<KvDataSrc> TKvDataSrcList;
typedef std::list<KvDataSrc>::iterator ITKvDataSrcList;
typedef std::list<KvDataSrc>::const_iterator CITKvDataSrcList;

#endif
