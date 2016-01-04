/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: SubscriberData.cc,v 1.1.2.2 2007/09/27 09:02:22 paule Exp $                                                       

 Copyright (C) 2007 met.no

 Contact information:
 Norwegian Meteorological Institute
 Box 43 Blindern
 0313 OSLO
 NORWAY
 email: kvalobs-dev@met.no

 This file is part of KVALOBS

 KVALOBS is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License as 
 published by the Free Software Foundation; either version 2 
 of the License, or (at your option) any later version.
 
 KVALOBS is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 General Public License for more details.
 
 You should have received a copy of the GNU General Public License along 
 with KVALOBS; if not, write to the Free Software Foundation Inc., 
 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include <unistd.h>
#include <stdio.h>
#include <iomanip>
#include <time.h>
#include <sstream>
#include <string>
#include <vector>
#include <boost/lexical_cast.hpp>
#include <dnmithread/mtcout.h>
#include <milog/milog.h>
#include <miutil/trimstr.h>
#include <miutil/commastring.h>
#include <corbahelper/corbaApp.h>
#include <fileutil/dir.h>
#include <kvalobs/kvPsSubscribers.h>
#include "ServiceApp.h"
#include "SubscriberData.h"

using namespace std;
using namespace miutil;
using namespace boost;
using dnmi::thread::Thread;

std::string SubscriberData::createSubscriberid(TSubType st) {
  int COUNT_MAX = 1000;
  int count = 0;
  std::ostringstream ost;
  std::string subid;
  time_t t;
  char tBuf[30];
  struct tm tTm;
  std::string servicename;

  switch (st) {
    case DataSub:
      servicename = "data";
      break;
    case DataNotifySub:
      servicename = "datanotify";
      break;
    case HintSub:
      servicename = "kvHint";
      break;
  }

  // The chema used to create a uniqe subscriberid:
  // 'kvalobs_service_servicename_datotid_helper'
  // helper is used too garanti uniqnes if datotid and servicename 
  // is equal. 

  if (time(&t) < 0) {
    LOGERROR("createSubscriberid: time() failed!");
    return false;
  }

  if (!gmtime_r(&t, &tTm)) {
    LOGERROR("createSubscriberid: gmtime_r() failed!");
    return false;
  }

  sprintf(tBuf, "%04d%02d%02dT%02d%02d%02d", tTm.tm_year + 1900, tTm.tm_mon + 1,
          tTm.tm_mday, tTm.tm_hour, tTm.tm_min, tTm.tm_sec);

  for (count = 0; count < COUNT_MAX && subid.empty(); count++) {
    ost.str("");   //reset ost.
    ost << "kvalobs_service_" << servicename << "_" << tBuf << "_" << count;

    subid = ost.str();

    switch (st) {
      case DataSub: {
        ITDataSub it = dataSubs.find(subid);
        if (it != dataSubs.end())
          subid.erase();
      }
        break;
      case DataNotifySub: {
        ITDataNotifySub it = dataNotifySubs.find(subid);
        if (it != dataNotifySubs.end())
          subid.erase();
      }
        break;
      case HintSub: {
        ITHintSub it = hintSubs.find(subid);
        if (it != hintSubs.end())
          subid.erase();
      }
        break;
    }
  }

  if (subid.empty()) {
    LOGERROR("createSubscriberid: cant create subscriberid!\n");
  }

  return subid;
}

bool SubscriberData::removeDataNotifySubscriber(
    const std::string &subscriberid) {
  ITDataNotifySub it = dataNotifySubs.find(subscriberid);

  if (it == dataNotifySubs.end())
    return false;

  it->second.get()->subscribed(false);

  if (it->second.get()->persistent())
    return true;

  it->second.get()->terminate();
  removeSubscriberFile(subscriberid);
  return true;
}

bool SubscriberData::removeDataSubscriber(const std::string &subscriberid) {
  ITDataSub it = dataSubs.find(subscriberid);

  if (it == dataSubs.end())
    return false;

  it->second.get()->subscribed(false);

  if (it->second.get()->persistent())
    return true;

  it->second.get()->terminate();
  removeSubscriberFile(subscriberid);
  return true;
}

