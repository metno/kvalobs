/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvpush.cc,v 1.1.2.2 2007/09/27 09:02:48 paule Exp $                                                       

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
#include <iostream>
#include <list>
#include <sstream>
#include <kvalobs/kvDbGate.h>
#include <kvalobs/kvStation.h>
#include <kvalobs/kvData.h>
#include <kvalobs/kvWorkelement.h>
#include <kvalobs/kvPath.h>
#include "kvpushApp.h"

using namespace std; 
using namespace miutil;
using namespace kvalobs;
using namespace dnmi::db;



void 
use(int exitcode);

int 
main(int argc, char** argv)
{
	miutil::miTime undefTime;
  	Connection     *con;
	Options        opt;
	
	KvPushApp app(argc, argv);

	try{
		app.getOpt(argc, argv, opt);
	}
	catch(GetOptEx &ex){
		cerr << "Exception: " << ex.what() << endl << endl;
		use(1);
	}


   cout << opt << endl;	

	if(opt.help)
		use(0);
		
	if(opt.typeids.empty() && opt.stations.empty()){
		cout << "typelist and stationlist cant both be empty!" 
			  << endl << endl;
		use(1);
	}
	
	if(opt.fromtime.undef()){
		cout << "Fromtime must be given!" 
			  << endl << endl;
		use(1);
	}
	
	
	if(opt.fromtime > opt.totime){
		miutil::miTime tmp=opt.fromtime;
		opt.fromtime=opt.totime;
		opt.totime=tmp;
	}
	
	con=app.getNewDbConnection();

 	if(!con){
 		cerr << "Cant connect to the database!" << endl;
 		exit(1);
 	}

	
	app.selectDataAndUpdateWorkque(opt);
	
	app.releaseDbConnection(con);

	return 0;
};

void 
use(int exitcode)
{
	cout << "kvpush, a helper program to trigger pushing of" << endl
		  << "data to subscribers of kvalobs data.!" << endl << endl 
		  << "Use" << endl
		  << "   kvpush [-h] [-i typeidlist] -s stationlist -f fromdate [-t todate]" << endl
		  << "\t-h            Print this help screen and exit!" << endl
		  << "\t-i typeidlist The typeid(s) of the data to push. This is a list" << endl
		  << "\t              on the form typeid1,typeid2,typeid3" << endl
		  << "\t              There must be no space between the the commas" << endl
		  << "\t              and the typid(s)." << endl
		  << "\t              All types is used if the list is empty or if " << endl
		  << "\t              -i is omited." << endl
		  << "\t-s stationlist A list of stations to push data form. The " << endl
		  << "\t              The list must be on the form st1,st2,stN" << endl
		  << "\t              There must be now space between the stations." << endl
		  << "\t-f fromdate   Push data from this date." << endl
		  << "\t-f todate     Push data to this date. If omitted "<< endl
		  << "\t              current time is used." << endl << endl
		  << "Date format: " << endl
		  << "   'YYYY-MM-DDThh:mm:ss', mm:ss in the time part can be omitted." << endl 
		  << endl;
		 
	exit(exitcode);
}

