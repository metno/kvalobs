
#ifndef __Qc2App_h__
#define __Qc2App_h__

#include <kvalobs/kvapp.h>
//#include <kvskel/managerInput.hh>
#include <kvskel/kvService.hh>
#include <kvalobs/kvStationInfo.h>
#include <dnmithread/CommandQue.h>
#include <kvdb/dbdrivermgr.h>

/// Application type.
/// The Qc2 application class: follows the standard Qc1 kvalobs model.

class Qc2App: public KvApp
{
  Qc2App(); //No implementation

  dnmi::thread::CommandQue             inQue;
  dnmi::db::DriverManager              dbMgr;
  std::string                          dbConnect;
  std::string                          dbDriver;
  std::string                          dbDriverId;
  bool shutdown_;
  bool                                 orbIsDown;

  CKvalObs::CManager::CheckedInput_var refServiceCheckedInput;
  CKvalObs::CService::DataReadyInput_var refKvServiceDataReady;

  boost::mutex mutex;

 public:
  Qc2App(int argn, char **argv, 
	    const std::string &driver_,
	    const std::string &connect_,
	    const char *options[][2]=0);
  virtual ~Qc2App();

  //inherited from KvApp
  virtual bool isOk()const;

  //For sending data to kvServiced
  bool sendDataToKvService(const kvalobs::kvStationInfoList &info_, bool &busy);

  CKvalObs::CService::DataReadyInput_ptr lookUpKvService(bool forceNS,
                                                     bool &usedNS);

  /*
   * force a shutdown
   */
  void doShutdown(){ shutdown_=true;}
  /**
   * shutdown returns true when the application is in 
   * the terminating state.
   */
  bool shutdown();

  /**
   * Creates a new connection to the database. The caller must
   * call releaseDbConnection after use.
   */
  dnmi::db::Connection *getNewDbConnection();
  void                 releaseDbConnection(dnmi::db::Connection *con);

  
};




#endif
