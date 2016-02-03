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
#ifndef SRC_KVDATAINPUTD_KVDATACLT_OPTION_H_
#define SRC_KVDATAINPUTD_KVDATACLT_OPTION_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include "lib/kvalobs/kvPath.h"
#include "lib/miconfparser/miconfparser.h"

using std::string;
using std::endl;
using std::cout;
using std::cerr;
using std::endl;

void
use(bool exit = true);

struct Options {
  string file;
  string confile;
  string decoder;
  string kvserver;
  bool verbose;
  string cwd;
  miutil::conf::ConfSection *conf;

  Options()
      : verbose(false),
        conf(0) {
    char buf[PATH_MAX];
    if (getcwd(buf, PATH_MAX)) {
      cwd = buf;

      if (cwd[cwd.length() - 1] != '/')
        cwd += "/";
    }
  }

  bool loadConf() {
    conf = miutil::conf::ConfParser::parse(confile);

    if (!conf) {
      cerr << "Error parsing conf file '" << confile << "'." << endl;
      return false;
    }

    return true;
  }

  string theFile() {
    if (!file.empty() && fileExist(file))
      return file;
    return "";
  }

  string httpServer() {
    string server;

    if (kvserver.empty()) {
      if (!loadConf())
        return "";

      miutil::conf::ValElementList v = conf->getValue("kvserver");

      if (v.empty()) {
        cerr << "Missing key 'kvserver' in conf file '" << confile << "'." << endl;
        return "";
      }

      kvserver = v.valAsString();
    }

    if (kvserver.find("http://") == string::npos)
      server = "http://" + kvserver;

    string::size_type i = server.find_first_of("/", 7);
    if (i == string::npos)
      server += "/v1/observation";
    else if (i == (server.length() - 1))
      server += "v1/observation";
    else if (*server.rbegin() != '/')
      server += "/";

    return server;
  }

  bool fileExist(const std::string &file) {
    struct stat sbuf;

    if (stat(file.c_str(), &sbuf) == 0)
      return S_ISREG(sbuf.st_mode);
    else
      return false;
  }

  string mkfpath(const string &path, const string &file) {
    string p;
    if (path.empty())
      return cwd + file;

    if (path[0] != '/')
      p = cwd + path;
    else
      p = path;

    if (*p.rbegin() != '/')
      return p + "/" + file;
    else
      return p + file;
  }

  bool parse(int argc, char** argv) {
    char ch;
    opterr = 0;  // dont print to standard error.

    while ((ch = getopt(argc, argv, "c:d:s:hv")) != -1) {
      switch (ch) {
        case 'c':
          confile = optarg;
          break;
        case 'd':
          decoder = optarg;
          break;
        case 'h':
          use();
          break;
        case 's':
          kvserver = optarg;
          break;
        case 'v':
          verbose = true;
          break;
        case '?':
          cout << "Unknown option -" << static_cast<char>(optopt) << endl;
          use();
          break;
        case ':':
          cout << "Option -: " << optopt << " missing argument!" << endl;
          use();
          break;
      }
    }

    if (decoder.empty() && verbose) {
      cerr << "Missing -d decoder\n\n";
      cerr << "Assume the first line in the file specifies the decoder!\n\n";
    }

    if (optind < argc) {
      file = argv[optind];
    } else {
      cerr << "Missing filename!" << endl;
      use();
      return false;
    }

    if (!confile.empty()) {
      if (confile[0] != '/')
        confile = mkfpath(cwd, confile);
    } else {
      confile = mkfpath(cwd, ".kvdataclt");
      if (!fileExist(confile)) {
        confile = mkfpath(getenv("HOME"), ".kvdataclt");

        if (!fileExist(confile))
          confile.erase();
      }
    }

    if (confile.empty())
      confile = mkfpath(kvPath(kvalobs::sysconfdir), "kvdataclt.conf");

    if (kvserver.empty() && !fileExist(confile)) {
      cerr << "No configuration file found. Use --help for more information..\n";
      return false;
    }
    return true;
  }
};

void use(bool exit_) {
  cout << "\n\n  kvdataclt [-d decoder] [-h] filename\n\n" << "\t-d decoder, ex. synop, autoobs, ....\n"
       << "\t-c conffile use this confile instead of kvdataclt.conf\n" << "\t-h print this help screen and exit!\n" << "\t-v be verbose.\n"
       << "\t-s kvalobs server to send the data to.\n\n" << "  If -d option is missing: Assume the decoder is given with the first line in the file.\n\n"
       << "  If -s option is missing: Search for a configuration files named .kvdataclt in the current\n"
       << "  directory, and the in the home directory. It then search the kvalobs sysconf directory for\n" << "  kvdataclt.conf. \n\n"
       << "  The the configuration file format is: kvserver=host:port\n\n";

  if (exit_)
    exit(1);
}

#endif  // SRC_KVDATAINPUTD_KVDATACLT_OPTION_H_
