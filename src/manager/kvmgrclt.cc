/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvmgrclt.cc,v 1.1.2.4 2007/09/27 09:02:35 paule Exp $                                                       

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
#include <fstream>
#include <iostream>
#include <list>
#include <vector>

#include <sstream>
#include <db/dbdrivermgr.h>
#include <kvalobs/kvDbGate.h>
#include <kvalobs/kvStation.h>
#include <kvalobs/kvData.h>
#include <kvalobs/kvWorkelement.h>
#include <puTools/miString>
#include "kvmgrcltApp.h"

using namespace std; 
using namespace miutil;
using namespace kvalobs;
using namespace dnmi::db;


int main(int argc, char** argv)
{
	miutil::miTime undefTime;
  	Connection    *con;

	KvMgrCltApp app(argc, argv);

	con=app.getNewDbConnection();

 	if(!con){
 		cerr << "Cant connect to the database!" << endl;
 		exit(1);
 	}
 
    kvDbGate gate(con);



//	if(!gate.insert(kvWorkelement(180,
//								  miTime("2005-12-16 06:00:00"), 
//								  330, 
//								  miTime::nowTime(), 
//								  10,
//								  undefTime,
//				                  undefTime, 
//				                  undefTime, 
//				                  undefTime, 
//				                  undefTime), 
//		                          true)){
//    	cerr << "Can't save kvWorkelement into the" << endl <<
//	    		"the table 'workque' in  the database!" << endl <<
//	    		"[" << gate.getErrorStr() << "]" << endl;
//	}else{
//		app.sendSignalToManager(180, 330, miTime("2005-12-16 06:00:00"));
//	}
//	
//	return 0;




    // Creating station list ...

    list<kvStation> stations;
    gate.select(stations," where static=true order by stationid");


	list<kvData>           data;
	list<kvData>::iterator dit;	
	ostringstream          ost;
	
	miTime startObst("2005-12-16 06:00:00");
	miTime endObst(startObst);
	miTime obst(startObst);
	//endObst.addDay(1);
	
	cerr << "Obstime: " << obst << endl;
	
	for(list<kvStation>::const_iterator sit=stations.begin();
		sit!=stations.end(); 
		++sit){
					
		ost.str("");
		ost << " where stationid=" << sit->stationID() << " AND "
			<< "       obstime='" << obst.isoTime() << "'" 
			<< "       order by typeid";
	
		if(!gate.select(data, ost.str())){
			cerr << "Cant query the data table! " << endl 
				 << " Reason: " << gate.getErrorStr() << endl;
			goto error;
		}
		
		dit=data.begin();
		
		if(dit==data.end()){
			cerr << sit->stationID() << ", " << obst << " NO data!" << endl;
			continue;
		}
		
		while(dit!=data.end()){
			int      tid=dit->typeID();
			int      sid=dit->stationID();
			miTime dObst=dit->obstime();
			miTime tbt=dit->tbtime();
			bool     qc1=false;  

			for(; dit!=data.end() && dit->typeID()==tid; ++dit);
			
			cerr << dObst << ", " << sid << ", " << tid << "  check observation!";				

  			if(!gate.insert(kvWorkelement(sid, 
										  dObst, 
										  tid, 
										  tbt, 
										  10,
										  undefTime,
			                              undefTime, 
			                              undefTime, 
			                              undefTime, 
			                              undefTime), 
		                                     true)){
    			cerr << "Can't save kvWorkelement into the" << endl <<
	     				"the table 'workque' in  the database!" << endl <<
	    				"[" << gate.getErrorStr() << "]" << endl;
		    }else{
		    	if(app.sendSignalToManager(sid, tid, dObst))
		    		cerr << " (Manager Signaled!)";
		    	else
		    		cerr << " (FAILED Signaling Manager!)";
		    		
		    	cerr << endl;
		    }
		}
	}


	//for(;sit!=stations.end(); ++sit) 
	//	cout << sit->stationID() << "  " << sit->name() << endl;

error:
	app.releaseDbConnection(con);

    return 0;
};
