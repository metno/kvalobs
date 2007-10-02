/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: PreProcessMissingData.cc,v 1.6.6.1 2007/09/27 09:02:36 paule Exp $                                                       

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
#include <PreProcessMissingData.h>
#include <kvDbGate.h>
#include <kvData.h>
#include <kvObsPgm.h>
#include <milog.h>
#include <PreProcessWorker.h>

using namespace kvalobs;
using namespace milog;

PreProcessMissingData::PreProcessMissingData()
{
}

PreProcessMissingData::~PreProcessMissingData()
{
}

void PreProcessMissingData::doJob(long                 stationId, 
				  long                 typeId, 
				  const miutil::miTime &obstime,
				  dnmi::db::Connection &con)
{
  std::ostringstream ost;
  
  ost << "PreProcessMissingData(stationid=" << stationId << ")";
  LogContext ctxt(ost.str());
  ost.str("");

  LOGINFO("doJob STARTING typeId:" << typeId << " obstime:"
	  << obstime   << std::endl);
  
  // init database connection
  kvDbGate dbGate(&con);
  bool result;
  int  missingParamCount=0;  
  int  paramObsPgmCount=0;  //Counts of params that should be in the observation
  // first fetch all observations matching stationId, obstime and typeId
  std::list<kvalobs::kvData> datalist;
  
  try{
    result = dbGate.select(datalist,
			   kvQueries::selectDataFromType(stationId,
							 typeId,
							 obstime));
  }
  catch(dnmi::db::SQLException &ex){
    LOGERROR("Exception: " << ex.what() << std::endl);
  }
  catch(...){
    LOGERROR("Unknown exception: con->exec(ctbl) .....\n");
  }
  
  if (!result) return;
  
  // .. then fetch the observation program for this station
  std::list<kvalobs::kvObsPgm> obspgmlist;
  
  try{
    result = dbGate.select(obspgmlist,
			   kvQueries::selectObsPgm(stationId,
						   obstime));
  }
  catch(dnmi::db::SQLException &ex){
    LOGERROR("Exception: " << ex.what() << std::endl);
  }
  catch(...){
    LOGERROR("Unknown exception: con->exec(ctbl) .....\n");
  }
  
  if (!result) return;
  
  // loop through obs_pgm and check if we have data for each
  // active parameter
  std::list<kvalobs::kvObsPgm>::const_iterator itop;
  
  for (itop=obspgmlist.begin(); itop!=obspgmlist.end(); itop++){
    // no missing-check for collector=TRUE
    if (itop->collector())
      continue;
    
    // check if this obspgm is active now..
    if (!itop->isOn(obstime))
      continue;
    
    paramObsPgmCount++;

    int paramid= itop->paramID();
    int level  = itop->level();
    int sensor, nr_sensor= itop->nr_sensor();
    bool countedThis=false;
      
    // loop through all sensors
    for (sensor=0; sensor<nr_sensor; sensor++){
      // check if we have this parameter
      std::list<kvalobs::kvData>::const_iterator itd;
      
      for (itd=datalist.begin(); itd!=datalist.end(); itd++){
	if (itd->paramID() == paramid &&
	    itd->level()   == level &&
	    itd->sensor()  == sensor)
	  break;
      }
      
      if (itd==datalist.end()){ // paramid not found
	if(!countedThis){
	  missingParamCount++;
	  countedThis=true;
	}
	/*
	  LOGINFO("PreProcessMissingData::doJob Missing observation for paramid:"
	  << paramid << " level:" << level << " sensor:" << sensor
	  << ", inserting one with missing-flag.."
	  << std::endl);
	*/
	// insert missing data
	float original =-32767.0;
	float corrected=-32767.0;
	//int level =  0;  // get from obs_pgm when available !!!!!!!!!!!
	kvControlInfo controlinfo;
	kvUseInfo     useinfo;
	miutil::miString failed;
	
	controlinfo.MissingFlag(kvQCFlagTypes::status_orig_and_corr_missing);
	//useinfo.setUseFlags(controlinfo);
	
	kvData data(stationId,
		    obstime,
		    original,
		    paramid,
		    miutil::miTime::nowTime(),
		    kvData::kv_typeid, //typeId,
		    sensor,
		    level,
		    corrected,
		    controlinfo,
		    useinfo,
		    failed);
	try{
	  result = dbGate.insert(data,true);
	}
	catch(dnmi::db::SQLException &ex){
	  LOGERROR("Exception: " << ex.what() << std::endl);
	}
	catch(...){
	  LOGERROR("Unknown exception: con->exec(ctbl) .....\n");
	}
      }
    }
  }
  
  if(paramObsPgmCount>0){
    if(missingParamCount>0){
      LOGINFO("Missing " << missingParamCount << " parameters. Should be " 
	      << paramObsPgmCount << " parameters for the station!");
    }else{
      LOGINFO("No missing parameters for the station!");
    }
  }else{
    LOGINFO("No parameters is expecting for the stations at this time!");
  }
  
  LOGINFO("doJob FINISHED" << std::endl);
}
