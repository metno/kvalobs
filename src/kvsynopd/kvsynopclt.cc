/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvsynopclt.cc,v 1.5.2.3 2007/09/27 09:02:23 paule Exp $                                                       

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
#include <unistd.h>
#include <getopt.h>
#include <iostream>
#include <sstream>
#include <kvskel/kvsynopd.hh>
#include <list>
#include <dnmithread/mtcout.h>
#include "kvsynopCltApp.h"

using namespace std;

int
main(int argn, char **argv)
{
  Options opt;
  SynopCltApp app(argn, argv);

  opt=app.options();

  if(opt.cmd==Options::Uptime){
    miutil::miTime startTime;
    long           u;

    if(app.uptime(startTime, u)){
      int days=u/86400;
      int hours=(u%86400)/3600;
      int min=((u%86400)%3600)/60;
      int sec=((u%86400)%3600)%60;
      COUT("kvsynopd:\n\tStarted: " << startTime << "\n\tuptime: "
	   << days << " days " << hours << " hours " <<
	   min << " minutes " << sec << " seconds!\n");
    }else{
      CERR("Cant get uptime from <kvsynopd>!\n");
    }
  }else if(opt.cmd==Options::StationList){
    kvsynopd::StationInfoList list; 

    if(app.stationsList(list)){
      for(CORBA::ULong i=0; i<list.length(); i++){
	COUT("wmono: " << list[i].wmono << endl);
	COUT("   stationid: ");
	
	for(CORBA::ULong j=0; j<list[i].stationIDList.length(); j++){
	  COUT(list[i].stationIDList[j] << " ");
	}
	COUT(endl);
	
	string info(list[i].info);
	info.insert(0, "   ");
	string::size_type index=info.find("\n");
	
	while(index!=string::npos){
	  info.replace(index, 1, "\n   ");
	  index++;
	  index=info.find("\n", index);
	}
	
	COUT(info << endl << endl);
      }
    }else{
      CERR("Cant get station list from <kvsynopd>!");
    }
  }else if(opt.cmd==Options::Delays){
    miutil::miTime t;
    kvsynopd::DelayList dl;

    if(!app.delayList(dl, t)){
      CERR("Cant get delay list from <kvsynopd>!");
    }else{
      COUT("Delay list at: " << t << endl <<
	   "---------------------------------------------------" << endl);
      for(CORBA::ULong i=0; i<dl.length(); i++){
	COUT("wmono: " << setfill('0') << setw(5) << dl[i].wmono 
	     << " obstime: " << dl[i].obstime 
	     << " delay to: " << dl[i].delay << endl<< setfill(' '));
      }
      COUT(endl);
    }
  }else if(opt.cmd==Options::Synop){
     
    if(opt.time.undef()){
      CERR("Invalid time <" << opt.time << ">!");
    }else{
      Options::IIntList it=opt.wmonoList.begin();
      kvsynopd::SynopData d;
      TKeyVal keyvals;
      
      for(; it!=opt.wmonoList.end(); it++){
	if(!app.createSynop(*it, opt.time, keyvals, 20, d)){
	  if(app.shutdown()){
	    break;
	  }else{
	    CERR("Create synop failed!\n");
	  }
	continue;
	}
	
	if(!d.isOk){
	  CERR("Cant create synop for <" << d.stationid << ">!\n"
	       << "Reason: " << d.message << endl);
	  continue;
	}
	
	CERR("Created synop for <" << d.stationid << "> termin: " << d.termin
	     << endl << "Message: " << d.message << endl << "Synop: " << endl
	     << d.synop << endl << endl);
      }

    }
  }else if(opt.cmd==Options::Reload){
    CERR("OPTION: Reload!" << endl);
    app.reloadConf();
  }else if(opt.cmd==Options::CacheReload){
    string msg;
    int  count;

    kvsynopd::ReloadList *reloadlist=app.cacheReloadList(msg);

    if(!reloadlist){
      CERR("Cant get station list from <kvsynopd>!");
    }else{
      COUT("wmono: ");
      count=0;

      for(CORBA::ULong i=0; i<reloadlist->length(); i++, count++){

	if(count==8){
	  COUT("\n-----  ");
	  count=0;
	}

	COUT((*reloadlist)[i].wmono << "(" << 
	     (*reloadlist)[i].eventsWaiting << ") ");
      }
      
      COUT(endl<<endl);
      delete reloadlist;
    }
  }else{
      CERR("Unknown option!");
  }
 
  app.doShutdown();
  return 0;
}


