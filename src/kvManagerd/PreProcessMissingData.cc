/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: PreProcessMissingData.cc,v 1.3.2.6 2007/09/27 09:02:35 paule Exp $                                                       

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
#include <stdlib.h>
#include "PreProcessMissingData.h"
#include <kvalobs/kvDbGate.h>
#include <kvalobs/kvData.h>
#include <kvalobs/kvObsPgm.h>
#include <milog/milog.h>
#include "PreProcessWorker.h"

using namespace kvalobs;
using namespace milog;

PreProcessMissingData::PreProcessMissingData()
{
}

PreProcessMissingData::~PreProcessMissingData()
{
}

void 
PreProcessMissingData::doJob(long                 stationId, 
						     long                 typeId, 
							 const miutil::miTime &obstime,
							 dnmi::db::Connection &con)
{
  	std::ostringstream ost;
  	ost << "PreProcessMissingData(stationid=" << stationId << ")";
  	LogContext ctxt(ost.str());
  	ost.str("");

  	if(typeId<0){
    	LOGDEBUG1("Generated data has is not checked for missing. typeId=" << 
	      		  typeId);
    	return;
  	}else{
    	LOGDEBUG("doJob STARTING typeId:" << typeId << " obstime:"
	    		<< obstime   << std::endl);
  	}
  	// init database connection
  	kvDbGate dbGate(&con);
  	bool     result;
  	int      missingParamCount=0;  
  	int      paramObsPgmCount=0;  //Counts of params that should be in the observation
  
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
  
  	if (!result) 
  		return;
  
  	// .. then fetch the observation program for this station
  	std::list<kvalobs::kvObsPgm> obspgmlist;
  
  	try{
    	result = dbGate.select(obspgmlist,
							   kvQueries::selectObsPgm(stationId,
													   typeId,
						   	   						   obstime)
						   	   );
  	}
  	catch(dnmi::db::SQLException &ex){
    	LOGERROR("Exception: " << ex.what() << std::endl);
  	}
  	catch(...){
    	LOGERROR("Unknown exception: con->exec(ctbl) .....\n");
  	}
  
  	if (!result) 
  		return;
  
  	miutil::miTime tbtime(miutil::miTime::nowTime());
  	
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
      
      		if (itd==datalist.end()){ //paramid not found
				if(!countedThis){
	  				missingParamCount++;
	  				countedThis=true;
				}
	
				// insert missing data
				float            original =-32767.0;
				float            corrected=-32767.0;
				kvControlInfo    controlinfo;
				kvUseInfo        useinfo;
				miutil::miString failed;
		
				controlinfo.MissingFlag(kvQCFlagTypes::status_orig_and_corr_missing);
				
				//NOTE:
				//Børge Moe 2005.12.2005
				//
				//This should be taken care of in QAbase. But it is missed!
				//Until this i fixed in QAbase it is set here.
				useinfo.set(1, 8); //original value is missing
				
				//useinfo.setUseFlags(controlinfo);
	
	
				kvData data(stationId,
			    			obstime,
			    			original,
			    			paramid,
			    			tbtime,
			    			typeId,
			    			sensor,
			    			level,
			    			corrected,
			    			controlinfo,
			    			useinfo,
			    			failed);
				try{
	  				result = dbGate.insert(data,false);
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
      		LOGDEBUG("No missing parameters for the station!");
    	}
  	}else{
    	LOGDEBUG("No parameters is expecting for the stations at this time!");
  	}
  
  	LOGDEBUG("doJob FINISHED" << std::endl);
}
