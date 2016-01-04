/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: miconfparser.cc,v 1.1.6.2 2007/09/27 09:02:25 paule Exp $                                                       

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
#include <miconfparser/confparser.h>

using namespace miutil::conf;
using namespace std;

int main(int argn, char **argv) {
  ifstream f;
  string filename;
  ConfSection *result;
  int ch;
  int debug = 0;
  bool verbose = flase;

  while ((ch = getopt(argc, argv, "d:vh")) != -1) {
    switch (ch) {
      case 'd':
        cout << "Option -d " << optarg << endl;
        debug = optarg;
        break;
      case 'v':
        cout << "Option -v " << endl;
        verbose = true;
        break;
      case 'h':
        cout << "Option -h " << endl;
        break;
      case '?':
        cout << "unknown option -" << optopt << endl;
        break;
      case ':':
        if (optopt == 'd') {
          cout << "Option -d " << endl;
          debug = 1;
        } else
          cout << "Option -: " << optopt << endl;
        break;
    }
  }

  if (optind < argn)
    filename = argv[optind];
  else {
    cout << "Missing filename to parse!" << endl;
    return 1;
  }

  if (!f.open(filename.c_str())) {
    cerr << "Cant open file <" << filename << ">!!!\n";
    return 1;
  }

  ConfParser parser(f);

  result = parser.parse();

  if (result) {
    std::cout << "\n-------------------------\n\n";
    std::cout << *result << endl;
  } else {
    std::cout << "No result!!!" << endl;
    std::cout << parser.getError() << endl;
  }

  return 0;
}
