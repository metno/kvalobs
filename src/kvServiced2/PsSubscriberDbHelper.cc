/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: PsSubscriberDbHelper.cc,v 1.1.2.2 2007/09/27 09:02:22 paule Exp $                                                       

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
#include <kvalobs/kvDbGate.h>
#include "PsSubscriberDbHelper.h"
#include "kvPsSubscriber.h"

using namespace kvalobs;
using namespace std;

bool PsSubscriberDbHelper::saveStationInfo(DataToSubscribersPtr data) {
  kvPsSubscriber pssub(subscriber);

  pssub.set(data->stationid, data->typeid_, data->obstime);

  kvDbGate gate(&con);

  if (!gate.insert(pssub)) {
    if (gate.getError() == kvDbGate::Duplicate) {
      if (!gate.update(pssub)) {
        IDLOGWARN(subscriber, "DBERROR: " << gate.getErrorStr());
        return false;
      }

      IDLOGINFO(
          subscriber,
          "STINFO touched: sid: " << data->stationid << " tid: " << data->typeid_ << " " << data->obstime);

      LOGDEBUG(
          "STINFO touched: sid: " << data->stationid << " tid: " << data->typeid_ << " " << data->obstime);
    }
  } else {
    IDLOGINFO(
        subscriber,
        "STINFO saved: sid: " << data->stationid << " tid: " << data->typeid_ << " " << data->obstime);

    LOGDEBUG(
        "STINFO saved: sid: " << data->stationid << " tid: " << data->typeid_ << " " << data->obstime);
  }

  return true;
}

std::list<DataToSubscribersPtr> PsSubscriberDbHelper::getStationInfo(
    int maxrows) {
  std::list<DataToSubscribersPtr> retData;
  std::list<kvPsSubscriber> pssubList;
  ostringstream ost;
  kvDbGate gate(&con);
  DataToSubscribersPtr dts;
  string tblName(kvPsSubscriber(subscriber).tableName());

  ost << " ORDER BY stationid, typeid, obstime DESC LIMIT " << maxrows;

  if (!gate.select(pssubList, ost.str(), tblName)) {
    IDLOGWARN(
        subscriber,
        "DBERROR: SELECT failed (" << tblName <<"): " << gate.getErrorStr());

    LOGDEBUG(
        "DBERROR: SELECT failed (" << tblName <<"): " << gate.getErrorStr());
    return retData;
  }

  for (std::list<kvPsSubscriber>::iterator it = pssubList.begin();
      it != pssubList.end(); it++) {
    try {
      dts.reset(new DataToSubscribers());
    } catch (...) {
      LOGERROR("PsSubscriberDbHelper: NO MEM!");
      return std::list<DataToSubscribersPtr>();
    }

    dts->stationid = it->stationID();
    dts->typeid_ = it->typeID();
    dts->obstime = it->obstime();

    if (!gate.select(
        dts->dataList,
        kvQueries::selectData(it->stationID(), it->typeID(), it->obstime()))) {
      IDLOGWARN(
          subscriber,
          "DBERROR: SELECT failed (data): sid: " << it->stationID() <<" tid: " << it->typeID() << " " << it->obstime() << endl << gate.getErrorStr());
      LOGDEBUG(
          "DBERROR: SELECT failed (data): sid: " << it->stationID() <<" tid: " << it->typeID() << " " << it->obstime() << endl << gate.getErrorStr());
      return std::list<DataToSubscribersPtr>();
    }

    if (!gate.select(
        dts->textDataList,
        kvQueries::selectTextData(it->stationID(), it->typeID(),
                                  it->obstime()))) {
      IDLOGWARN(
          subscriber,
          "DBERROR: SELECT failed (text_data): sid: " << it->stationID() <<" tid: " << it->typeID() << " " << it->obstime() << endl << gate.getErrorStr());
      LOGDEBUG(
          "DBERROR: SELECT failed (text_data): sid: " << it->stationID() <<" tid: " << it->typeID() << " " << it->obstime() << endl << gate.getErrorStr());
      return std::list<DataToSubscribersPtr>();
    }

    retData.push_back(dts);
  }

  return retData;
}

void PsSubscriberDbHelper::deleteExperiedStationInfo(int delete_after_hours) {
}

bool PsSubscriberDbHelper::removeStationInfo(DataToSubscribersPtr data) {
  kvDbGate gate(&con);
  kvPsSubscriber pssub(subscriber);
  bool error = true;

  pssub.set(data->stationid, data->typeid_, data->obstime);

  if (!gate.remove(pssub)) {
    error = false;
    IDLOGWARN(
        subscriber,
        "DBERROR: DELETE failed: sid" << data->stationid << " tid: " << data->typeid_ << " " << data->obstime);

    LOGDEBUG(
        "DBERROR: DELETE failed: sid" << data->stationid << " tid: " << data->typeid_ << " " << data->obstime);
  }

  return error;
}

