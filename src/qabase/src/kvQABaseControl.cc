/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvQABaseControl.cc,v 1.42.2.6 2007/09/27 09:02:38 paule Exp $                                                       

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
#include "../include/kvPerlParser.h"
#undef list

#include <errno.h>
#include "../include/kvQABaseControl.h"

#include "../include/kvQABaseTypes.h"
#include "../include/kvQABaseDBConnection.h"
#include "../include/kvQABaseScriptManager.h"
#include "../include/kvQABaseMetadata.h"
#include "../include/kvQABaseMeteodata.h"

#include <kvalobs/kvChecks.h>
#include <list>

#include <milog/milog.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

using namespace std;
using namespace milog;

kvQABaseControl::kvQABaseControl()
  : logpath_("./log")
{
}

kvQABaseControl::kvQABaseControl(const std::string& logp)
  : logpath_(logp)
{
}

kvQABaseControl::~kvQABaseControl()
{
  kvPerlParser::freePerl();
}


/*
  main routine: runChecks
  do all relevant checks for a station (currently all available checks)
  - input: kvStationInfo with station, observation time and parameters
  - input: database connection

  - run relevant checks for station
  - update kvStationInfo

  return-value != 0 means trouble
*/
int kvQABaseControl::runChecks(kvalobs::kvStationInfo& stationinfo,
			       dnmi::db::Connection *con)
{
  // Skip stations with typeid < 0 (aggregated values) (by Vegard B�nes)
  if ( stationinfo.typeID() < 0 )
    return 0;
  
  HtmlStream *html;
  int stationid = stationinfo.stationID();
  miutil::miTime obstime = stationinfo.obstime();
  
  kvQABaseDBConnection dbcon(con); // the Database connection
  if (!dbcon.dbOk()){
    LOGERROR("kvQABaseControl::runChecks ERROR no database connection" << endl);
    return 1;
  }
  bool result;

  // make sure we are supposed to run checks for this station
  std::list<kvalobs::kvObsPgm> oprogramlist;
  // - if station not in obs_pgm: skip it!
  // ...unless this is an unknown ship, stationid>10000000
  if (stationid <= 10000000){
    result= dbcon.getObsPgm(stationid, obstime, oprogramlist);
    if (oprogramlist.size() == 0){
      return 0;
    }
  }
  
  //Bxrge Moe
  //2004.9.21
  //Have changed how frequent the static tables is updated. 
  //They was updated once for every read. I have changed this
  //to one time every hour. This mean that we may have wrong data
  //for at most one hour. We can remedy this by restarting kvalobs.
  //In the future can we implement a reread by a signal, ex SIGHUP. 
   if(updateStaticTime.undef() || updateStaticTime<miutil::miTime::nowTime()){
     updateStaticTime=miutil::miTime::nowTime();
     updateStaticTime.addHour(1);
     
     dbcon.updateStatics();
   }

  // open new html logger-stream
  html=openHTMLStream(stationid, obstime);

  Logger::createLogger("html", html);
  Logger::logger("html").logLevel(milog::INFO);
  //Logger::logger("html").logLevel(milog::DEBUG);
   
  IDLOGINFO("html", "<h1>"
	    << "kvQABaseControl::runChecks for station:" << stationid
	    << " and obstime:" << obstime
	    << "</h1>" << endl);
  IDLOGINFO("html","<CODE><PRE>");
  
  kvQABaseMeteodata meteod(dbcon,     // Meteorological data manager
			   stationinfo);
  kvQABaseMetadata metad(dbcon);      // Metadata manager
  kvQABaseScriptManager sman(dbcon);  // Perlscript manager
  kvPerlParser parser;                // the perlinterpreter

  // Return if any data has been modified by HQC (ugly and temporary hack) (by Vegard B�nes):
  {
    typedef std::map<miutil::miTime, kvQABaseDBConnection::obs_data> Data;
    Data data_;
    bool res = dbcon.getObservations( stationinfo.stationID(), stationinfo.obstime(), stationinfo.obstime(), data_ );
    if ( ! res ) {
      IDLOGERROR("html","kvQABaseControl::runChecks ERROR on preread of data table" << endl);
      IDLOGINFO("html","</PRE></CODE>");
      Logger::removeLogger("html");
      return 1;
    }
    for ( Data::const_iterator it = data_.begin(); it != data_.end(); ++ it ) {
      typedef std::vector<kvalobs::kvData> KvDL;
      const KvDL & dl = it->second.data;
      for ( KvDL::const_iterator itb = dl.begin(); itb != dl.end(); ++ itb ) {
        if ( itb->typeID() == stationinfo.typeID() and itb->controlinfo().flag( 15 ) ) {
          IDLOGERROR("html","kvQABaseControl::runChecks Will not check HQC corrected data" << endl);
          
          //Børge Moe
          //2005.12.15
          //It seems that useinfo flag is reset when HQC has done its job.
          //Here, as a quick fix, ensure that useinfo(1)=8 when original is missing.
          for( KvDL::const_iterator myitb = dl.begin(); myitb != dl.end(); ++myitb ){
//          	if((myitb->controlinfo().flag(6)==3 || myitb->controlinfo().flag(6)==1) &&
//          		myitb->useinfo().flag(1)!=8){
//          		kvalobs::kvData d(*myitb);
//				kvalobs::kvUseInfo uinfo(d.useinfo());
//					
//				uinfo.set(1, 8);
//          		d.useinfo(uinfo);

			//Update useflags for params where fhqc > 0, ie HQC has been done.
			if(myitb->controlinfo().flag(15)>0){
				kvalobs::kvData d(*myitb);
				kvalobs::kvUseInfo uinfo(d.useinfo());
				
				uinfo.setUseFlags(d.controlinfo());
				d.useinfo(uinfo);
          		
          		IDLOGERROR("html", "kvQABaseControl::runChecks: Updating useinfo paramid: " << 
          							d.paramID() << " sensor: " << d.sensor() <<
          					       " level: " << d.level() << endl <<
          					       "   old useinfo: " << myitb->useinfo() << " new: " << 
          					       d.useinfo() << " controlinfo: " << d.controlinfo() <<
          					       endl
          			  	  );
          		
          		if(!dbcon.setObservation(d)){
          			IDLOGERROR("html", "kvQABaseControl::runChecks: failed to update usinfo!"<<endl);
          		}
          	}
          }
           
          IDLOGINFO("html","</PRE></CODE>");
          Logger::removeLogger("html");          
          return 0;
        }
      }
    }
  }

  // find relevant checks (QC1) for this observation
  std::list<kvalobs::kvChecks> checks;
  result= dbcon.getChecks(stationid, obstime, checks);

  if (!result){
    IDLOGERROR("html","kvQABaseControl::runChecks ERROR. Error reading checks"
	       << endl);
    IDLOGINFO("html","</PRE></CODE>");
    Logger::removeLogger("html");
    return 1;
  }

  if (checks.size() == 0){
    IDLOGINFO("html","kvQABaseControl::runChecks No appropriate checks found"
	      << endl);
    IDLOGINFO("html","</PRE></CODE>");
    Logger::removeLogger("html");
    return 0;
  }
  
  // clear old data
  meteod.clear();
  
  // Loop through checks
  std::list<kvalobs::kvChecks>::const_iterator cp=checks.begin();

  for (; cp!= checks.end(); cp++){
    IDLOGINFO("html", "<HR>" << endl);
    IDLOGINFO("html", "<H2>Check loop, type:" << cp->qcx()
	      << " name:" << cp->checkname() << "</H2>" << endl); 

    string checkstr; // final check string
    string algostr;  // algorithm string
    string metadata; // meta-data string
    string meteodata;// meteorological data string

    /*  Let script-manager find correct perl-script for check
	and validate the signatures  */

    bool sig_ok; // signature ok
    result= sman.findAlgo(cp->checkname(),
			  cp->checksignature(),
			  stationid,sig_ok);
    if (!result){
      IDLOGERROR("html","kvQABaseControl::runChecks failed in ScriptManager.findAlgo"
		 << endl
		 << "  Algorithm not identified!" << endl);
      continue;

    } else if (!sig_ok){
      IDLOGERROR("html","kvQABaseControl::runChecks failed in ScriptManager.findAlgo"
		 << endl
		 << "  BAD signature(s)!" << endl);
      continue;
    }

    result= sman.getScript(algostr);

    if (!result){
      IDLOGERROR("html","kvQABaseControl::runChecks failed in ScriptManager.getScript"
		 << endl);
      continue;
    }
    
    /* fetch meteodata (observations and model-data) for check */

    bool skipcheck= false;
    result= meteod.data_asPerl(cp->qcx(),
			       cp->medium_qcx(),
			       cp->language(),
			       sman,
			       oprogramlist,
			       skipcheck,
			       meteodata);
    if (!result){
      IDLOGERROR("html","kvQABaseControl::runChecks failed in meteod.data_asPerl"
		 << endl);
      continue;
    }

    if (skipcheck){
      IDLOGINFO("html","kvQABaseControl::runChecks skipping check (obs_program)"
		<< endl);
      continue;
    }
    
    /* fetch metadata for check */
    result= metad.data_asPerl(stationid,
			      cp->qcx(),
			      obstime,
			      sman,
			      metadata);
    if (!result){
      IDLOGERROR("html","kvQABaseControl::runChecks failed in metad.data_asPerl"
		 << endl);
      continue;
    }
    
    /* combine it all to final check-script */
    checkstr= "#==========================================\n";
    checkstr+="# KVALOBS check-script                                               \n";
    checkstr+="# type:" + cp->qcx() +                    "\n";
    checkstr+="#==========================================\n\n";

    checkstr+= "use strict;\n";

    checkstr+= metadata + meteodata + algostr;

    IDLOGINFO("html", "Final checkstring:" << endl
	      << "<font color=#007700>" << checkstr << "</font>" << endl);
    
    /* ----- run check ---------------------------------- */
    std::map<string, double> retvalues;
    result= parser.runScript(checkstr, retvalues);
    parser.freePerl();
    /* -------------------------------------------------- */

    if (!result){
      IDLOGERROR("html","kvQABaseControl::runChecks failed in parser.runScript"
		 << endl);
      continue;
    }
    
    // TEST --------------------------------------------------
    IDLOGDEBUG("html", "Successfully run check." << endl);
    IDLOGDEBUG("html", "Number of return parameters:" << retvalues.size()
	       << endl);
    if (retvalues.size() > 0){
      std::map<string, double>::iterator dp= retvalues.begin();
      for (; dp!=retvalues.end(); dp++)
	IDLOGDEBUG("html","Param:" << dp->first << " equals "
		   << dp->second << endl);
    }
    // ----------------------------------------------------------
    
    if (retvalues.size() == 0){
      IDLOGWARN("html","No return values from check - skip update.."
		<< endl);
      continue;
    }

    /* Update parameters with new control-flags and any return-variables
             - also update kvStationInfo */
    IDLOGINFO("html","Updating observation(s) with return values from check"
	      << endl);
    result= meteod.updateParameters(retvalues);

    if (!result){
      IDLOGERROR("html","kvQABaseControl::runChecks failed in updateParameters"
		 << endl);
      continue;
    }
  }

  parser.freePerl();

  IDLOGINFO("html","</PRE></CODE>");
  Logger::removeLogger("html");
  
  LOGINFO("kvQABaseControl::runChecks FINISHED" << endl);

  return 0;
}