bool SubscriberData::removeHintSubscriber(const std::string &subscriberid) {
  ITHintSub it = hintSubs.find(subscriberid);

  if (it == hintSubs.end())
    return false;

  hintSubs.erase(it);
  removeSubscriberFile(subscriberid);

  return true;
}

bool SubscriberData::writeSubscriberFile(const std::string &subscriberid,
                                         const KvDataSubscriberInfo *si,
                                         const std::string &corbaref) {
  ofstream fs;
  string path(subPath + subscriberid + ".sub");
  miTime tNow(miTime::nowTime());

  try {
    fs.open(path.c_str(), ios_base::out | ios_base::trunc);

    if (!fs.is_open()) {
      LOGERROR("Cant create subscriber info file <" << path << ">!");
      return false;
    } else {
      LOGINFO(
          "Writing subscriber <" << subscriberid << "> " << endl <<"to file <" << path << ">!");
    }
  } catch (...) {
    LOGERROR("OUT OF MEM: cant create an instance of ofstream!");
    return false;
  }

  /*
   * OBS, OBS
   * The 'Last Call: ' line must match the 'Last Call: ' line in the function
   * updateSubscriberFile. 
   */

  fs << "Last call: " << left << setw(30) << setfill('#') << tNow.isoTime()
      << endl;

  fs << "Created: " << tNow.isoTime() << endl << "Subid: " << subscriberid
      << endl << "CORBA ref: " << corbaref << endl;

  if (!si) {
    fs.close();
    return true;
  }

  fs << "StatusId: ";

  switch (si->status()) {
    case CKvalObs::CService::All:
      fs << "All";
      break;
    case CKvalObs::CService::OnlyFailed:
      fs << "OnlyFailed";
      break;
    case CKvalObs::CService::OnlyOk:
      fs << "OnlyOk";
      break;
  }

  fs << endl;
  fs << "QcId:";

  if (!si->qcAll()) {
    for (KvDataSubscriberInfo::CITQc it = si->qcbegin(); it != si->qcend();
        it++) {
      switch (*it) {
        case CKvalObs::CService::QC1:
          fs << " QC1";
          break;
        case CKvalObs::CService::QC2d:
          fs << " QC2d";
          break;
        case CKvalObs::CService::QC2m:
          fs << " QC2m";
          break;
        case CKvalObs::CService::HQC:
          fs << " HQC";
          break;
      }
    }
  } else {
    fs << " All" << endl;
  }

  fs << "Stations:";

  KvDataSubscriberInfo::CITStations it = si->stbegin();

  if (it == si->stend()) {
    fs << "All";
  } else {
    for (; it != si->stend(); it++) {
      fs << " " << *it;
    }
  }

  fs << endl;

  fs.close();

  return true;
}

