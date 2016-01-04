/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: testdir.cc,v 1.2.2.2 2007/09/27 09:02:29 paule Exp $                                                       

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
#include "../include/fileutil/file.h"
#include "../include/fileutil/dir.h"

using namespace dnmi::file;
using namespace std;

int main(int argn, char **argv) {
  Dir dir;
  string file;
  File f;

  if (!dir.open("../obj", "*.o")) {
    cerr << "Can't open directory!\n";
    return 1;
  }

  try {
    while (dir.hasNext()) {
      file = dir.next();

      cerr << "Found file: <" << file << ">\n";
    }
  } catch (DirException &ex) {
    cerr << "Exception: " << ex.what() << endl;
  }

  try {
    dir.rewind();

    while (dir.hasNext()) {
      dir.next(f);

      cerr << "Found file: <" << f.name() << ">\n";
      cerr << "......path: <" << f.path() << ">\n";
      cerr << "......file: <" << f.file() << ">\n";
    }
  } catch (DirException &ex) {
    cerr << "Exception: " << ex.what() << endl;
  }

}