//COMMENT:
//22072003 Bxrge Moe
//I have replaced the call of 'system' to run a shell command with a
//a call to mkdir. We have greater control of the creation of the
//directory. The regression system asumes that logdir is set up via 
//symblolic links and manipulates the links. In this senario we never 
//know where the links points or if the link is in a state of chance. We
//have a race between us and the regression system. It is better that we
//test and create a directory in a controlled way. We never creates a log 
//file if the the top most part of the directory dont exist ie. the link 
//does not exist. In this senario we may lose a logfile or two, but that 
//is not critical. In the case where the link is missing we open /dev/null 
//as the logfile.
//
//This solution is not bullet prof if the regresion system copys files
//and a file that is written to by us is at the same time read by the
//regression system. But it works as a quick fix.

milog::HtmlStream*
kvQABaseControl::openHTMLStream(long stationid, const miutil::miTime &obstime)
{
  using namespace std;

  milog::HtmlStream *html;
  ostringstream     ost;
  list<string>      pathlist;
  bool              error=false;

  ost << stationid;
  
  try{
    html=new HtmlStream();
  }
  catch(...){
    LOGERROR("OUT OF MEMMORY, cant allocate HtmlStream!");
    return 0;
  }

  pathlist.push_back(ost.str());
  pathlist.push_back(obstime.isoDate());

  ost.str("");
  //logpath_ is the directory where we shall put our 
  //html files.
  ost << logpath_;
 
  for(list<string>::iterator it=pathlist.begin();
      it!=pathlist.end() && !error;
      it++){
    ost << "/" << *it;

    if(mkdir(ost.str().c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)<-1){
      if(errno==EEXIST){
	continue;
      }else{
	error=true;
      }
    }
  }
 
  if(error){
    LOGINFO("kvQABaseControl::runChecks for station:" << stationid
	    << " and obstime:" << obstime << endl
	    << "Logging all activity to: /dev/null" << endl
	    << "A directory in the logpath maybe missing or a dangling link!");
     html->open("/dev/null");
     return html;
  }
  
  ost <<  "/log-" << obstime.isoClock()
      << ".html";
 
  miutil::miString logfilename = ost.str();
  logfilename.replace(":","-");

  LOGINFO("kvQABaseControl::runChecks for station:" << stationid
	  << " and obstime:" << obstime << endl
	  << "Logging all activity to:" << logfilename << endl);
  

  if(!html->open(logfilename)){
    LOGERROR("Failed to create logfile for the html output. Filename:\n" <<
	     logfilename << "\nUsing /dev/null!");
    html->open("/dev/null");
  }

  return html;
}
