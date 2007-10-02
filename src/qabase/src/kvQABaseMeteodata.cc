/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvQABaseMeteodata.cc,v 1.41.2.5 2007/09/27 09:02:38 paule Exp $                                                       

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

#include "../include/kvQABaseMeteodata.h"
#include "../include/kvQABaseTypes.h"

#include <set>
#include <sstream>

#include <milog/milog.h>

using namespace miutil;

kvQABaseMeteodata::kvQABaseMeteodata(kvQABaseDBConnection& dbcon,
				     kvalobs::kvStationInfo& stationinfo)
  : dbcon_(dbcon), stationinfo_(stationinfo),
    qcx_(""), medium_qcx_("")
{
}

// clear datalists
void
kvQABaseMeteodata::clear()
{
  obsdata.clear();
  textdata.clear();
  modeldata.clear();

  minObsTime_.clear();
  maxObsTime_.clear();

  minTextTime_.clear();
  maxTextTime_.clear();

  minModelTime_.clear();
  maxModelTime_.clear();
}

/*
  load all observation-data for one station, several timesteps
*/
bool
kvQABaseMeteodata::loadObsData(const int sid,
			       const int tstart,
			       const int tstop)
{
  IDLOGDEBUG("html", "kvQABaseMeteodata::loadObsData "
	     << std::endl);
  
  if (stationinfo_.obstime().undef()) return false;

  // find proper miTimes for required timespan
  miTime stime= stationinfo_.obstime();
  stime.addMin(tstop);
  miTime etime= stationinfo_.obstime();
  etime.addMin(tstart);

  IDLOGDEBUG("html", "kvQABaseMeteodata::loadObsData for times: "
	     << stime << " until " << etime
	     << std::endl);
  // check if required data already exist..
  if (obsdata.count(sid) > 0){
    if (stime >= minObsTime_[sid] && etime <= maxObsTime_[sid]){
      IDLOGDEBUG("html", "...already loaded..."
		 << std::endl);
      return true;
    } else if (stime < minObsTime_[sid]){
      minObsTime_[sid]= stime;
    } else if (etime > maxObsTime_[sid]){
      maxObsTime_[sid]= etime;
    }
  } else {
    minObsTime_[sid]= stime;
    maxObsTime_[sid]= etime;
  }

  // fetch observation-data from db
  if (!dbcon_.getObservations(sid,stime,etime,obsdata[sid])){
    IDLOGERROR("html", "kvQABaseMeteodata::loadObsData ERROR"
	       << " could not get observations from DB."
	       << std::endl);
    return false;
  }
    
  return true;
}


/*
  load all text-data for one station, several timesteps
*/
bool
kvQABaseMeteodata::loadTextData(const int sid,
				const int tstart,
				const int tstop)
{
  IDLOGDEBUG("html", "kvQABaseMeteodata::loadTextData "
	     << std::endl);
  if (stationinfo_.obstime().undef()) return false;

  // find proper miTimes for required timespan
  miTime stime= stationinfo_.obstime();
  stime.addMin(tstop);
  miTime etime= stationinfo_.obstime();
  etime.addMin(tstart);

  IDLOGDEBUG("html", "kvQABaseMeteodata::loadTextData for times: "
	     << stime << " until " << etime
	     << std::endl);
  // check if required data already exist..
  if (textdata.count(sid) > 0){
    if (stime >= minTextTime_[sid] && etime <= maxTextTime_[sid]){
      IDLOGDEBUG("html", "...already loaded..."
		 << std::endl);
      return true;
    } else if (stime < minTextTime_[sid]){
      minTextTime_[sid]= stime;
    } else if (etime > maxTextTime_[sid]){
      maxTextTime_[sid]= etime;
    }
  } else {
    minTextTime_[sid]= stime;
    maxTextTime_[sid]= etime;
  }

  // fetch text-data from db
  if (!dbcon_.getTextData(sid,stime,etime,textdata[sid])){
    IDLOGERROR("html", "kvQABaseMeteodata::loadTextData ERROR"
	       << " could not get observations from DB."
	       << std::endl);
    return false;
  }
    
  return true;
}


/*
  load all model-data for one station, several timesteps
*/
bool
kvQABaseMeteodata::loadModelData(const int sid,
				 const int tstart,
				 const int tstop)
{
  IDLOGDEBUG("html", "kvQABaseMeteodata::loadModelData "
	     << std::endl);
  if (stationinfo_.obstime().undef()) return false;

  // find proper miTimes for required timespan
  miTime stime= stationinfo_.obstime();
  stime.addMin(tstop);
  miTime etime= stationinfo_.obstime();
  etime.addMin(tstart);

  IDLOGDEBUG("html", "kvQABaseMeteodata::loadModelData for times: "
	     << stime << " until " << etime
	     << std::endl);
  // check if required data already exist..
  if (modeldata.count(sid) > 0){
    if (stime >= minModelTime_[sid] && etime <= maxModelTime_[sid]){
      IDLOGDEBUG("html", "...allready loaded..."
		 << std::endl);
      return true;
    } else if (stime < minModelTime_[sid]){
      minModelTime_[sid]= stime;
    } else if (etime > maxModelTime_[sid]){
      maxModelTime_[sid]= etime;
    }
  } else {
    minModelTime_[sid]= stime;
    maxModelTime_[sid]= etime;
  }

  // fetch  model-data from db
  if (!dbcon_.getModelData(sid,stime,etime,modeldata[sid])){
    IDLOGERROR("html", "kvQABaseMeteodata::loadModelData ERROR"
	       << " could not get model-data from DB."
	       << std::endl);
    return false;
  }
    
  return true;
}


