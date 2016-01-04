/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: model2kv.cc,v 1.5.2.1 2007/09/27 09:02:37 paule Exp $                                                       

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
#include <setup.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <puTools/miTime.h>
#include <puCtools/timemanip.h>
#include <boost/algorithm/string.hpp>

using namespace std;

using namespace miutil;

/// program to prepare model-data from "pose" to
/// the kvalobs table format
/// juergens@met.no

map<std::string, std::string> pi_map;
std::string model_id;
miTime from;
miTime to;
vector<std::string> allpar;

void setTime(miTime& to, timeline t) {
  to.setTime(t.realtime.year, t.realtime.month, t.realtime.day, t.realtime.hour,
             0, 0);
}

/// reads the list of parameter index from a file.
/// the file is created by kv2metdat and is dumped
/// independently from this job. So the system can 
/// run without an existing database

bool readPindexMap(const std::string& fname) {
  ifstream pin(fname.c_str());

  std::string line;
  vector<std::string> words;

  while (pin) {
    getline(pin, line);
    boost::trim(line);
    if (line.empty())
      continue;
    boost::split(words, line, boost::is_any_of(":"));
    if (words.size() < 2)
      continue;
    pi_map[words[0]] = words[1];
  }
  return !pi_map.empty();

}

/// read and format for upload
/// the real upload is done in a proper script
/// (see $KVALOBS/src/script/model2kvalobs/bin)

std::string format2kvalobs(struct filecontents* datain) {

  ostringstream out;

  int i, j, k;
  std::string station, pindex, pi;
  float scale;
  miTime valid;

  if (datain) {
    setTime(from, *(datain->time[0]));
    setTime(to, *(datain->time[datain->ntime - 1]));

    for (j = 0; j < datain->npar; j++) {
      pi = std::string(datain->par[j]->alias);

      if (bool(pi_map.count(pi))) {
        pindex = pi_map[pi];
        allpar.push_back(pindex);

        for (i = 0; i < datain->npos; i++) {
          station = std::string(datain->pos[i]->name);

          scale = powf(10, float(datain->par[j]->scale));

          for (k = 0; k < datain->ntime; k++) {
            setTime(valid, *(datain->time[k]));

            if (datain->data)
              out << station << "," << valid << "," << pindex << ",0,"
                  << model_id << ","
                  << scale * float(datain->data[i][0][j][k][0].data[0]) << endl;
          }
        }
      }
    }
  }
  return out.str();
}
;

/// program to prepare model-data from "pose" to
/// the kvalobs table format
/// juergens@met.no

int main(int argc, char** argv) {
  string kvParams;
  int ip = -1;

  if (argc < 3) {
    cerr << "Usage: " << argv[0]
         << " -p <param.kvalobs>  <pose.input> <infile> <outfile>" << endl;
    return 1;
  }

  for (int i = 0; i < argc; ++i) {
    if (strcmp(argv[i], "-p") == 0) {
      if ((i + 1) < argc) {
        kvParams = argv[i + 1];
      }
      ip = i;
      break;
    }
  }

  if (kvParams.empty()) {
    cerr << "Missing kvalobs parameterfile. Use the -p parameterfile option!"
         << endl;
    return 1;
  }

  //Remove the -p parameterfile option from the argv list.

  if (ip < 0) {
    cerr << "Unrecoverable internal error ...." << endl;
    return 1;
  }

  for (int i = ip; i < (argc - 2); ++i)
    argv[i] = argv[i + 2];

  argc -= 2;

  /* Debug:
   for( int i=0; i<argc; ++i )
   cerr << "argv[" << i << "]= '"<<argv[i]<< "'" << endl;
   */

  if (!readPindexMap(kvParams)) {
    cerr << "Could not read <" << kvParams
         << "> for identification ... exiting!" << endl;
    return 1;
  }

  model_id = "0";

  struct parameter** parameterlist;
  struct filecontents *datain;

  int parlistsz;

  // get data from pose stuff

  set_cl(argc, argv);
  parseinput();
  parsepardefs(setup.pardeffile, &parameterlist, &parlistsz);

  datain = static_cast<struct filecontents*>(malloc(
      setup.nfiles * sizeof(struct filecontents)));

  readDataFiles(setup.posfilelist, datain, setup.nfiles, &parameterlist,
                &parlistsz, &setup.sub);

  if (setup.nfiles)
    if (setup.posfilelist[0].modname)
      model_id = setup.posfilelist[0].modname;

  ofstream pout(setup.outfile);

  clean_setup();

  std::string buf = format2kvalobs(datain);

  pout << "COPY  model_data from stdin using delimiters ',';" << endl << buf
       << "\\." << endl;

  cleanupFilecontents(datain, setup.nfiles);

  return 0;
}
;
