/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: ServiceDataCommand.cc,v 1.3.2.1 2007/09/27 09:02:35 paule Exp $                                                       

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
#include <kvalobs/kvWorkelement.h>
#include <milog/milog.h>
#include <kvalobs/kvDbGate.h>
#include "ServiceDataCommand.h"

using namespace kvalobs;
using namespace std;

ServiceDataCommand::
ServiceDataCommand(const CKvalObs::StationInfoList &stInfo)
  :CheckedDataCommandBase(stInfo)
{
}

ServiceDataCommand::
~ServiceDataCommand()
{
}


bool       
ServiceDataCommand::
executeImpl()
{
  const int  LIMIT=10;
  ostringstream ost;
  int errorCount=0;
  list<kvalobs::kvWorkelement> workList;
  list<kvalobs::kvWorkelement>::iterator it;
  dnmi::db::Connection *con=helper().connection();

  milog::LogContext logContext("kvServiced");
  
  if(!con)
    return false;

  kvDbGate gate(con);

  ost << "WHERE  service_stop IS NOT NULL "  
      << "ORDER BY stationid, typeid, obstime DESC "
      << "LIMIT " << LIMIT;

  
  if(!gate.select(workList, ost.str(), "workque")){
    LOGERROR("DBERROR: Cant retrive data from workque!" <<
	     "Querry: " << ost.str() << endl <<
	     "Reason: " << gate.getErrorStr());
    return false;
  }
    

  for(it=workList.begin(); it!=workList.end(); it++){
    if(!gate.remove(*it, "workque")){
      errorCount++;
      LOGERROR("DBERROR: can't remove from workque: " << endl <<
	       "-- Stationid: " << it->stationID() << " obstime: " <<
	       it->obstime() << " typeid: " << it->typeID() <<
	       endl << "Reason: " << gate.getErrorStr());
    }

    if(!gate.insert(*it, "workstatistik", true)){
      LOGERROR("DBERROR: cant insert into workstatistik: " << endl <<
	       "-- Stationid: " << it->stationID() << " obstime: " <<
	       it->obstime() << " typeid: " << it->typeID() <<
	       endl << "Reason: " << gate.getErrorStr());
    }
  }
  if (errorCount > 0) {
	LOGINFO("Cleaned up work queue: error count=" << errorCount <<
		" #workelements: " << workList.size());
   } else {
    LOGDEBUG("Cleaned up work queue: no errors");
   }
}



#if 0

bool       
ServiceDataCommand::
executeImpl()
{
  list<kvWorkelement> workList;
  bool                hasElement;
  ostringstream ost;
  dnmi::db::Connection *con=helper().connection();

  milog::LogContext logContext("kvServiced");
  
  if(!con)
    return false;

  kvDbGate gate(con);

  kvalobs::IkvStationInfoList it=getStationInfo().begin();

  for(; it!=getStationInfo().end(); it++){
    ost.str("");
    hasElement=true;

    try{
      con->beginTransaction();
    }
    catch(...){
      LOGERROR("DBERROR (transaction begin) " <<endl << con->lastError());
      return false;
    }
 
   
    ost << "WHERE  stationid=" << it->stationID() 
	<< "  AND obstime='" << it->obstime().isoTime() 
	<< "' AND typeid=" << it->typeID();

    if(!gate.select(workList, ost.str(), "workque")){
      LOGERROR("DBERROR: Cant retrive from  workque!" << endl <<
	       "Reason: " << gate.getErrorStr());
      hasElement=false;
    }
        
    if(workList.empty()){
      try{
	con->endTransaction();
      }
      catch(...){
	LOGERROR("DBERROR (transaction end): " << endl << con->lastError());
	return false;
      }
      continue;
    }

    if(hasElement){
      if(!gate.insert(workList.front(), "workstatistik")){
	LOGERROR("DBERROR: cant insert workelement in workstatistik!"<<endl<<
		 "Reason: " << gate.getErrorStr());
      }
    }
    
    ost.str("");
    
    ost << "DELETE FROM workque WHERE  stationid=" << it->stationID() 
	<< "  AND obstime='" << it->obstime().isoTime() 
	<< "' AND typeid=" << it->typeID();

    if(!gate.exec(ost.str())){
      LOGERROR("DBERROR: cant delete element from workque!");

      if(hasElement){
	try{
	  con->rollBack();
	  continue;
	}
	catch(...){
	  LOGERROR("DBERROR (transaction rollback): " << endl << 
		   con->lastError());
	  return false;
	}
      }	
    }

    try{
      con->endTransaction();
    }
    catch(...){
      LOGERROR("DBERROR (transaction end): " << endl << con->lastError());
      return false;
    }
  }
  
  return true;
} 

#endif
