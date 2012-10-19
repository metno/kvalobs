/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: testmkdir.cc,v 1.1.2.3 2007/09/27 09:02:29 paule Exp $                                                       

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
#include <iostream>
#include <string>
#include "../include/fileutil/mkdir.h"

using namespace std;
using namespace dnmi::file;

int
main(int argn, char **argv)
{
	string path;
	string newdir("my");
	
	cerr << "createdir my  ....." ;
	if(!mkdir(newdir, path)){
		cerr << "Failed!" << endl;
		
		return 1;
	}
	cerr << "Ok!" << endl;
	cerr << "create an existing dir ....";
	if(!mkdir(newdir, path)){
		cerr << "Failed!" << endl;
		
		return 1;
	}
	
	cerr << "Ok!" << endl;
	path=newdir;
	newdir="dir1";
	
	cerr << "create a dir in a subdir .... ";
	
	if(!mkdir(newdir, path)){
		cerr << "Failed!" << endl;
		
		return 1;
	}
	
	cerr << "Ok!" << endl;
	
	
	newdir="/dir2/dir3";
	cerr <<"create a dir with parrentsdir ....";
	if(!mkdir(newdir, path)){
		cerr << "Failed!" << endl;
		
		return 1;
	}
	
	cerr << "ok!" << endl;
	
	cerr << "create dir that allready exist ...";
	
	if(!mkdir(newdir, path)){
		cerr << "Failed!" << endl;
		
		return 1;
	}
	
	cerr << "Ok!" << endl;
	
	return 0;
}
