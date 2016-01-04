/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: DataCommand.cc,v 1.1.2.2 2007/09/27 09:02:21 paule Exp $                                                       

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
#include <milog/milog.h>
#include "DataCommand.h"

using namespace std;

DataCommand::DataHelper*
DataCommand::obsDataList(DataToSubscribersPtr dp) {
  CORBA::Long datai;
  char *sTmp;
  char buf[64];
  bool hasData;

  DataHelper *data;

  if (dp->dataList.empty() && dp->textDataList.empty())
    return 0;

  try {
    data = new DataHelper(dp->stationid, dp->typeid_, dp->obstime);
  } catch (...) {
    LOGERROR("NOMEM: KvDataSubscriber::DataHelper!");
    return 0;
  }

  CKvalObs::CService::ObsDataList &d = data->data;
  d.length(1);

  hasData = false;
  d[0].stationid = dp->stationid;

  if (dp->dataList.empty()) {
    d[0].dataList.length(0);
  } else {
    d[0].dataList.length(dp->dataList.size());
    datai = 0;

    for (list<kvalobs::kvData>::const_iterator it = dp->dataList.begin();
        it != dp->dataList.end(); it++) {
      hasData = true;
      d[0].dataList[datai].stationID = it->stationID();
      d[0].dataList[datai].obstime = it->obstime().isoTime().c_str();
      d[0].dataList[datai].original = it->original();
      d[0].dataList[datai].paramID = it->paramID();
      d[0].dataList[datai].tbtime = it->tbtime().isoTime().c_str();
      d[0].dataList[datai].typeID_ = it->typeID();

      sprintf(buf, "%d", it->sensor());
      sTmp = CORBA::string_dup(buf);

      if (sTmp) {
        d[0].dataList[datai].sensor = sTmp;
      } else {
        LOGERROR("DataCommand): NOMEM for <kvData::sensor>!");
      }

      d[0].dataList[datai].level = it->level();
      d[0].dataList[datai].corrected = it->corrected();
      d[0].dataList[datai].controlinfo = it->controlinfo().flagstring().c_str();
      d[0].dataList[datai].useinfo = it->useinfo().flagstring().c_str();
      d[0].dataList[datai].cfailed = it->cfailed().c_str();

      datai++;
    }

    if (datai != dp->dataList.size()) {
      LOGERROR("Inconsistent size, dataList!");
      d[0].dataList.length(datai);
    }
  }

  if (dp->textDataList.empty()) {
    d[0].textDataList.length(0);
  } else {
    datai = 0;
    d[0].textDataList.length(dp->textDataList.size());

    for (list<kvalobs::kvTextData>::const_iterator it =
        dp->textDataList.begin(); it != dp->textDataList.end(); it++) {
      hasData = true;

      d[0].textDataList[datai].stationID = it->stationID();
      d[0].textDataList[datai].obstime = it->obstime().isoTime().c_str();
      d[0].textDataList[datai].original = it->original().c_str();
      d[0].textDataList[datai].paramID = it->paramID();
      d[0].textDataList[datai].tbtime = it->tbtime().isoTime().c_str();
      d[0].textDataList[datai].typeID_ = it->typeID();

      datai++;
    }

    if (datai != dp->textDataList.size()) {
      LOGERROR("Inconsistent size, textDataList!");
      d[0].textDataList.length(datai);
    }
  }

  return data;
}

int DataCommand::execute(KvDataSubscriberInfo &sinf) {
  int ret;

  if (!sinf.thisStation(data()->stationid) || !sinf.checkStatusAndQc(data())) {
    LOGDEBUG(
        "DataCommand::excute: NOT interested in this dataset!" << endl << sinf);

    return 0;
  }

  DataHelper *d = obsDataList(data());

  if (!d) {
    LOGWARN("UNEXPECTED: No Data!");
    return 0;
  }

  try {
    ret = 0;

    subscriber->callback(d->data);
    LOGINFO(
        "SUCCESS: CALL Stationid: " << d->stationid << " typeid: " << d->typeid_ << " obstime: " << d->obstime);
  } catch (CORBA::TRANSIENT &ex) {
    LOGERROR(
        "EXCEPTION: (timeout?) Can't send <Data> event to subscriber!" << endl << "Subscriberid: " << sinf.subscriberid() << ">!");
    ret = 1;
  } catch (...) {
    LOGERROR(
        "EXCEPTION: Can't send <Data> event to subscriber!" << endl << "Subscriberid: " << sinf.subscriberid() << ">!");
    ret = -1;
  }

  delete d;
  return ret;
}