// return data for one parameter
bool
kvQABaseMeteodata::data_asPerl(const std::string qcx,            // check-id
			       const std::string medium_qcx,     // medium class. of check
			       const int language,               // algorithm type
			       const kvQABaseScriptManager& sman,// active Script-Manager
			       const std::list<kvalobs::kvObsPgm>& oplist, // obs_pgm
			       bool& skipcheck,                  // obs_pgm may skip check..
			       std::string& data)                // return perl-data
{
  qcx_       = qcx;
  medium_qcx_= medium_qcx;
  language_  = language;

  data.clear();

  data+="\n";
  data+="#==========================================\n";
  data+="#  Data\n";
  data+="#\n";

  bool result;

  result= sman.getVariables(kvQABase::obs_data, obs_vars);
  if (!result) {
    IDLOGERROR("html", "kvQABaseMeteodata::data_asPerl ERROR"
	       << " could not get observation variables."
	       << std::endl);
    return false;
  }
  result= sman.getVariables(kvQABase::refobs_data, refobs_vars);
  if (!result) {
    IDLOGERROR("html", "kvQABaseMeteodata::data_asPerl ERROR"
	       << " could not get reference observation variables."
	       << std::endl);
    return false;
  }
  result= sman.getVariables(kvQABase::model_data, model_vars);
  if (!result) {
    IDLOGERROR("html", "kvQABaseMeteodata::data_asPerl ERROR"
	       << " could not get model variables."
	       << std::endl);
    return false;
  }


  // if no data needed - return
  if (obs_vars.pars.size() == 0 &&
      refobs_vars.pars.size() == 0 &&
      model_vars.pars.size()==0){
    IDLOGINFO("html","No observation- or model-data needed: RETURN");
    return true;
  }

  skipcheck= false;
  // using observation_program // if normal station:
  if (oplist.size() > 0){
    // ensure that requested obs.parameters are active in observation_program
    std::list<kvalobs::kvObsPgm>::const_iterator itrop;
    std::vector<kvQABase::script_par>::const_iterator itrpa;
    for (itrpa=obs_vars.pars.begin(); itrpa!=obs_vars.pars.end(); itrpa++){
      if (!itrpa->normal) continue;
      int pid= itrpa->paramid;
      std::string vname= itrpa->name;

      IDLOGINFO("html", "Checking obs_pgm for par:" << vname
		<< " nr:" << pid << std::endl);
      for (itrop=oplist.begin(); itrop!=oplist.end(); itrop++){
	if (itrop->paramID() == pid){
	  break;
	  /* ============= REMOVED 2003-12-18
	  // 	if (itrop->collector()) // collector: always check
	  // 	  break;
	  // 	if (!itrop->isOn(obstime_)) // is parameter inactive?
	  // 	  skipcheck= true;
	  // 	break;
	  */
	}
      }
      if (skipcheck || itrop==oplist.end()){
	// parameter inactive or not found in obs_program
	IDLOGINFO("html", " SKIPPING check "
		  << (skipcheck ? "(inactive in obs_program) !"
		      : "(param not found in obs_program for station) !")
		  << std::endl);
	skipcheck= true;
	return true;
      }
    }
  }
  

  // Read and put data into script_var structure
  if (!fillObsVariables(obs_vars)){
    IDLOGERROR("html", "kvQABaseMeteodata::data_asPerl ERROR"
	       << " could not fill var-structures for observations."
	       << std::endl);
    return false;
  }

  if (!fillObsVariables(refobs_vars)){
    IDLOGERROR("html", "kvQABaseMeteodata::data_asPerl ERROR"
	       << " could not fill var-structures for reference observations."
	       << std::endl);
    return false;
  }

  if (!fillModelVariables(model_vars)){
    IDLOGERROR("html", "kvQABaseMeteodata::data_asPerl ERROR"
	       << " could not fill var-structures for model."
	       << std::endl);
    return false;
  }


//   IDLOGDEBUG("html","---------- Dump of obs_vars" << std::endl);
//   if (obs_vars.allpos.size() > 0){
//     IDLOGDEBUG("html","Source:" << obs_vars.source << std::endl);
//     IDLOGDEBUG("html","Timestart:" << obs_vars.timestart << std::endl);
//     IDLOGDEBUG("html","Timestop:" << obs_vars.timestop << std::endl);
//     for (int k=0; k<obs_vars.allpos.size(); k++)
//       IDLOGDEBUG("html","  Station:" << obs_vars.allpos[k] << std::endl);
//     for (int k=0; k<obs_vars.alltimes.size(); k++)
//       IDLOGDEBUG("html","  Timeoffset:" << obs_vars.alltimes[k] << std::endl);
//     IDLOGDEBUG("html","Variables" << std::endl);
//     for (int k=0; k<obs_vars.pars.size(); k++){
//       IDLOGDEBUG("html","-------------------------------" << std::endl);
//       IDLOGDEBUG("html","  - var-signature:" << obs_vars.pars[k].signature << std::endl);
//       IDLOGDEBUG("html","  - var-name:" << obs_vars.pars[k].name << std::endl);
//       IDLOGDEBUG("html","  - var-paramid:" << obs_vars.pars[k].paramid << std::endl);
//       IDLOGDEBUG("html","  - #var-values:" << obs_vars.pars[k].values.size() << std::endl);
//     }
//   }
//   IDLOGDEBUG("html","----------------------------------" << std::endl);
  
//   IDLOGDEBUG("html","---------- Dump of refobs_vars" << std::endl);
//   if (refobs_vars.allpos.size() > 0){
//     IDLOGDEBUG("html","Source:" << refobs_vars.source << std::endl);
//     IDLOGDEBUG("html","Timestart:" << refobs_vars.timestart << std::endl);
//     IDLOGDEBUG("html","Timestop:" << refobs_vars.timestop << std::endl);
//     for (int k=0; k<refobs_vars.allpos.size(); k++)
//       IDLOGDEBUG("html","  Station:" << refobs_vars.allpos[k] << std::endl);
//     for (int k=0; k<refobs_vars.alltimes.size(); k++)
//       IDLOGDEBUG("html","  Timeoffset:" << refobs_vars.alltimes[k] << std::endl);
//     IDLOGDEBUG("html","Variables" << std::endl);
//     for (int k=0; k<refobs_vars.pars.size(); k++){
//       IDLOGDEBUG("html","-------------------------------" << std::endl);
//       IDLOGDEBUG("html","  - var-signature:" << refobs_vars.pars[k].signature << std::endl);
//       IDLOGDEBUG("html","  - var-name:" << refobs_vars.pars[k].name << std::endl);
//       IDLOGDEBUG("html","  - var-paramid:" << refobs_vars.pars[k].paramid << std::endl);
//       IDLOGDEBUG("html","  - #var-values:" << refobs_vars.pars[k].values.size() << std::endl);
//     }
//   }
//   IDLOGDEBUG("html","----------------------------------" << std::endl);
  
//   IDLOGDEBUG("html","---------- Dump of model_vars" << std::endl);
//   if (model_vars.allpos.size() > 0){
//     IDLOGDEBUG("html","Source:" << model_vars.source << std::endl);
//     IDLOGDEBUG("html","Timestart:" << model_vars.timestart << std::endl);
//     IDLOGDEBUG("html","Timestop:" << model_vars.timestop << std::endl);
//     for (int k=0; k<model_vars.allpos.size(); k++)
//       IDLOGDEBUG("html","  Station:" << model_vars.allpos[k] << std::endl);
//     for (int k=0; k<model_vars.alltimes.size(); k++)
//       IDLOGDEBUG("html","  Timeoffset:" << model_vars.alltimes[k] << std::endl);
//     IDLOGDEBUG("html","Variables" << std::endl);
//     for (int k=0; k<model_vars.pars.size(); k++){
//       IDLOGDEBUG("html","-------------------------------" << std::endl);
//       IDLOGDEBUG("html","  - var-signature:" << model_vars.pars[k].signature << std::endl);
//       IDLOGDEBUG("html","  - var-name:" << model_vars.pars[k].name << std::endl);
//       IDLOGDEBUG("html","  - var-paramid:" << model_vars.pars[k].paramid << std::endl);
//       IDLOGDEBUG("html","  - #var-values:" << model_vars.pars[k].values.size() << std::endl);
//     }
//   }
//   IDLOGDEBUG("html","----------------------------------" << std::endl);



  std::string obs_varstring;
  if (!sman.makePerlVariables(obs_vars, obs_varstring)){
    return false;
  }
  std::string refobs_varstring;
  if (!sman.makePerlVariables(refobs_vars, refobs_varstring)){
    return false;
  }
  std::string model_varstring;
  if (!sman.makePerlVariables(model_vars, model_varstring)){
    return false;
  }
  
  // add observation-time as variabel
  miTime obstime= stationinfo_.obstime();
  data+= "my @obstime=(" + miString(obstime.year())  + "," +
    miString(obstime.month()) + "," +
    miString(obstime.day())   + "," +
    miString(obstime.hour())  + "," +
    miString(obstime.min())   + "," +
    miString(obstime.sec())   + ");\n";
  
  data+= "\n";

  // add meteorological data strings
  data+= obs_varstring + "\n";
  data+= refobs_varstring + "\n";
  data+= model_varstring + "\n";
  
  data+="#\n";
  data+="#==========================================\n";
  data+="\n";

  return true;
}