bool SubscriberData::readSubscriberFile(const std::string &fname,
                                        ServiceApp &app) {
  using namespace CKvalObs::CService;
  const string prefix("kvalobs_service_");

  StatusId statusid;
  KvDataSubscriberInfo::TStations stations;
  KvDataSubscriberInfo::TQc qc;
  miTime lastCall;
  miTime created;
  string subid;
  string cref;
  TSubType subType;

  string::size_type i;
  string buf;
  string key;
  string val;
  ifstream fs;
  string subtype;   //Subscriber type 

  LOGINFO("Reading subscriber file: '"<<fname<<"'.");

  fs.open(fname.c_str(), ios_base::in);

  if (!fs.is_open()) {
    LOGERROR("OPEN ERROR: readSubscriberFile <" << fname << ">!");
    return false;
  }

  while (!fs.eof()) {
    if (!fs.good())
      break;

    if (!getline(fs, buf))
      continue;

    trimstr(buf);

    if (buf.empty())
      continue;

    i = buf.find(":");

    if (i == string::npos) {
      LOGERROR("FORMAT ERROR: readSubscriberFile <" << fname << "> failed!");
      fs.close();
      return false;
    }

    key = buf.substr(0, i);
    val = buf.substr(i + 1);

    trimstr(val);
    trimstr(key);

    if (key.find("Last call") != string::npos) {
      lastCall = miTime(val);
    } else if (key.find("Created") != string::npos) {
      created = miTime(val);
    } else if (key.find("Subid") != string::npos) {
      subid = val;
    } else if (key.find("CORBA ref") != string::npos) {
      cref = val;
    } else if (key.find("StatusId") != string::npos) {
      if (val == "All")
        statusid = All;
      else if (val == "OnlyFailed")
        statusid = OnlyFailed;
      else
        statusid = OnlyOk;
    } else if (key.find("QcId") != string::npos) {
      CommaString cval(val, " ");
      qc = KvDataSubscriberInfo::TQc(cval.size());

      for (int i = 0; i < cval.size(); i++) {
        try {
          if (cval[i] == "QC1") {
            qc[i] = QC1;
          } else if (cval[i] == "QC2d") {
            qc[i] = QC2d;
          } else if (cval[i] == "QC2m") {
            qc[i] = QC2m;
          } else if (cval[i] == "HQC") {
            qc[i] = HQC;
          } else if (cval[i] == "All") {
            //An empty qc list represent the 'All' value
            qc.resize(0);
            break;
          } else {
            LOGERROR("readSubscriberFile unknown QcId <" << cval[i] << ">");
          }
        } catch (...) {
          LOGERROR(
              "EXCEPTION: readSubscriberFile: while setting QcId" << " index=" << i << " cval.size="<< cval.size());
        }
      }
    } else if (key.find("Stations") != string::npos) {
      CommaString cval(val, " ");
      stations = KvDataSubscriberInfo::TStations(cval.size());

      for (int i = 0; i < cval.size(); i++) {
        if (cval[i] == "All") {
          stations.resize(0);
          break;
        }

        try {
          stations[i] = atoi(cval[i].c_str());
        } catch (...) {
          LOGERROR(
              "EXCEPTION: readSubscriberFile: while setting stationid " << "index=" << i << " cval.size="<< cval.size());
        }
      }
    } else {
      LOGWARN(
          "Uknown key <" << key << ">: readSubscriberFile <" << fname << ">!");
    }
  }

  if (!fs.eof()) {
    LOGERROR("IO Error: readSubscriberFile <" << fname << "> failed!");
    fs.close();
    return false;
  }

  fs.close();

  if (subid.empty() || cref.empty()) {
    LOGERROR(
        "Format error: readSubscriberFile <" << fname << "> " << "missing key <Subid> or <CORBA ref>!");
    return false;
  }

  i = subid.find(prefix);

  if (i == string::npos) {
    LOGERROR(
        "Inavlid subscriberid [missing prefix]: subid <" << subid << ">,  readSubscriberFile <" << fname << ">!");
    return false;
  }

  i = subid.find("_", prefix.length());

  if (i == string::npos) {
    LOGERROR(
        "Inavlid subscriberid [missing subscriber type]: " << subid << ">,  readSubscriberFile <" << fname << ">!");
    return false;
  }

  subtype = subid.substr(prefix.length(), i - prefix.length());

  if (subtype == "datanotify") {
    subType = DataNotifySub;
  } else if (subtype == "data") {
    subType = DataSub;
  } else if (subtype == "kvHint") {
    subType = HintSub;
  } else {
    LOGERROR(
        "Unknown subscriber type: " << subtype << "\nsubscriberid:          " << subid << "\nreadSubscriberFile:    " << fname);
    return false;
  }

  CORBA::Object_var objPtr;

  switch (subType) {
    case DataSub: {
      objPtr = app.corbaRef(cref);
      kvDataSubscriber_var ptr = kvDataSubscriber::_narrow(objPtr);

      if (CORBA::is_nil(ptr)) {
        removeSubscriberFile(subid);
        break;
      }

      KvDataSubscriber *sub;

      try {
        sub = new KvDataSubscriber(statusid, qc, stations, app, ptr, false,
                                   subid);
      } catch (...) {
        LOGERROR("NO MEM!")
        break;
      }
      createDataSubscriber(sub);
      break;
    }
    case DataNotifySub: {
      objPtr = app.corbaRef(cref);
      kvDataNotifySubscriber_var ptr = kvDataNotifySubscriber::_narrow(objPtr);

      if (CORBA::is_nil(ptr)) {
        removeSubscriberFile(subid);
        break;
      }

      KvDataNotifySubscriber *sub;

      try {
        sub = new KvDataNotifySubscriber(statusid, qc, stations, app, ptr,
                                         false, subid);
      } catch (...) {
        LOGERROR("NO MEM!")
        break;
      }
      createDataNotifySubscriber(sub);
      break;
    }
    case HintSub: {
      objPtr = app.corbaRef(cref);
      kvHintSubscriber_var ptr = kvHintSubscriber::_narrow(objPtr);

      if (CORBA::is_nil(ptr)) {
        removeSubscriberFile(subid);
        break;
      }

      createHintSubscriber(ptr, subid);
      break;
    }
  }

  return true;
}

