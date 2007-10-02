/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvQABaseScriptManager.cc,v 1.19.6.2 2007/09/27 09:02:38 paule Exp $                                                       

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
#include "../include/kvQABaseScriptManager.h"
#include <puTools/miString>
#include <sstream>

#include <milog/milog.h>

kvQABaseScriptManager::kvQABaseScriptManager(kvQABaseDBConnection& dbcon)
  : dbcon_(dbcon), algo_selected(false)
{
}


kvQABaseScriptManager::~kvQABaseScriptManager()
{
  clear();
}


bool kvQABaseScriptManager::findAlgo(const std::string name,
				     const std::string argu,
				     const int sid, bool& sig_ok)
{
  sig_ok= false;
  clear(); // clear var-structures

  miutil::miString algoname = name;
  miutil::miString argument = argu;
  miutil::miString signature;
  algoname.trim();
  int nname= algoname.length();
  int nargu= argument.length();

  if (nname==0){
    IDLOGERROR("html", "kvQABaseScriptManager::findAlgo ERROR"
	       << " name string empty"
	       << std::endl);
    return false;
  }
  
  IDLOGINFO("html", "Algoname:" << algoname << std::endl);
  IDLOGINFO("html", "Argument:" << argument << std::endl);

  if (!dbcon_.getAlgo(algoname, algo)){
    IDLOGERROR("html", "kvQABaseScriptManager::findAlgo ERROR"
	       << " Algorithm not found in DB:"
	       << algoname
	       << ":" << std::endl);
    return false;
  }

  signature= algo.signature();
  IDLOGINFO("html", "Found Signature:" << signature << std::endl);
  
  // compare signature against argument - and make script variables
  std::vector<miutil::miString> sig_p1, sig_p2, sig_var, sig_pos, sig_time;
  std::vector<miutil::miString> arg_p1, arg_p2, arg_var, arg_pos, arg_time;
  int source;        // source-type..
  std::string sname; // and sourcename

  sig_p1= signature.split('|'); // split on sources
  arg_p1= argument.split('|');
  if (sig_p1.size() != arg_p1.size()) {
    IDLOGERROR("html", "kvQABaseScriptManager::findAlgo ERROR"
	       << " Signature and Argument has different number of sources:"
	       << signature << " and " << argument
	       << std::endl);
    return true;
  }
  int m= sig_p1.size();

  // loop on sources
  for (int i=0; i<m; i++){
    //IDLOGINFO("html", "Source:" << i << std::endl);
    // due to miString::split quirks we add a ';'
    // - should not be a problem
    sig_p1[i]+= ";";
    arg_p1[i]+= ";";
    sig_p2= sig_p1[i].split(';',false); // split on parts
    arg_p2= arg_p1[i].split(';',false);
    if (sig_p2.size() != arg_p2.size()){
      IDLOGERROR("html", "kvQABaseScriptManager::findAlgo ERROR"
		 << " Signature and Argument do not match:"
		 << sig_p1[i] << " and " << arg_p1[i]
		 << std::endl);
      return true;
    }
    int numparts= sig_p2.size();
    if (numparts<4){
      IDLOGERROR("html", "kvQABaseScriptManager::findAlgo ERROR"
		 << " Illegal script-signature:"
		 << sig_p1[i]
		 << std::endl);
      return true; // at least source;variables
    }

    // first check source
    source=-1;
    sname= sig_p2[0];
    if (sname == "obs")         source= kvQABase::obs_data;
    else if (sname == "refobs") source= kvQABase::refobs_data;
    else if (sname == "model")  source= kvQABase::model_data;
    else if (sname == "meta")   source= kvQABase::meta_data;
    if (source < 0) {
      IDLOGERROR("html", "kvQABaseScriptManager::findAlgo ERROR"
		 << " unknown source-type:"
		 << sname
		 << std::endl);
      return true;
    }
    if (sname != arg_p2[0]){
      IDLOGERROR("html", "kvQABaseScriptManager::findAlgo ERROR"
		 << " source-types differ. Signature:"
		 << sname << " and Argument:" << arg_p2[0]
		 << std::endl);
      return true;
    }
    
    // then check parameters
    sig_var=  sig_p2[1].split(','); // split on sub-parts
    arg_var=  arg_p2[1].split(',');
    // equal number of parameters?
    if (sig_var.size() != arg_var.size()){
      IDLOGERROR("html", "kvQABaseScriptManager::findAlgo ERROR"
		 << " number of variables. Signature:"
		 << sig_var.size() << " and Argument:" << arg_var.size()
		 << std::endl);
      return true;
    }

    // then check positions
    int npos=1;
    std::vector<int> positions;
    positions.push_back(sid);
    if (arg_p2[2].exists()){ // any extra positions
      //sig_pos=  sig_p2[2].split(','); // split on sub-parts
      arg_pos=  arg_p2[2].split(',');
      for (int j=0; j<arg_pos.size(); j++){
	if (!arg_pos[j].isNumber()){
	  IDLOGWARN("html", "kvQABaseScriptManager::findAlgo WARNING"
		    << " position-argument not number:"
		    << arg_pos[j]
		    << std::endl);
	  return true; //continue;
	}
	positions.push_back(atoi(arg_pos[j].c_str()));
	npos++;
      }
    }
    // then check times
    int tstart=0, tstop=0;
    if (arg_p2[3].exists()){ // time-interval exists
      //sig_time=  sig_p2[3].split(','); // split on sub-parts
      arg_time=  arg_p2[3].split(',');
      if (arg_time.size()==2){
	if (!arg_time[0].isNumber() ||
	    !arg_time[1].isNumber()){
	  IDLOGWARN("html", "kvQABaseScriptManager::findAlgo WARNING"
		    << " time-arguments not numbers:"
		    << arg_time[0] << " and " << arg_time[1]
		    << std::endl);
	  return true;
	}
	tstart= atoi(arg_time[0].c_str());
	tstop=  atoi(arg_time[1].c_str());
	if (tstart < tstop){
	  IDLOGWARN("html", "kvQABaseScriptManager::findAlgo WARNING"
		    << " time-arguments in wrong order. Starttime:"
		    << arg_time[0] << " is less than Stoptime:" << arg_time[1]
		    << std::endl);
	  return true;
	}
      }
    }
    // add to variables lists
    variables[source].dsource= source;
    variables[source].source= sname;
    variables[source].timestart=tstart;
    variables[source].timestop=tstop;
    variables[source].missing_data= false;
    variables[source].allpos= positions;
    variables[source].allnormal= (source!= kvQABase::meta_data);

    kvQABase::script_par par;
    for (int j=0; j<sig_var.size(); j++){
      par.paramid =  0;
      par.sensor  =  0;
      par.level   = -32767;
      par.typeID  = -32767;
      par.signature= sig_var[j];
      par.normal=    (source!= kvQABase::meta_data);

      miutil::miString parameter= arg_var[j];
      par.name= parameter;
      if (parameter.length() < 1){
	IDLOGERROR("html", "kvQABaseScriptManager::findAlgo ERROR"
		   << " missing parameter-name:"
		   << arg_p2[1]
		   << std::endl);
	return true;
      }
//       // check if requested parameter is an abnormal parameter
//       // even if it is listed in obs_, refobs_ or model_data (ex: var_station)
//       if (par.normal && parameter.contains(":")){
// 	IDLOGINFO("html", "kvQABaseScriptManager::findAlgo "
// 		  << " This is not a normal parameter:"
// 		  << parameter
// 		  << std::endl);
// 	par.normal= false;
//       }

      // parameter possibly contains level, sensor and typeID
      // (name&level&sensor&typeID)
      std::vector<miutil::miString> parvt= parameter.split("&",false);
      miutil::miString parametername = parvt[0];

      if (parvt.size() > 1){
	// level
	if (parvt[1].exists()){
	  if (!parvt[1].isNumber()){
	    IDLOGERROR("html", "kvQABaseScriptManager::findAlgo ERROR."
		       << " LEVEL-argument not a number:"
		       << parvt[1] << std::endl);
	    return true;
	  }
	  par.level= atoi(parvt[1].c_str());
	}
	if (parvt.size() > 2){
	  // sensor
	  if (parvt[2].exists()){
	    if (!parvt[2].isNumber()){
	      IDLOGERROR("html", "kvQABaseScriptManager::findAlgo ERROR."
			 << " SENSOR-argument not a number:"
			 << parvt[2] << std::endl);
	      return true;
	    }
	    par.sensor= atoi(parvt[2].c_str());
	    if (par.sensor > 9 || par.sensor < 0){
	      IDLOGERROR("html", "kvQABaseScriptManager::findAlgo ERROR."
			 << " SENSOR-argument is not between 0 and 9:"
			 << parvt[2] << std::endl);
	      return true;
	    }
	  }
	  if (parvt.size() > 3){
	    // typeID
	    if (parvt[3].exists()){
	      if (!parvt[3].isNumber()){
		IDLOGERROR("html", "kvQABaseScriptManager::findAlgo ERROR."
			   << " typeID-argument not a number:"
			   << parvt[3] << std::endl);
		return true;
	      }
	      par.typeID= atoi(parvt[3].c_str());
	    }
	  }
	}
      }
      
      if (par.normal && !dbcon_.getParameterId(parametername, par.paramid)){
	IDLOGERROR("html", "kvQABaseScriptManager::findAlgo ERROR"
		   << " unknown parameter-name:"
		   << parametername
		   << std::endl);
	return true;
      }
      variables[source].pars.push_back(par);
      variables[source].allnormal &= par.normal;
    }
  }

  sig_ok= true;    // signature and argument match
  algo_selected= true;

  return true;
}