bool
kvQABaseMeteodata::fillObsVariables(kvQABase::script_var& vars)
{
  std::set<int> alltimes; // unique list of timeoffsets

  std::vector<int>::iterator pp= vars.allpos.begin();
  for (; pp!=vars.allpos.end(); pp++){
    IDLOGINFO("html", " -OBS processing for pos:" << *pp << std::endl);
    
    // load data
    if (!loadObsData(*pp,vars.timestart,vars.timestop)){
      return false;
    }

    // if still no data...give up
    if (obsdata[*pp].size() == 0){
      IDLOGERROR("html", "no OBSERVATIONS found for station:"  << *pp
		 << " and timesteps:" << vars.timestart
		 << "," << vars.timestop
		 << std::endl);
      return false;
    }

    // fetch text data
    if (!loadTextData(*pp,vars.timestart,vars.timestop)){
      return false;
    }
    
    // loop observation times
    std::map<miTime, kvQABaseDBConnection::obs_data>::iterator tp;
    for (tp= obsdata[*pp].begin(); tp!=obsdata[*pp].end(); tp++){
      int mindiff= miTime::minDiff(tp->first,stationinfo_.obstime());
      
      if (mindiff > vars.timestart)
	continue;
      if (mindiff < vars.timestop)
	continue;

      IDLOGINFO("html", "   -OBS processing for time:"
		<< tp->first << std::endl);
      alltimes.insert(mindiff);

      // loop parameters and search for them in data
      int n= vars.pars.size();
      for (int j=0; j<n; j++){
	int vid= vars.pars[j].paramid;
	if (vid > 1000)// skip text_data!
	  continue;
	int sid= vars.pars[j].sensor;
	int lid= vars.pars[j].level;
	int tid= vars.pars[j].typeID;
	std::string vname= vars.pars[j].name;
	std::string value= kvQABase::missing_value;
	int status= kvQCFlagTypes::status_orig_and_corr_missing;
	kvalobs::kvControlInfo cinfo;

	IDLOGINFO("html", "     -OBS processing for par:" << vname
		  << " nr:" << vid << std::endl);

	// find parameter in data-structs
	int i,np= tp->second.data.size();
	for (i=0; i<np; i++){
	  /* check for:
	     - correct parameter
	     - specific sensor OR first available
	     - specific level  OR first available
	     - specific typeID OR the original stationinfo.typeID()!!
	       ( -typeID == typeID )
	  */
	  if ( tp->second.data[i].paramID() == vid                   &&
	       (sid == -32767 || tp->second.data[i].sensor() == sid) &&
	       (lid == -32767 || tp->second.data[i].level()  == lid) &&
	       ((tid == -32767 && abs(tp->second.data[i].typeID())
		 == abs(stationinfo_.typeID())) ||
		abs(tp->second.data[i].typeID()) == abs(tid)) ){
 	    std::ostringstream ost;
	    // the observation is the corrected value
	    ost << tp->second.data[i].corrected();
	    value= ost.str();
	    // fetch controlinfo
	    cinfo= tp->second.data[i].controlinfo();
	    // fetch status from controlinfo
	    status= cinfo.MissingFlag();
	    // ensure missing_data flag sanity
	    vars.missing_data |= (status > 0);
	    // set correct sensor, level and typeID in vars
	    vars.pars[j].sensor = tp->second.data[i].sensor();
	    vars.pars[j].level  = tp->second.data[i].level();
	    vars.pars[j].typeID = tp->second.data[i].typeID();
	    break;
	  }
	}
	if (i == np){
	  IDLOGINFO("html", "Missing OBSERVATION parameter:" << vname
		    << " for station:"  << *pp
		    << " and timestep:" << mindiff
		    << std::endl);
	  vars.missing_data= true;
	}

	// add data value
	vars.pars[j].values[*pp][mindiff].value=  value;
	vars.pars[j].values[*pp][mindiff].status= status;
	vars.pars[j].values[*pp][mindiff].cinfo=  cinfo;
      }
    }

    // ============================================================================

    // loop text_data observation times
    std::map<miTime, kvQABaseDBConnection::text_data>::iterator ttp;
    for (ttp= textdata[*pp].begin(); ttp!=textdata[*pp].end(); ttp++){
      int mindiff= miTime::minDiff(ttp->first,stationinfo_.obstime());
      
      if (mindiff > vars.timestart)
	continue;
      if (mindiff < vars.timestop)
	continue;

      IDLOGINFO("html", "   -OBS processing for time:"
		<< ttp->first << std::endl);
      alltimes.insert(mindiff);

      // loop parameters and search for them in data
      int n= vars.pars.size();
      for (int j=0; j<n; j++){
	int vid= vars.pars[j].paramid;
	if (vid <= 1000)// skip normal observation data!
	  continue;
	int tid= vars.pars[j].typeID;
	std::string vname= vars.pars[j].name;
	std::string value= kvQABase::missing_value;
	int status= kvQCFlagTypes::status_orig_and_corr_missing;
	kvalobs::kvControlInfo cinfo;

	IDLOGINFO("html", "     -OBS processing for par:" << vname
		  << " nr:" << vid << std::endl);
	
	// find parameter in data-structs
	int i,np= ttp->second.data.size();
	for (i=0; i<np; i++){
	  /* check for:
	     - correct parameter
	     - specific typeID OR the original stationinfo.typeID()!!
	       ( -typeID == typeID )
	  */
	  if ( ttp->second.data[i].paramID() == vid &&
	       ((tid == -32767 && abs(ttp->second.data[i].typeID())
		 == abs(stationinfo_.typeID())) ||
		abs(ttp->second.data[i].typeID()) == abs(tid)) ){
	    // the observation here is the original value
	    value= ttp->second.data[i].original();
	    // set status= OK
 	    status= kvQCFlagTypes::status_ok;
	    // ensure missing_data flag sanity
	    vars.missing_data |= (status > 0);
 	    // set correct typeID in vars
 	    vars.pars[j].typeID = ttp->second.data[i].typeID();
	    break;
	  }
	}
	if (i == np){
	  IDLOGINFO("html", "Missing OBSERVATION parameter:" << vname
		    << " for station:"  << *pp
		    << " and timestep:" << mindiff
		    << std::endl);
	  vars.missing_data= true;
	}

	// add data value
	vars.pars[j].values[*pp][mindiff].value=  value;
	vars.pars[j].values[*pp][mindiff].status= status;
	vars.pars[j].values[*pp][mindiff].cinfo=  cinfo;
      }
    }
    


    // ============================================================================

    /*
      Keep this var_station code a while longer!!!
    */

//     if (!vars.allnormal){
//       // loop var_station times
//       std::map<miTime, kvQABaseDBConnection::var_data>::iterator vp;
//       for (vp= vardata[*pp].begin(); vp!=vardata[*pp].end(); vp++){
// 	int mindiff= miTime::minDiff(vp->first,obstime_);
      
// 	if (mindiff > vars.timestart)
// 	  continue;
// 	if (mindiff < vars.timestop)
// 	  continue;

// 	IDLOGINFO("html", "   -VAR processing for time:"
// 		  << vp->first << std::endl);
// 	alltimes.insert(mindiff);

// 	// loop parameters
// 	int n= vars.pars.size();
// 	for (int j=0; j<n; j++){
// 	  if (vars.pars[j].normal)
// 	    continue; // skip normal data
// 	  std::string vname= vars.pars[j].name;
// 	  std::string value= kvQABase::missing_value;
// 	  int status= kvQCFlagTypes::status_ok;
// 	  IDLOGINFO("html", "     -VAR processing for par:" << vname
// 		    << std::endl);

// 	  // find parameter in data-struct
// 	  int np= vp->second.data.size();
// 	  if (np > 0){
// 	    std::ostringstream ost;
// 	    if        (vname == "var:lat"){
// 	      ost << vp->second.data[0].lat();
// 	    } else if (vname == "var:lon") {
// 	      ost << vp->second.data[0].lon();
// 	    } else if (vname == "var:direction") {
// 	      ost << vp->second.data[0].direction();
// 	    } else if (vname == "var:speed"){
// 	      ost << vp->second.data[0].speed();
// 	    } else {
// 	      IDLOGERROR("html", "Unknown VAR parameter:" << vname
// 			 << " for station:"  << *pp
// 			 << " and timestep:" << mindiff
// 			 << std::endl);
// 	      ost << kvQABase::missing_value;
// 	      status= kvQCFlagTypes::status_orig_and_corr_missing;
// 	    }
// 	    value= ost.str();
// 	  } else {
// 	    status= kvQCFlagTypes::status_orig_and_corr_missing;
// 	  }
// 	  if (status > 0){
// 	    IDLOGERROR("html", "Missing VAR parameter:" << vname
// 		       << " for station:"  << *pp
// 		       << " and timestep:" << mindiff
// 		       << std::endl);
// 	    vars.missing_data = true;
// 	  }

// 	  // add data value
// 	  vars.pars[j].values[*pp][mindiff].value=  value;
// 	  vars.pars[j].values[*pp][mindiff].status= status;
// 	}
//       }
//     }

  } // position-loop
  
  // update complete set of timeoffsets (in reverse order)
  vars.alltimes.clear();
  std::set<int>::reverse_iterator rt= alltimes.rbegin();
  for (; rt!=alltimes.rend(); rt++)
    vars.alltimes.push_back(*rt); // add timestep to list

  return true;
}



