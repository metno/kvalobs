#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <milog/milog.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <sstream>
#include <fstream>
#include <fileutil/dir.h>
#include "CollectData.h"

using namespace dnmi;
using namespace std;
using namespace CKvalObs::CDataSource;

/*
 * CollectData leser datafiler fra AutoObs og
 * aosmsd og sender dem til kvalobs.
 *
 * Datafilene er på formen 
 * 
 * decodespec
 * [metaconf]
 * data
 *
 * decodespec::=decoder [extrainfospec]
 * extrainfospec::='/' extrainfo [extrainfospec]
 * extrainfo::=key '=' val
 *
 * metaconf::=<++metaconf++> '\n'
 *            xmlcodedmetaconf '\n'
 *            <--metaconf-->   '\n'
 *
 *
 *Metainformasjon 'xmlcodedmetaconf' er kodet slik:
 *
 *<?xml version="1.0" ?>
 *<meta>
 *  <params>
 *      <param name="sa_sd">
 *          <value>1</value>
 *          <value>1</value>
 *      </param>
 *      <param name="piotr">
 *          <value>2</value>
 *      </param>
 *  </params>
 *</meta>
 *
 * data er kodet i et format som 'decoder' forventer
 * data inn på.
 *
 */

CollectData::CollectData(App &app_, bool lowpri_, const std::string &datadir_,
                         const KvDataReceiver &dataReceiver_)
    : app(app_),
      lowpri(lowpri_),
      datadir(datadir_),
      server(dataReceiver_),
      dataReady_(false),
      dataReadyCond(&dataReadyMutex) {
  start_undetached();
}

CollectData::~CollectData() {
}

bool CollectData::dataReady(unsigned long timeoutInSecs, bool &timeout) {
  timeout = false;
  unsigned long s, ns;

  omni_mutex_lock lock(dataReadyMutex);

  if (dataReady_) {
    dataReady_ = false;
    return true;
  } else {
    get_time(&s, &ns, timeoutInSecs);

    if (!dataReadyCond.timedwait(s, ns)) {
      timeout = true;
      return false;
    } else {
      bool ret = dataReady_;
      dataReady_ = false;
      return ret;
    }
  }
}

void CollectData::notifyData() {
  omni_mutex_lock lock(dataReadyMutex);
  dataReady_ = true;
  dataReadyCond.broadcast();
}

/**
 * returns true if the file could be received by kvalobs and
 *         is saved i the databse. false otherwise.
 */

bool CollectData::sendMessageToKvalobs(const std::string &msg,
                                       const std::string &obsType,
                                       bool &kvServerIsUp, bool &tryToResend) {
  Result* resTmp = app.sendDataToKvalobs(msg, obsType, server);

  if (!resTmp) {
    kvServerIsUp = false;
    tryToResend = true;
    LOGERROR(
        "Cant connect to " << server.dataReceiver().confName << ". Is it running?" << endl);
    LOGERROR("Tryed to send to: " << server.dataReceiver().confName << endl);
    return false;
  }

  LOGINFO("Sendt to server: " << server.dataReceiver().confName);

  Result_var res(resTmp);
  kvServerIsUp = true;

  if (res->res == CKvalObs::CDataSource::OK) {
    tryToResend = false;
    return true;
  } else if (res->res == CKvalObs::CDataSource::NOTSAVED) {
    tryToResend = true;
    LOGERROR(server.dataReceiver().confName << " NOTSAVED: " <<res->message);
  } else if (res->res == CKvalObs::CDataSource::ERROR) {
    LOGERROR(server.dataReceiver().confName << " ERROR: " << res->message);
    tryToResend = true;
  } else if (res->res == CKvalObs::CDataSource::NODECODER) {
    LOGERROR(server.dataReceiver().confName<<" NODECODER: " << res->message);
    tryToResend = false;
  } else if (res->res == CKvalObs::CDataSource::DECODEERROR) {
    LOGERROR(
        server.dataReceiver().confName << " DECODEERROR (rejected): " << res->message);
    tryToResend = false;
  } else {
    LOGERROR(
        "kvalobs Unknown response from " << server.dataReceiver().confName << ". Check if the code is in sync" << " with 'datasource.idl'!");
    tryToResend = false;
  }

  return false;
}

void CollectData::removeFiles() {
  FileList files;
  IFileList it;

  if (!app.getFileList(files, datadir))
    return;

  for (it = files.begin(); it != files.end(); it++) {
    if (it->isFile()) {
      unlink(it->name().c_str());
    }
  }
}

