/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: kvinputd_clt.cc,v 1.1.2.6 2007/09/27 09:02:18 paule Exp $

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
#include <string.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include "lib/kvalobs/kvapp.h"
#include "lib/miutil/httpclient.h"
#include "kvDataInputd/kvdataclt/Option.h"

using std::string;
using std::endl;
using std::cerr;
using std::ifstream;
using std::ostringstream;

bool
readFile(const std::string &file, std::string &content, std::string &decoder);


int doHttp(Options &opt) {
  string url = opt.httpServer();
  string file = opt.theFile();

  if (url.empty())
    return 1;

  if (file.empty()) {
    cerr << "Missing file or file do not exist." << endl;
    return 1;
  }

  miutil::HTTPClient http;

  try {
    http.postFile(url, file, "text/plain");
  } catch (const miutil::HttpException &ex) {
    cerr << "HttpError: " << ex.what() << endl;
    return 1;
  }

  cerr << "Time: " << http.totalTime() << "s" << endl;
  cerr << "ReturnCode:     " << http.returnCode() << endl;
  cerr << "Content-length: " << http.contenlLength() << endl;
  cerr << "Content-type:   " << http.contentType() << endl;
  cerr << "Content:" << endl;
  cerr << http.content() << endl << endl;
  return 0;
}

int main(int argc, char** argv) {
  Options opt;

  if (!opt.parse(argc, argv))
    return 1;

  return doHttp(opt);
}

bool readFile(const std::string &file, std::string &content,
              std::string &decoder) {
  ifstream fist(file.c_str());
  ostringstream ost;
  string data;

  if (!fist) {
    cerr << "Cant open file: " << file << endl;
    return false;
  }

  decoder.erase();
  content.erase();

  while (decoder.empty())
    getline(fist, decoder);

  while (getline(fist, data))
    ost << data << endl;

  if (!fist.eof()) {
    cerr << "Cant read file: " << file << endl;
    fist.close();
    return false;
  }

  fist.close();
  content = ost.str();

  return true;
}