bool SubscriberData::updateSubscriberFile(
    const std::string &subscriberid, const miutil::miTime &timeForLastCall) {
  string path(subPath + subscriberid + ".sub");
  fstream fs;

  fs.open(path.c_str(), ios_base::out | ios_base::in);

  if (!fs.is_open()) {
    LOGERROR("Can't open file: <" << path <<">");
    return false;
  }

  /*
   * OBS, OBS, OBS, OBS, OBS
   * The 'Last Call: ' line must match the 'Last Call: ' line in the function
   * writeFileHeader. 
   */
  fs << "Last call: " << left << setw(30) << setfill('#')
      << timeForLastCall.isoTime() << endl;

  fs.close();

  return true;
}

bool SubscriberData::removeSubscriberFile(const std::string &subscriberid) {
  string path(subPath + subscriberid + ".sub");
  string newpath(subPath + "/terminated/" + subscriberid + ".sub");

  if (rename(path.c_str(), newpath.c_str()) != 0) {
    if (unlink(path.c_str()) < 0)
      return false;
  }

  return true;
}

SubscriberData::SubscriberData() {
  char *env = getenv("KVALOBS");

  if (!env) {
    subPath = "./";
  } else {
    subPath = env;

    if (!subPath.empty() && subPath[subPath.length() - 1] != '/')
      subPath += "/";

    subPath += "var/kvalobs/service/subscribers/";
  }
}

SubscriberData::SubscriberData(const std::string &fname) {
  subPath = fname;

  if (!subPath.empty() && subPath[subPath.length() - 1] != '/')
    subPath += "/";
}

SubscriberData::~SubscriberData() {
  LOGDEBUG("SubscriberData::deleted!");
}

std::string SubscriberData::createDataSubscriber(KvDataSubscriber *subscriber) {
  CorbaHelper::CorbaApp *capp = CorbaHelper::CorbaApp::getCorbaApp();

  mutex::scoped_lock lock(mutex);

  string id = subscriber->subscriberid();
  bool hasId = false;

  if (!id.empty())
    hasId = true;

  if (!hasId) {
    id = createSubscriberid(DataSub);
    subscriber->subscriberid(id);
  }

  if (id.empty()) {
    delete subscriber;
    return id;
  }

  CKvalObs::CService::kvDataSubscriber_var sub(subscriber->subscriber());

  if (!hasId)
    writeSubscriberFile(id, subscriber, capp->corbaRef(sub));

  dataSubs[id] = Thread<KvDataSubscriber>(subscriber);

  if (!dataSubs[id].start()) {
    LOGERROR("Cant start a new <data> subscriber thread!");
    dataSubs.erase(id);

    removeSubscriberFile(id);

    return string();
  }

  subscriber->subscribed(true);

  return id;
}

bool SubscriberData::createPsDataSubscriber(KvPsDataSubscriber *subscriber) {
  CorbaHelper::CorbaApp *capp = CorbaHelper::CorbaApp::getCorbaApp();

  mutex::scoped_lock lock(mutex);

  string id = subscriber->subscriberid();

  psDataSubs[id] = Thread<KvPsDataSubscriber>(subscriber);

  if (!psDataSubs[id].start()) {
    LOGERROR("Cant start a new persistent <data> subscriber thread!");
    psDataSubs.erase(id);

    return false;
  }

  subscriber->subscribed(true);

  return true;
}