bool
kvQABaseMeteodata::fillModelVariables(kvQABase::script_var& vars)
{
  std::set<int> alltimes; // unique list of timeoffsets

  std::vector<int>::iterator pp= vars.allpos.begin();
  for (; pp!=vars.allpos.end(); pp++){
    IDLOGINFO("html", " - MODEL processing for pos:" << *pp << std::endl);
    
    // load data
    if (!loadModelData(*pp,vars.timestart,vars.timestop)){
      return false;
    }

    // if still no data...give up
    if (modeldata[*pp].size() == 0){
      IDLOGERROR("html", "no MODELDATA found for station:"  << *pp
		 << " and timesteps:" << vars.timestart << "," << vars.timestop
		 << std::endl);
      return false;
    }

    // loop times
    std::map<miTime, kvQABaseDBConnection::model_data>::iterator tp;
    for (tp= modeldata[*pp].begin(); tp!=modeldata[*pp].end(); tp++){
      IDLOGINFO("html", "   -MODEL processing for time:" << tp->first << std::endl);
      int mindiff= miTime::minDiff(tp->first,stationinfo_.obstime());
      
      if (mindiff > vars.timestart)
	continue;
      if (mindiff < vars.timestop)
	continue;

      alltimes.insert(mindiff);

      // loop parameters
      int n= vars.pars.size();
      for (int j=0; j<n; j++){
	int vid= vars.pars[j].paramid;
	std::string vname= vars.pars[j].name;
	std::string value= kvQABase::missing_value;
	int status= kvQCFlagTypes::status_ok;
	IDLOGINFO("html", "     -MODEL processing for par:" << vname
		  << " nr:" << vid << std::endl);

	// find parameter in data-structs
	int i,np= tp->second.data.size();
	for (i=0; i<np; i++){
	  if (tp->second.data[i].paramID() == vid){
	    std::ostringstream ost;
	    ost << tp->second.data[i].original();
	    value= ost.str();
	    status= kvQCFlagTypes::status_ok;
	    break;
	  }
	}
	if (i == np){
	  IDLOGERROR("html", "Missing MODEL parameter:" << vname
		     << " for station:" << *pp
		     << " and timestep:" << mindiff
		     << std::endl);
	  status= kvQCFlagTypes::status_original_missing;
	  vars.missing_data= true;
	  // should we really........
	  return false;
	}
	
	// add data value
	vars.pars[j].values[*pp][mindiff].value=  value;
	vars.pars[j].values[*pp][mindiff].status= status;
      }
    }
  }
  
  // update complete set of timeoffsets (in reverse order)
  vars.alltimes.clear();
  std::set<int>::reverse_iterator rt= alltimes.rbegin();
  for (; rt!=alltimes.rend(); rt++)
    vars.alltimes.push_back(*rt); // add timestep to list

  return true;
}