bool kvQABaseScriptManager::getScript(std::string& s) const
{
  if (!algo_selected) return false;
  s= algo.script();
  return true;
}

bool kvQABaseScriptManager::getAlgoName(std::string& name) const
{
  if (!algo_selected) return false;
  name= algo.checkname();
  return true;
}

bool kvQABaseScriptManager::getVariables(const kvQABase::data_source source,
					 kvQABase::script_var& vars)
  const
{
  if (!algo_selected) return false;
  if (source < 0 || source >3) return false;
  vars= variables[source];
  return true;
}
 
void kvQABaseScriptManager::clear()
{
  algo_selected= false;

  for (int i=0; i<4; i++)
    variables[i].clear();
}

bool kvQABaseScriptManager::makePerlVariables(kvQABase::script_var& vars,
					      std::string& varstring) const
{
  std::ostringstream ost;
  
  int npos=  vars.allpos.size();
  int ntime= vars.alltimes.size();
  int npar=  vars.pars.size();

  if (npos == 0 || ntime == 0 || npar == 0) {
    varstring = "";
    return true;
  }

  // missing data and data-status only for observations
  bool make_missing = (vars.dsource == kvQABase::obs_data ||
		       vars.dsource == kvQABase::refobs_data);

  // controlinfo only for observations
  bool make_cinfo = (vars.dsource == kvQABase::obs_data ||
		     vars.dsource == kvQABase::refobs_data);

  // make time-variables
  ost << "my $" << vars.source << "_numtimes=" << ntime << ";\n";
  ost << "my @" << vars.source << "_timeoffset=(";
  std::vector<int>::const_iterator tp= vars.alltimes.begin();
  for (int i=0; tp!=vars.alltimes.end(); tp++,i++){
    if (i>0) ost << ",";
    ost << *tp;
  }
  ost << ");\n\n";

  // make position-variables
  ost << "my $" << vars.source << "_numstations=" << npos << ";\n";
  ost << "my @" << vars.source << "_stations=(";
  std::vector<int>::const_iterator pp= vars.allpos.begin();
  for (int i=0; pp!=vars.allpos.end(); pp++,i++){
    if (i>0) ost << ",";
    ost << *pp;
  }
  ost << ");\n\n";

  if (make_missing){
    // make global missing data variable
    ost << "my $" << vars.source << "_missing="
	<< (vars.missing_data ? 1 : 0) << ";\n\n";
  }

  // loop over parameters
  for (int i=0; i<npar; i++){
    bool first= true;
    ost  << "my @" << vars.pars[i].signature << "=(";
    std::ostringstream ost2; // data status stream
    if (make_missing) ost2 << "my @" << vars.pars[i].signature << "_missing=(";
    std::ostringstream ost3; // controlinfo stream
    if (make_cinfo) ost3 << "my @" << vars.pars[i].signature << "_controlinfo=(";
    std::string margin(vars.pars[i].signature.length()+18, ' ');

    // loop over positions
    for (pp=vars.allpos.begin(); pp!=vars.allpos.end(); pp++){
      // loop over times
      for (tp=vars.alltimes.begin(); tp!=vars.alltimes.end(); tp++){
	if (!first)                 ost  << ",";
	if (!first && make_missing) ost2 << ",";
	if (!first && make_cinfo)   ost3 << ",";
	std::string value= kvQABase::missing_value;
	int status= kvQCFlagTypes::status_orig_and_corr_missing;
	kvalobs::kvControlInfo cinfo;
	
	// find data-value
	if (vars.pars[i].values.count(*pp)!=0)
	  if (vars.pars[i].values[*pp].count(*tp)!=0){
	    value=  vars.pars[i].values[*pp][*tp].value;
	    status= vars.pars[i].values[*pp][*tp].status;
	    cinfo=  vars.pars[i].values[*pp][*tp].cinfo;
	  }
	ost  << value;

	if (make_missing){
 	  // write data-status
	  ost2 << status;
	}
	if (make_cinfo){
	  // write controlinfo
	  if (!first) ost3 << "\n" << margin;
	  for (int f=0; f<16; f++){
	    if (f != 0) ost3 << ",";
	    ost3 << cinfo.flag(f);
	  }
	}
	first= false;
      }
    }
    ost << ");\n";
    if (make_missing){
      ost2 << ");\n\n";
      ost << ost2.str(); // add data_status
    }
    if (make_cinfo){
      ost3 << ");\n\n";
      ost << ost3.str(); // add controlinfo
    }
  }
  
  varstring = ost.str();
  return true;
}