bool SubscriberData::createPsDataNotifySubscriber(
    KvPsDataNotifySubscriber *subscriber) {
  CorbaHelper::CorbaApp *capp = CorbaHelper::CorbaApp::getCorbaApp();

  mutex::scoped_lock lock(mutex);

  string id = subscriber->subscriberid();

  psDataNotifySubs[id] = Thread<KvPsDataNotifySubscriber>(subscriber);

  if (!psDataNotifySubs[id].start()) {
    LOGERROR("Cant start a new persistent <notify> subscriber thread!");
    psDataSubs.erase(id);

    return false;
  }

  subscriber->subscribed(true);

  return true;
}

bool SubscriberData::registerPsDataSubscriber(
    CKvalObs::CService::kvDataSubscriberExt_ptr s, const std::string &subid) {
  mutex::scoped_lock lock(mutex);

  for (ITPsDataSub it = psDataSubs.begin(); it != psDataSubs.end(); it++) {
    if (it->first == subid) {
      it->second.get()->subscriber(s);
      it->second.get()->subscribed(true);
      it->second.get()->connection(true);
      return true;
    }
  }

  return false;
}

std::string SubscriberData::createDataNotifySubscriber(
    KvDataNotifySubscriber *subscriber) {
  CorbaHelper::CorbaApp *capp = CorbaHelper::CorbaApp::getCorbaApp();

  mutex::scoped_lock lock(mutex);

  string id = subscriber->subscriberid();
  bool hasId = false;

  if (!id.empty())
    hasId = true;

  if (!hasId) {
    id = createSubscriberid(DataNotifySub);
    subscriber->subscriberid(id);
  }

  if (id.empty()) {
    delete subscriber;
    return id;
  }

  CKvalObs::CService::kvDataNotifySubscriber_var sub(subscriber->subscriber());

  if (!hasId)
    writeSubscriberFile(id, subscriber, capp->corbaRef(sub));

  dataNotifySubs[id] = Thread<KvDataNotifySubscriber>(subscriber);

  if (!dataNotifySubs[id].start()) {
    LOGERROR("Cant start a new <datanotify> subscriber thread!");
    dataNotifySubs.erase(id);
    removeSubscriberFile(id);

    return string();
  }

  subscriber->subscribed(true);

  return id;
}

std::string SubscriberData::createHintSubscriber(
    CKvalObs::CService::kvHintSubscriber_ptr s, const std::string &subid) {
  CorbaHelper::CorbaApp *capp = CorbaHelper::CorbaApp::getCorbaApp();

  mutex::scoped_lock lock(mutex);

  string id = subid;

  if (id.empty())
    id = createSubscriberid(HintSub);

  if (id.empty()) {
    CORBA::release(s);
    return id;
  }

  if (subid.empty())
    writeSubscriberFile(id, 0, capp->corbaRef(s));

  hintSubs[id] = CKvalObs::CService::kvHintSubscriber::_duplicate(s);

  return id;
}

void SubscriberData::removeSubscriber(const std::string &subscriberid) {
  mutex::scoped_lock lock(mutex);

  if (removeDataNotifySubscriber(subscriberid))
    return;

  if (removeDataSubscriber(subscriberid))
    return;

  removeHintSubscriber(subscriberid);
}

bool SubscriberData::unregisterPsSubscriber(const std::string &id) {
  int type_ = kvalobs::kvPsSubscribers::typeFromSubscriberid(id);
  string name_ = kvalobs::kvPsSubscribers::nameFromSubscriberid(id);

  if (type_ < 0 || name_.empty())
    return false;

  if (type_ == 0) {  //data
    mutex::scoped_lock lock(mutex);

    for (ITPsDataSub it = psDataSubs.begin(); it != psDataSubs.end(); it++) {
      if (it->first == id) {
        LOGINFO("Persisten data subscriber unregister <" << id << ">.");
        it->second.get()->subscriber(kvskel::kvDataSubscriberExt::_nil());
        it->second.get()->subscribed(false);
        return true;
      }
    }
    LOGINFO("No persisten data subscriber with subid <" << id << ">.");
    return false;
  } else if (type_ == 1) {  //notify
    mutex::scoped_lock lock(mutex);

    for (ITPsDataNotifySub it = psDataNotifySubs.begin();
        it != psDataNotifySubs.end(); it++) {
      if (it->first == id) {
        LOGINFO("Persisten datanotify subscriber unregister <" << id << ">.");
        it->second.get()->subscriber(kvskel::kvDataNotifySubscriberExt::_nil());
        it->second.get()->subscribed(false);
        return true;
      }
    }
    LOGINFO("No persisten datanotify subscriber with subid <" << id << ">.");
    return false;
  } else {
    LOGINFO("Unknown subscriber type <" << type_ << "> (id <" << id << ">).");
    return false;
  }
}

