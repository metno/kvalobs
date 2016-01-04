#include <kvalobs/kvPsSubscribers.h>
#include "PsSubscriberDbHelper.h"

template<typename cmdType>
void SubscriberThread<cmdType>::setLogger(const std::string &name) {
  using namespace milog;

  try {
    FLogStream *logs = new FLogStream(1, 204800);  //200k
    std::ostringstream ost;

    ost << kvpath() << "/var/log/kvService/" << name << ".log";

    if (logs->open(ost.str())) {
      Logger::pushLogStream(logs);
    } else {
      LOGERROR("Cant open the logfile <" << ost.str() << ">!");
      delete logs;
    }
  } catch (...) {
    LOGERROR("Cant create a logstream for name: " << name);
  }
}

template<typename cmdType>
dnmi::db::Connection*
SubscriberThread<cmdType>::getDbConnection() {
  dnmi::db::Connection* dbCon = 0;

  do {
    dbCon = getNewDbConnection();

    if (dbCon) {
      break;
    }

    LOGINFO("Can't create a connection to the database, retry in 5 seconds ..");
    sleep(5);
  } while (!terminate_ && !shutdown());

  return dbCon;
}

template<typename cmdType>
int SubscriberThread<cmdType>::run() {
  milog::LogContext cntxt(subscriberid());
  dnmi::thread::CommandBase *cmd = 0;
  cmdType *theCmd;
  retry = 0;
  maxRetry = 0;

  if (persistent())
    return runPersistent();

  LOGINFO("Thread for subscriber: " << subscriberid() << " started!");

  time (&tick);

  while (!terminate_ && !shutdown() && maxRetry < tick) {
    if (!cmd) {
      //LOGDEBUG("SubscriberThread::run: before getDataFromQue");  
      cmd = getDataFromQue(2);
      //LOGDEBUG("SubscriberThread::run: after getDataFromQue cmd=" << 
      //         (cmd?"true":"false"));  

      if (!cmd)
        continue;

    } else {
      //LOGDEBUG("SubscriberThread::run: leftover");  
      //We have a left over dataset that is not sendt to the subscriber
      sleep(2);

      time(&tick);

      if (tick < retry)
        continue;
    }

    theCmd = dynamic_cast<cmdType*>(cmd);

    if (!theCmd) {
      LOGERROR("Unexpected command!");
      delete cmd;
      cmd = 0;
      continue;
    }

    typename cmdType::_ptr_type sub = subscriber_;

    //LOGDEBUG("SubscriberThread::run: CALLING theCmd");
    int ret = (*theCmd)(*static_cast<KvDataSubscriberInfo*>(this), sub);

    time(&tick);

    if (ret == 0) {
      maxRetry = 0;
      delete theCmd;
      cmd = 0;
    } else if (ret < 0) {
      maxRetry = tick;
    } else {
      if (maxRetry == 0) {
        maxRetry = tick + MAX_RETRY;
      }

      retry = tick + RETRY;
    }
  }

  inque_.suspend();
  inque_.clear();

  if (terminate_) {
    LOGINFO("Terminate: Unsubscribed!");
    return 1;
  }

  if (maxRetry > 0) {
    LOGWARN("TERMINATE: The subscriber is NOT responding!");
    return 2;
  }

  LOGINFO("Terminate: Normal exit!");

  return 0;
}

template<typename cmdType>
int SubscriberThread<cmdType>::runPersistent() {
  milog::LogContext cntxt("peristent/" + subscriberid());
  dnmi::thread::CommandBase *cmd = 0;
  cmdType *theCmd;
  bool moredata = true;
  bool dataSendt;

  std::string name = kvalobs::kvPsSubscribers::nameFromSubscriberid(
      subscriberid());
  ;

  setLogger(name);

  time (&retry);

  LOGINFO(
      "Persistent thread for subscriber: " << subscriberid() << " started!");

  while (!terminate_ && !shutdown()) {
    if (moredata && subscribed() && connected() && dataQueEmpty()) {
      tryToSendSavedData(moredata);
      continue;
    } else {
      tryToSendSavedData(moredata);
    }

    cmd = getDataFromQue(2);

    if (!cmd)
      continue;

    theCmd = dynamic_cast<cmdType*>(cmd);

    if (!theCmd) {
      LOGERROR("Unexpected command!");
      delete cmd;
      cmd = 0;
      continue;
    }

    if (!subscribed() || !connected()) {
      saveDataInfo(theCmd, moredata);
      delete cmd;
      cmd = 0;
      continue;
    }

    if (!sendPsData(*theCmd, moredata, 0)) {
      saveDataInfo(theCmd, moredata);
    }

    delete cmd;
    cmd = 0;
  }

  inque_.suspend();
  inque_.clear();

  if (terminate_) {
    LOGINFO("Terminate: Unsubscribed!");
    return 1;
  }

  LOGINFO("Terminate: Normal exit!");

  return 0;

}

