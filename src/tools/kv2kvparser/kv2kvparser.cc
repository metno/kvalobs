/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id$                                                       

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
#include <string>
#include <list>
#include <fstream>
#include <milog/milog.h>
#include <fileutil/readfile.h>
#include <decodeutility/kvDataFormatter.h>
#include <decodeutility/kvalobsdataparser.h>
#include <decodeutility/KvDataContainer.h>
#include "lib/miutil/timeconvert.h"


namespace pt = boost::posix_time;


using namespace std;
//using namespace kvalobs::decoder;
using namespace kvalobs::serialize;
using namespace kvalobs;
using namespace decodeutility::kvdataformatter;
using decodeutility::KvDataContainer;
using dnmi::file::ReadFile;

void parse(KvalobsData & data, const std::string & obs)  {
  try {
    internal::KvalobsDataParser::parse(obs, data);
  } catch (exception & e) {
    cerr << "Failed to parse file. " << e.what() << endl;
    exit(2);
  }
}



int main(int argn, char **argv) {
  string xml;
  if( argn < 2) {
    cerr << "Must give a xml file to parse\n";
    return 1;
  }

  cerr << "parsing file '" << argv[1] << "'\n";

  if( ! ReadFile(argv[1], xml) ) {
    cerr << "Failed to read file '"<< argv[1]<<"'.\n";
    return 1;
  }

  KvalobsData data;
  parse(data, xml);

  list<kvData> dl;
  list<kvTextData> tdl;

  data.data(dl,tdl);

  for ( auto &it : dl) {
    cerr << pt::to_kvalobs_string(it.obstime()) << "," << it.stationID() << "," << it.typeID() << "," << it.paramID() << "," << it.sensor() << ","
              << it.level() << "," << it.original() << ", " << pt::to_kvalobs_string(it.tbtime()) << endl;
      
  }

  
}
