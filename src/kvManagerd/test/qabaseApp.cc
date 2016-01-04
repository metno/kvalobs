/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: qabaseApp.cc,v 1.3.2.2 2007/09/27 09:02:36 paule Exp $                                                       

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
#include "qabaseApp.h"
#include <dnmithread/mtcout.h>
#include <string>

using namespace std;

QaBaseApp::QaBaseApp(int argn, char **argv, const char *opt[][2])
    : KvApp(argn, argv, opt),
      shutdown_(false),
      refManager(CKvalObs::CManager::CheckedInput::_nil()) {
}

QaBaseApp::~QaBaseApp() {
}

bool QaBaseApp::isOk() const {
  //For now!
  return KvApp::isOk();
}

bool QaBaseApp::sendToManager(kvalobs::kvStationInfoList &retList,
                              CKvalObs::CManager::CheckedInput_ptr manager) {
  kvalobs::IkvStationInfoList it;
  CKvalObs::StationInfoList stInfo;
  CORBA::Long i;
  CORBA::Long k;

  if (retList.empty())
    return true;

  if (CORBA::is_nil(manager)) {
    CERR("FATAL: Manager callback is NIL\n");
    return false;
  }

  stInfo.length(retList.size());
  it = retList.begin();

  for (k = 0; it != retList.end(); it++, k++) {
    stInfo[k].stationId = it->stationID();
    stInfo[k].obstime = CORBA::string_dup(it->obstime().isoTime().c_str());
    stInfo[k].typeId_ = it->typeID();
  }

  try {
    manager->checkedData(stInfo);
    return true;
  } catch (CORBA::TRANSIENT &ex) {
    CERR("WARNING:(sendToManager) Exception CORBA::TRANSIENT!\n");
  } catch (CORBA::COMM_FAILURE &ex) {
    CERR("WARNING:(sendToManager) Exception CORBA::COMM_FAILURE!\n");
  } catch (...) {
    CERR("WARNING:(sendToManager) Exception unknown!\n");
    return false;
  }

  //Shall newer happend!
  return false;
}