bool SubscriberData::hasSubscribers() {
  mutex::scoped_lock lock(mutex);

  if (dataSubs.empty() && dataNotifySubs.empty() && psDataSubs.empty()
      && psDataNotifySubs.empty())
    return false;

  return true;

}

void SubscriberData::readAllSubscribersFromFile(ServiceApp &app) {
  dnmi::file::Dir dir;
  string file;

  if (!dir.open(subPath, "*.sub")) {
    LOGERROR(
        "DIR OPEN ERROR: Cant open directory <" << subPath << ">. " << dir.getErr());
    return;
  }

  try {
    while (dir.hasNext()) {
      file = dir.next();

      //I dont check the return value from readSubscriberFromFile
      //because the errors are reported in the function. And it doesn't
      //matter for the controll flow here.
      readSubscriberFile(subPath + file, app);
    }
  } catch (dnmi::file::DirException &ex) {
    LOGERROR(
        "SUBSCRIBER READ DIR ERROR: cant read the subscribers in the\n" << "directory <" << subPath << ">. " << ex.what());
  } catch (...) {
    LOGERROR(
        "UNEXPECTED EXCEPTION: While reading the subscribers in the\n" << "directory <" << subPath << ">");
  }
}

void SubscriberData::sendKvHintUp() {
  ITHintSub it = hintSubs.begin();

  while (it != hintSubs.end()) {
    try {
      it->second->kvUp();
      it++;
    } catch (...) {
      LOGINFO(
          "HINT SUBSCRIBER NOT LISTNING: Deletes the subscriber <" << it->first << ">!");
      ITHintSub tit = it;
      it++;
      hintSubs.erase(tit);
    }
  }
}

void SubscriberData::sendKvHintDown() {
  ITHintSub it = hintSubs.begin();

  while (it != hintSubs.end()) {
    try {
      it->second->kvDown();
      it++;
    } catch (...) {
      LOGINFO(
          "HINT SUBSCRIBER NOT LISTNING: Deletes the subscriber <" << it->first << ">!");
      ITHintSub tit = it;
      it++;
      hintSubs.erase(tit);
    }
  }
}