/*
  Update parameters with return-variables from check.
  Update stationinfo
  Finally save parameters to DB 
*/
bool
kvQABaseMeteodata::updateParameters(std::map<std::string, double>& params)
{
  /*
    loop through the map of return-values from check
    - recognize parameter
    - update flags, corrected, missing
  */

  std::map<std::string,obs_keys> updated_obs;

  const miString type_flag      = "flag";
  const miString type_corrected = "corrected";
  const miString type_missing   = "missing";
  const miString type_subcheck  = "subcheck";
  
  //std::string subcheck;

  std::map<std::string, double>::iterator pp= params.begin();

/*  
 * Removed by Vegard B�nes:
 * subcheck should be of the same form as the other params, only being shown 
 * on its relevant parameters/station.
 * 
  // first check for subcheck value
  for (; pp!=params.end(); pp++){
    std::string key= pp->first;
    double value= pp->second;
    if ( key == type_subcheck ){
      std::ostringstream ost;
      ost << "." << value;
      subcheck = ost.str();
      break;
    }
  }
*/

  // then check all other keys..
  pp = params.begin();
  for (; pp!=params.end(); pp++){
    const std::string key= pp->first;
    double value= pp->second;

    // skip subcheck keys here..
    if ( key == type_subcheck )
      continue;
    
    IDLOGINFO("html","Checking key:" << key << " with value:"
	      << value << std::endl);
    
    std::string par;  // parameter name
    int tidx=-1;      // time index
    int sidx=-1;      // station index
    std::string type; // data type

    std::vector<miString> vs= miString(key).split("_");
    if (vs.size() < 4){
      IDLOGERROR("html",
    		 "kvQABaseMeteodata::updateParameters. Bad parametername from script:"
    		 << key
    		 << " Expected <name>_<timeindex>_<stationindex>_<datatype>"
    		 << std::endl);
      continue;
    }
    int n= vs.size() - 1;

    type= vs[n].downcase();                               // datatype
    if (vs[n-1].isNumber()) sidx= atoi(vs[n-1].c_str());  // station index
    if (vs[n-2].isNumber()) tidx= atoi(vs[n-2].c_str());  // time index

    for (int i=0; i<n-2; i++) // rebuild parameter name...
      par+=((i ? "_":"") + vs[i]);

    std::string tmp_key= par + vs[n-2] + vs[n-1];
    
    IDLOGDEBUG("html"," par:" << par << " tidx:" << tidx << " sidx:" << sidx
	       << " type:" << type << std::endl);

    // next, identify correct obs-parameter
    if (type != type_flag && type != type_corrected && type != type_missing){
      IDLOGERROR("html", "kvQABaseMeteodata::updateParameters. unknown type of data:"
		 << type
		 << std::endl);
      continue;
    }
    if (tidx < 0 || tidx >= obs_vars.alltimes.size()){
      IDLOGERROR("html",
		 "kvQABaseMeteodata::updateParameters. timeindex from script out of bounds:"
		 << tidx
		 << std::endl);
      continue;
    }
    if (sidx < 0 || sidx >= obs_vars.allpos.size()){
      IDLOGERROR("html",
		 "kvQABaseMeteodata::updateParameters. stationindex from script out of bounds:"
		 << sidx
		 << std::endl);
      continue;
    }
    int ipar, npar= obs_vars.pars.size();
    for (ipar=0; ipar<npar; ipar++){
      if (obs_vars.pars[ipar].signature == par){
        break;
      }
    }
    if (ipar == npar){
      IDLOGERROR("html", "kvQABaseMeteodata::updateParameters. parameter not found:"
		 << par
		 << std::endl);
      continue;
    }
    // found station, time and parameter
    int timeoff = obs_vars.alltimes[tidx];
    int station = obs_vars.allpos[sidx];
    int paramid = obs_vars.pars[ipar].paramid;
    int sensor  = obs_vars.pars[ipar].sensor;
    int level   = obs_vars.pars[ipar].level;
    int typeID  = obs_vars.pars[ipar].typeID;

    // calculate parameter-time 
    miTime time= stationinfo_.obstime();
    time.addMin(timeoff);
    IDLOGDEBUG("html","LOOK FOR station:" << station << " timeoff:" << timeoff
	       << " paramid:"<<  paramid
	       << " sensor:" << (sensor != -32767 ? miString(sensor) : "ANY")
	       << " level:"   << (level  != -32767 ? miString(level)  : "ANY")
	       << " typeID:" << (typeID != -32767 ? miString(typeID) : "ANY")
	       << " time:"   <<  time << std::endl);
    
    // check if we really have loaded obsdata for these..
    if (obsdata.count(station) == 0){
      IDLOGERROR("html",
		 "kvQABaseMeteodata::updateParameters. obsdata not found for station:"
		 << station
		 << std::endl);
      continue;
    }
    if (obsdata[station].count(time) == 0){
      IDLOGERROR("html",
		 "kvQABaseMeteodata::updateParameters. obsdata not found for time:"
		 << time
		 << std::endl);
      continue;
    }
    // find correct parameter and typeID
    int np= obsdata[station][time].data.size();
    for (ipar=0; ipar<np; ipar++){
      if ( paramid == obsdata[station][time].data[ipar].paramID() &&
            sensor  == obsdata[station][time].data[ipar].sensor()  &&
            level   == obsdata[station][time].data[ipar].level()   &&
            typeID  == obsdata[station][time].data[ipar].typeID() ){
      	break;
      }
    }
    if (ipar == np){
      IDLOGERROR("html",
    		 "kvQABaseMeteodata::updateParameters. obsdata not found for paramid:"
    		 << paramid
    		 << " sensor:" <<  (sensor != -32767 ? miString(sensor) : "ANY")
    		 << " level:"   << (level  != -32767 ? miString(level)  : "ANY")
    		 << " typeID:" <<  (typeID != -32767 ? miString(typeID) : "ANY")
    		 << std::endl);
      continue;
    }
    // at last - the correct kvData!
    kvalobs::kvData newdata     = obsdata[station][time].data[ipar];
    kvalobs::kvControlInfo cinfo= newdata.controlinfo();
    kvalobs::kvUseInfo     uinfo= newdata.useinfo();

    // store keys for later
    if (updated_obs.count(tmp_key) == 0){
      obs_keys obsk;
      obsk.stationID = station;
      obsk.time      = time;
      obsk.typeID    = typeID;
      obsk.paridx    = ipar;
      updated_obs[tmp_key]= obsk;
    }
  
    if (type == type_flag){
      // update control-flags
      int flagvalue= static_cast<int>(value);
      IDLOGINFO("html","====================================================="
    		<< std::endl);
      IDLOGINFO("html", "kvQABaseMeteodata::updateParameters controlinfo BEFORE:"
		    << cinfo << std::endl);
      // ensure that new flagvalue is larger than old
      int oldflagvalue;
      cinfo.getControlFlag(medium_qcx_, oldflagvalue);
      if (oldflagvalue < flagvalue)
        cinfo.setControlFlag(medium_qcx_, flagvalue);
      IDLOGINFO("html", "kvQABaseMeteodata::updateParameters controlinfo  AFTER:"
    		<< cinfo << std::endl);
      IDLOGINFO("html","====================================================="
		    << std::endl);
      newdata.controlinfo(cinfo);

      // update cfailed if control-flag != 1
      if (flagvalue != 1){
      	miString cfailed= newdata.cfailed();
      	if (cfailed.length() > 0) cfailed+= ","; 
        
        // Added by Vegard B�nes:
        // Check for subcheck variable for param, station, time.
        // if we are to set cfailed, this will be added to the string.
        std::string subcheck = "";
        std::string::size_type pos = key.rfind( '_' );
        std::string firstPart = key.substr( 0, pos +1 );
        std::string subcheckKey = firstPart + type_subcheck;
        IDLOGINFO( "html", "Setting cfailed - checking for subcheck, key(" << subcheckKey << ")" << std::endl );
        std::map<std::string, double>::const_iterator find = params.find( subcheckKey );
        if ( find != params.end() ) {
          std::ostringstream ost;
          ost << "." << (int) find->second;
          IDLOGINFO( "html", "Found key!" << std::endl );
          subcheck = ost.str();
        }
        
      	cfailed+= qcx_ + subcheck + ":" + miString(language_);
        IDLOGINFO( "html", "cfailed = " << cfailed << std::endl);
      	newdata.cfailed(cfailed);
      }

      // update useinfo
      IDLOGINFO("html","====================================================="
    		<< std::endl);
      IDLOGINFO("html", "kvQABaseMeteodata::updateParameters     useinfo BEFORE:"
		    << uinfo << std::endl);
      uinfo.setUseFlags(cinfo);
      ////////uinfo.Confidence(100); //OBS OBS - setting confidence!!!!!
      ////////uinfo.addToErrorCount();
	
      IDLOGINFO("html", "kvQABaseMeteodata::updateParameters     useinfo  AFTER:"
    		<< uinfo << std::endl);
      IDLOGINFO("html","====================================================="
		    << std::endl);
      newdata.useinfo(uinfo);

    } else if (type == type_missing){
      // update missing flag
      int flagvalue= static_cast<int>(value);
      IDLOGINFO("html","====================================================="
    		<< std::endl);
      IDLOGINFO("html", "kvQABaseMeteodata::updateParameters controlinfo BEFORE:"
		    << cinfo << std::endl);
      cinfo.setControlFlag(kvQCFlagTypes::f_fmis, flagvalue);
      IDLOGINFO("html", "kvQABaseMeteodata::updateParameters controlinfo  AFTER:"
    		<< cinfo << std::endl);
      IDLOGINFO("html","====================================================="
		    << std::endl);
      newdata.controlinfo(cinfo);

      // update useinfo
      IDLOGINFO("html","====================================================="
    		<< std::endl);
      IDLOGINFO("html", "kvQABaseMeteodata::updateParameters     useinfo BEFORE:"
		    << uinfo << std::endl);
      uinfo.setUseFlags(cinfo);
      ////////uinfo.Confidence(100); //OBS OBS - setting confidence!!!!!
      ////////uinfo.addToErrorCount();
	
      IDLOGINFO("html", "kvQABaseMeteodata::updateParameters     useinfo  AFTER:"
    		<< uinfo << std::endl);
      IDLOGINFO("html","====================================================="
		    << std::endl);
      newdata.useinfo(uinfo);

    } else if (type == type_corrected){
      // update data
      newdata.corrected(value);
      // any flags?
    }

    // replace parameter with the new one
    obsdata[station][time].data[ipar] = newdata;
  }

  // run through updated observations again and save to db
  if (updated_obs.size() > 0){
    std::map<std::string,obs_keys>::const_iterator itr= updated_obs.begin();
    for (; itr!=updated_obs.end(); itr++){
      kvalobs::kvData newdata = obsdata[itr->second.stationID][itr->second.time].data[itr->second.paridx];

      // check for observation that has been rejected!
      kvalobs::kvControlInfo cinfo= newdata.controlinfo();
      if ( cinfo.iznogood(medium_qcx_) ){
      	// this observation is rejected ... set corrected to -32766
      	newdata.corrected(-32766);
      }

      // ..save it to db
      if (!dbcon_.setObservation(newdata)){
      	IDLOGERROR("html", "kvQABaseMeteodata::updateParameter ERROR"
		       << " could not save parameter to DB."
    		   << std::endl);
	      continue;
      }
    }
  }
  
  return true;
}