template<typename cmdType>
void SubscriberThread<cmdType>::tryToSendSavedData(bool &moredata) {
  using namespace std;
  using namespace kvalobs;

  if (!subscribed())
    return;

  if (!moredata)
    return;

  time (&tick);

  if (!connected() && tick < retry)
    return;

  int maxrows = 1;

  if (connected())
    maxrows = 10;

  string id = subscriberid();
  string name = kvPsSubscribers::nameFromSubscriberid(id);

  if (name.empty())
    return;

  dnmi::db::Connection *con = getDbConnection();

  if (!con)
    return;

  PsSubscriberDbHelper dbHelper(*con, name);

  list < DataToSubscribersPtr > data = dbHelper.getStationInfo(maxrows);

  if (data.empty()) {
    LOGDEBUG("tryToSendSavedData data.empty");
    moredata = false;
    releaseDbConnection(con);
    return;
  }

  moredata = true;

  std::list<DataToSubscribersPtr>::iterator it = data.begin();
  cmdType *cmd;

  try {
    cmd = new cmdType(*it);
  } catch (...) {
    LOGDEBUG("tryToSendSavedData: Exception: new cmdType");
    releaseDbConnection(con);
    return;
  }

  while (it != data.end() && !terminate_ && !shutdown()) {
    cmd->data(*it);

    if (sendPsData(*cmd, moredata, con)) {
      dbHelper.removeStationInfo(cmd->data());
    } else {
      break;
    }

    it++;
  }

  delete cmd;
  releaseDbConnection(con);
}

template<typename cmdType>
bool SubscriberThread<cmdType>::sendPsData(cmdType &cmd, bool &moredata,
                                           dnmi::db::Connection *con) {
  int iRetry = 0;
  int ret;

  if (!subscribed())
    return false;

  while (!terminate_ && !shutdown() && iRetry < 10) {
    LOGDEBUG("sendPsData: Calling subscriber!");
    ret = (cmd)(*static_cast<KvDataSubscriberInfo*>(this), subscriber_);

    if (ret == 0) {
      maxRetry = 0;
      time (&retry);
      subscribed(true);
      connection(true);
      return true;
    } else if (ret < 0) {
      subscribed(false);
      connection(false);
      return false;
    } else if (ret == 2) {
      //The subscriber is BUSSY, and cant accept the
      //data at the momment.
      sleep(2);
      iRetry++;
      maxRetry = 0;
    } else {  //ret>0
      time (&tick);
      retry = tick + RETRY;
      connection(false);

      if (maxRetry == 0) {
        maxRetry = tick + 300;  //5 min
      } else if (maxRetry < tick) {
        dnmi::db::Connection *myCon;

        if (con)
          myCon = con;
        else
          myCon = getDbConnection();

        if (myCon) {
          savePsSubscriberSIOR(con, subscriberid(), "");

          //Only release a connection created in 
          //this function

          if (!con)
            releaseDbConnection(myCon);
        }

        subscribed(false);
      }

      return false;
    }
  }

  return false;
}

template<typename cmdType>
void SubscriberThread<cmdType>::saveDataInfo(cmdType *cmd, bool &moredata) {
  using namespace std;
  using namespace kvalobs;

  string id = subscriberid();
  string name = kvPsSubscribers::nameFromSubscriberid(id);

  if (name.empty())
    return;

  dnmi::db::Connection *con = getDbConnection();

  if (!con)
    return;

  PsSubscriberDbHelper dbHelper(*con, name);

  dbHelper.saveStationInfo(cmd->data());
  moredata = true;

  releaseDbConnection(con);
}

