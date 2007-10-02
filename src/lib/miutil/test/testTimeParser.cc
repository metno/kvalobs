/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: testTimeParser.cc,v 1.1.2.4 2007/09/27 09:02:32 paule Exp $                                                       

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
#include <string>
#include "../include/miutil/miTimeParse.h"

using namespace std;
using namespace miutil;

int
main(int argn, char **argv){
	miTime nt("2006-01-02 19:30:00");
	string buf("200601020930bla");
	miTime time;
	
	cerr << "nt: " << nt << endl;
	cerr << "          10        20        30" << endl;
	cerr << "0123456789012345678901234567890" << endl;
	cerr << buf << endl;
	
	try{
		string::size_type i=miTimeParse("%Y%m%d%H%M", buf, time, nt);
		
		cerr << "string to parse: [" << buf << "] next pos: " << i << endl;
		cerr << "time: " << time << endl;
	}
	catch(miTimeParseException ex){
		cerr << ex.what() << endl;
	}
	
	
	try{
		buf="010220";
		string::size_type i=miTimeParse("%m%d%H", buf, time, nt);
		cerr << "nt: " << nt << endl;	
		cerr << "string to parse: [" << buf << "] next pos: " << i << endl;
		cerr << "time: " << time << endl;
	}
	catch(miTimeParseException ex){
		cerr << ex.what() << endl;
	}	
}
