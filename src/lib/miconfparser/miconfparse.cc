/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: miconfparse.cc,v 1.3.6.3 2007/09/27 09:02:25 paule Exp $                                                       

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
#include <stdlib.h>
#include <ctype.h>
#include <fstream>
#include <miconfparser/miconfparser.h>
#include <getopt.h>

using namespace miutil::conf;
using namespace std;

void use();

int main(int argn, char **argv) {
  ifstream f;
  string filename;
  ConfSection *result;
  int ch;
  int debug = 0;
  bool verbose = false;
  bool regression = false;
  bool listOutputBrace=false; 
  bool listUseBrace=false;
  bool allowMultipleSections=false;
  opterr = 0;  //dont print to standard error.

  while ((ch = getopt(argn, argv, "d:vhrbpm")) != -1) {
    switch (ch) {
      case 'd':
        if (!isdigit(optarg[0])) {
          cout << "Inavlid debugmode: " << optarg << endl;
          use();
        }
        debug = atoi(optarg);
        break;
      case 'v':
        verbose = true;
        break;
      case 'h':
        use();
        break;
      case '?':
        cout << "Unknown option -" << static_cast<char>(optopt) << endl;
        use();
        break;
      case 'r':
        regression = true;
        break;
      case 'b':
        listOutputBrace=true;
        break;
      case 'p':
        listUseBrace=true;
        break;
      case 'm':
        allowMultipleSections=true;
        break;
      case ':':
        cout << "Option -: " << optopt << " missing argument!" << endl;
        use();
        break;
    }
  }

  if (regression) {
    debug = 0;
    verbose = false;
  }

  
  if (optind < argn)
    filename = argv[optind];
  else {
    cout << "Missing filename to parse!" << endl;
    use();
    return 1;
  }

  f.open(filename.c_str());

  if (!f) {
    cerr << "Cant open file <" << filename << ">!!!\n";
    return 1;
  }

  if(listUseBrace) {
    miutil::conf::setListChars("[]");
  }

  ConfParser parser(f, allowMultipleSections);

  parser.debugLevel(debug);
  
  result = parser.parse();

  if (result) {
    if( listOutputBrace ) {
      debug = false;
      miutil::conf::setListChars("[]");
      std::cout << *result << endl;
      delete result;
      exit(0);
    }
    
    if (debug > 0 && verbose)
      std::cout << "\n-------------------------\n\n";
    
    if (verbose || regression)
      std::cout << *result << endl;

    if (!regression)
      cout << "\n ---- No error ---\n";

    delete result;
  } else {
    std::cout << "\n --- errors ---\n";
    std::cout << parser.getError() << endl;
  }

  return 0;
}

void use() {
  cout << "\n\tmiconfparse -dn -v -h -r filename\n"
       << "\t\t-dn debugmode, for debugging the parser.\n"
       << "\t\t    n=0,1,2 or 3, higher n -> more debug information.\n"
       << "\t\t-h  print this help screen and exit.\n"
       << "\t\t-v  print the the content of the file, but from\n"
       << "\t\t    the parse tree.\n"
       << "\t\t-r  same as -v, but print the parse tree in a form that\n"
       << "\t\t    can be parsed again. The -d option is ignored!\n"
       << "\t\t-b  output list as [], this make the result HCON readable.\n"
       << "\t\t-p  The parser assumes list use [] in the configuration.\n"
       << "\t\t-m  allow multiple sections in in the configuration.\n\n";

  exit(1);
}
