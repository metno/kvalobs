/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: NewDataJob.cc,v 1.13.6.1 2007/09/27 09:02:36 paule Exp $                                                       

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
#include <NewDataJob.h>
#include <milog/milog.h>

#include <list>
#include <miTime>
#include <kvDbGate.h>
#include <kvQueries.h>
#include <kvKeyVal.h>

using namespace kvalobs;

NewDataJob::NewDataJob()
{
};


NewDataJob::~NewDataJob()
{ 
  LOGDEBUG("DTOR: NewDataJob");
}


/* 
   This job will scan the table 'data' for data that has not been processed by 
   QaBase. This job will run once, at the startup of kvManagerd.
   
   Background: 
   kvManagerd has been down while kvDataInputd was alive. kvDataInputd 
   has added new observations to 'data' that have not been checked by QaBase

   To help in the  search:
   - QaBase saves timestamp (tbtime) for data of last successfull run
   - in addition, uses the fqclevel-bit of the data's controlflag
*/
void NewDataJob::doJob(dnmi::db::Connection &con)
{
  milog::LogContext context("NewDataJob");

  LOGDEBUG(jobName() << ": Do job! *******************************");

  /*
    Start searching for unprocessed data from tbtime>= startime
    Ignore data younger than stoptime
  */ 
  miutil::miTime startime = miutil::miTime::nowTime();
  miutil::miTime stoptime = miutil::miTime::nowTime();

  bool result;

  // init database connection
  kvDbGate dbGate(&con);
  std::list<kvKeyVal> values;

  /*
    ==============================================================
    First, fetch tabletime for qabase's last successfully processed data
    fetch key-values from DB
    ==============================================================
  */
  try{
    result = dbGate.select(values,
			   kvQueries::selectKeyValues("kvQabased","checkpoint"));
  }
  catch(dnmi::db::SQLException &ex){
    LOGERROR("Exception: " << ex.what() << std::endl);
  }
  catch(...){
    LOGERROR("Unknown exception: con->exec(ctbl) .....\n");
  }
  
  LOGDEBUG(jobName() << ": Found " << values.size() << " key-value-pairs");
  
  if (!result) return;
  
  // fetch the tabletime for previous qabase_run
  std::list<kvKeyVal>::const_iterator itk;
  
  for (itk=values.begin(); itk!=values.end(); itk++){
    miutil::miString s= itk->val();
    if (miutil::miTime::isValid(s))
      startime= miutil::miTime(s);
  }
  
  LOGDEBUG(jobName() << ": Got tabletime for previous qabase run:" << startime);
  
  // to be on the safe side, reduce startime by one hour
  startime.addHour(-1);
  
  /*
    ==============================================================
    Next, fetch all observations matching tbtime>=startime and tbtime<=stoptime
    ==============================================================
  */
  std::list<kvData> datalist;
  
  try{
    result = dbGate.select(datalist,
			   kvQueries::selectDataByTabletime(startime,stoptime,
							    "stationid,typeid,obstime"));
  }
  catch(dnmi::db::SQLException &ex){
    LOGERROR("Exception: " << ex.what() << std::endl);
  }
  catch(...){
    LOGERROR("Unknown exception: con->exec(ctbl) .....\n");
  }
  
  if (!result) return;

  /*
    ==============================================================
    loop through data and check for unchecked entries
    ==============================================================
  */
  int num_mis_params= 0;
  int num_mis_stations= 0;
  int stationid= -1;
  int typeID   = -1;
  miutil::miTime obstime= miutil::miTime::nowTime();

  kvStationInfo stationinfo(stationid,obstime,typeID);
  std::list<kvData>::const_iterator itd;
  
  for (itd=datalist.begin(); itd!=datalist.end(); itd++){

    kvControlInfo controlinfo= itd->controlinfo();

    if (controlinfo.flag(0) != 0)
      continue;
    
    if (itd->stationID() != stationid ||
	itd->obstime()   != obstime   ||
	itd->typeID()    != typeID){

      // post the previous stationinfo
      if (stationid!=-1) postToQue(stationinfo);

      // add a new stationinfo
      num_mis_stations++;

      stationid=                        itd->stationID();
      obstime=                          itd->obstime();
      typeID=                           itd->typeID();

      LOGDEBUG(jobName() << " : " << num_mis_stations
	       << ", Adding new station:" << stationid
	       << " for obstime:" << obstime);
    
      stationinfo = kvStationInfo(stationid, obstime, typeID);
    }
    
    num_mis_params++;

  } // data-loop

  // post the final stationinfo
  if (stationid!=-1) postToQue(stationinfo);
  
  LOGDEBUG(jobName() << " FINISHED. Pushed " << num_mis_stations
	   << " stations, total of "
	   << num_mis_params << " dataentries for processing to qabase");
}