void SubscriberData::forAllSubscribers(DataToSubscribersPtr data2sub) {
  mutex::scoped_lock lock(mutex);

  for (ITDataSub it = dataSubs.begin(); it != dataSubs.end(); it++) {
    DataCommand *dcmd = 0;

    try {
      dcmd = new DataCommand(data2sub);

      LOGDEBUG(
          "CALL: subscriber <" << it->first << "> stationid=" << data2sub->stationid << " typeid: " << data2sub->typeid_ << " obstime: " << data2sub->obstime);

      dcmd->subscriberid(it->first);
      it->second.get()->put(dcmd);
    } catch (const dnmi::thread::QueSuspended &ex) {
      LOGINFO("QueSuspend: subscriber <" << it->first << ">");
      delete dcmd;
    } catch (...) {
      LOGERROR("NOMEM: forAllSubscribers!");
      delete dcmd;
    }
  }

  for (ITPsDataSub it = psDataSubs.begin(); it != psDataSubs.end(); it++) {
    DataCommandExt *dcmd = 0;

    try {
      dcmd = new DataCommandExt(data2sub);

      LOGDEBUG(
          "CALL: ps subscriber <" << it->first << "> stationid=" << data2sub->stationid << " typeid: " << data2sub->typeid_ << " obstime: " << data2sub->obstime);

      it->second.get()->put(dcmd);
    } catch (const dnmi::thread::QueSuspended &ex) {
      LOGINFO("QueSuspend: subscriber <" << it->first << ">");
      delete dcmd;
    } catch (...) {
      LOGERROR("NOMEM: forAllSubscribers!");
      delete dcmd;
    }
  }

  for (ITDataNotifySub it = dataNotifySubs.begin(); it != dataNotifySubs.end();
      it++) {
    DataNotifyCommand *dcmd = 0;

    try {
      dcmd = new DataNotifyCommand(data2sub);

      LOGDEBUG(
          "CALL: subscriber <" << it->first << "> stationid=" << data2sub->stationid << " typeid: " << data2sub->typeid_ << " obstime: " << data2sub->obstime);

      dcmd->subscriberid(it->first);
      it->second.get()->put(dcmd);
    } catch (const dnmi::thread::QueSuspended &ex) {
      LOGINFO("QueSuspend: subscriber <" << it->first << ">");
      delete dcmd;
    } catch (...) {
      LOGERROR("NOMEM: forAllSubscribers!");
      delete dcmd;
    }
  }

  for (ITPsDataNotifySub it = psDataNotifySubs.begin();
      it != psDataNotifySubs.end(); it++) {
    DataNotifyCommandExt *dcmd = 0;

    try {
      dcmd = new DataNotifyCommandExt(data2sub);

      LOGDEBUG(
          "CALL: ps notify subscriber <" << it->first << "> stationid=" << data2sub->stationid << " typeid: " << data2sub->typeid_ << " obstime: " << data2sub->obstime);

      it->second.get()->put(dcmd);
    } catch (const dnmi::thread::QueSuspended &ex) {
      LOGINFO("QueSuspend: subscriber <" << it->first << ">");
      delete dcmd;
    } catch (...) {
      LOGERROR("NOMEM: forAllSubscribers!");
      delete dcmd;
    }
  }

}

/*
 * Walk through all the subscribers lists and find all subscribers that
 * we cant connect to and remove them from our list of subscribers.
 */
void SubscriberData::removeTerminatedSubscribers() {
  mutex::scoped_lock lock(mutex);

  int ret;

  for (ITDataSub it = dataSubs.begin(); it != dataSubs.end();) {
    if (it->second.join(ret)) {
      ITDataSub tit = it;
      it++;

      if (ret > 0) {
        if (ret == 1) {
          LOGINFO("UNSUBSCRIBE: DataSubscriber <" << tit->first << "> removed!");
        } else {
          LOGERROR(
              "NOT RESPONDING: DataSubscriber <" << tit->first << "> removed!");
        }
        //The subscriber has unsubscribed or we have lost connection
        //with the subscriber. Remove the subscriber file.
        removeSubscriberFile(tit->first);
      }

      dataSubs.erase(tit);
    } else {
      it++;
    }
  }

  for (ITDataNotifySub it = dataNotifySubs.begin(); it != dataNotifySubs.end();
      ) {
    if (it->second.join(ret)) {
      ITDataNotifySub tit = it;
      it++;

      if (ret > 0) {
        if (ret == 1) {
          LOGINFO(
              "UNSUBSCRIBE: DataNotifySubscriber <" << tit->first << "> removed!");
        } else {
          LOGERROR(
              "NOT RESPONDING: DataNotifySubscriber <" << tit->first << "> removed!");
        }

        //The subscriber has unsubscribed or we have lost connection
        //with the subscriber. Remove the subscriber file.
        removeSubscriberFile(tit->first);
      }

      dataNotifySubs.erase(tit);
    } else {
      it++;
    }
  }
}

void SubscriberData::joinAllSubscribers() {
  mutex::scoped_lock lock(mutex);

  for (ITDataSub it = dataSubs.begin(); it != dataSubs.end(); it++)
    it->second.join();

  for (ITPsDataSub it = psDataSubs.begin(); it != psDataSubs.end(); it++)
    it->second.join(true);

  for (ITDataNotifySub it = dataNotifySubs.begin(); it != dataNotifySubs.end();
      it++)
    it->second.join(true);

  for (ITPsDataNotifySub it = psDataNotifySubs.begin();
      it != psDataNotifySubs.end(); it++)
    it->second.join(true);
}