void*
CollectData::run_undetached(void*) {
  boost::posix_time::ptime nextTime;
  bool timeout;
  bool hasData = true;
  boost::posix_time::ptime dtNow;
  //  bool     firstTime=true;

  nextTime = boost::posix_time::second_clock::universal_time();

  while (!app.inShutdown()) {
    dtNow = boost::posix_time::second_clock::universal_time();

    if (!dataReady(2, timeout)) {

      if (!(hasData && dtNow > nextTime)) {
        continue;
      }
    }

    if (lowpri) {
      if (server.isNull() && !server.nextTry().is_not_a_date_time()
          && server.nextTry() > dtNow) {
        removeFiles();
        continue;
      }
    }

    if (!tryToSendSavedObservations()) {
      if (lowpri) {
        removeFiles();

        //Set the next time to retry.
        if (dtNow.time_of_day().minutes() < 30) {
          boost::posix_time::time_duration t(dtNow.time_of_day().hours(), 30,
                                             0);
          server.nextTry(boost::posix_time::ptime(dtNow.date(), t));
        } else {
          dtNow += boost::posix_time::hours(1);
          boost::posix_time::time_duration t(dtNow.time_of_day().hours(), 30,
                                             0);
          server.nextTry(boost::posix_time::ptime(dtNow.date(), t));
        }

        hasData = false;

        LOGINFO("Retry after: " << server.nextTry() << endl);
      } else {
        dtNow = boost::posix_time::second_clock::universal_time();
        nextTime += boost::posix_time::minutes(1);
        hasData = true;
      }
    } else {
      hasData = false;
    }
  }

  LOGDEBUG("Return from CollectData!");
  return 0;
}

bool CollectData::tryToSendSavedObservations() {
  const string metaconf_start("<++metaconf++>\n");
  const string metaconf_end("<--metaconf-->\n");
  FileList fileList;
  IFileList it;
  string content;
  string::size_type i;
  string type;
  string metaconf;
  bool kvServerIsUp;
  bool tryToResend;

  if (!app.getFileList(fileList, datadir)) {
    //No saved observations!
    return true;
  }

  LOGDEBUG("# saved obs: " << fileList.size());

  for (it = fileList.begin(); it != fileList.end() && !app.inShutdown(); it++) {

    if (!app.readFile(it->name(), content)) {
      LOGERROR("Cant read the file: " << it->name());
      continue;
    }

    i = content.find("\n");

    if (i == string::npos) {
      LOGERROR(
          "Format error: in savedfile: " << it->name() << endl << "Expecting '\\n'");
      unlink(it->name().c_str());
      continue;
    }

    type = content.substr(0, i);

    if (type.empty()) {
      LOGERROR(
          "Format error: in savedfile: " << it->name() << endl << "Expecting 'type'");
      unlink(it->name().c_str());
      continue;
    }

    content.erase(0, i + 1);

    /*
     *Sjekk om det er gitt meta informasjon.
     *Metainformasjon er xml kodet.
     *I datafilen er den angitt slik.
     *
     *
     *<++metaconf++>
     *<?xml version="1.0" ?>
     *<meta>
     *  <params>
     *      <param name="sa_sd">
     *          <value>1</value>
     *          <value>1</value>
     *      </param>
     *      <param name="piotr">
     *          <value>2</value>
     *      </param>
     *  </params>
     *</meta>
     *<--metaconf-->
     *
     */
    i = content.find(metaconf_start);

    if (i != string::npos) {
      content.erase(0, metaconf_start.length());

      i = content.find(metaconf_end);

      if (i == string::npos) {
        LOGERROR(
            "Format error: in savedfile: " << it->name() << endl << "Expecting '<--metaconf-->\\n'");
        unlink(it->name().c_str());
        continue;
      }

      metaconf = content.substr(0, i);
      content.erase(0, i + metaconf_end.length());

      type += "\n" + metaconf + "\n";
    }

    if (!sendMessageToKvalobs(content, type, kvServerIsUp, tryToResend)) {

      if (!tryToResend) {
        LOGERROR(
            "SAVEDOBS, " << server.dataReceiver().confName << " 'says' I should delete the observation.");
        unlink(it->name().c_str());
      } else {
        LOGERROR(
            "Cant send saved observation to " << server.dataReceiver().confName << "." << endl <<"Will try to send later!");
      }

      //Can't send the observation to kvalobs.
      //Just break out of loop.
      if (!kvServerIsUp)
        return false;
    } else {
      LOGINFO(
          server.dataReceiver().confName <<" got the observation. Deleteing local copy!");
      unlink(it->name().c_str());
    }
  }

  return true;
}

