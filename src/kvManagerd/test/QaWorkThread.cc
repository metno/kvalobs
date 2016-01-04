/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: QaWorkThread.cc,v 1.5.2.2 2007/09/27 09:02:36 paule Exp $                                                       

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
#include <dnmithread/mtcout.h>
#include "QaWorkThread.h"
#include "qabaseApp.h"
#include "QaWorkCommand.h"

using namespace kvalobs;
using namespace std;

void QaWork::operator()() {
  dnmi::thread::CommandBase *cmd;
  QaWorkCommand *work;
  IkvStationInfoList it;
  kvStationInfoList stationInfoList;
  kvalobs::kvStationInfoList retList;
  CERR("QaWork: starting work thread!\n");

  while (!app.inShutdown()) {
    cmd = app.getInQue().get();

    CERR("QaWork: command received....\n");

    if (!cmd)
      continue;

    work = dynamic_cast<QaWorkCommand*>(cmd);

    if (!work) {
      CERR("QaWork: Unexpected command ....\n");
      delete cmd;
      continue;
    }

    CKvalObs::CManager::CheckedInput_var manager = work->getCallback();
    stationInfoList = work->getStationInfo();
    it = stationInfoList.begin();

    //The list wil have one and only one element when it is received from 
    //kvManager.

    if (it != stationInfoList.end()) {  //Paranoia
      retList.clear();
      doWork(*it, retList);

      if (!retList.empty()) {
        //Return the result to kvManager.
        app.sendToManager(retList, manager);
      }
    }

    delete work;
  }
}

/**
 * The retList must contain the result that is to be returned to
 * the kvManager. The result may contain more parameters and there
 * may be results for additional stations. 
 */

void QaWork::doWork(kvalobs::kvStationInfo &params_,
                    kvalobs::kvStationInfoList &retList) {

  CERR("QaWork::doWork" << std::endl);

  retList.push_back(params_);

  CERR(
      "Processing: StationID: " << params_.stationID() << endl << "              obstime: " << params_.obstime() << endl << "               typeID: " << params_.typeID() << endl);

}
