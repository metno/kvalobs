/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: kvapp.cc,v 1.22.2.4 2007/09/27 09:02:31 paule Exp $

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
#include <stdio.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <string>
#include <stdlib.h>
#include <sstream>
#include <fstream>
#include <miconfparser/miconfparser.h>
#include <milog/milog.h>
#include <dnmithread/mtcout.h>
#include <kvalobs/kvbaseapp.h>
#include <kvalobs/kvPath.h>
#include <fileutil/pidfileutil.h>
#include <kvalobs/getLogInfo.h>

using namespace std;
using namespace miutil::conf;

namespace {
ConfSection* confLoader(const string &defaultConfName);
std::string getAppName(const std::string &progname);

}

ConfSection* KvBaseApp::conf = 0;
std::string KvBaseApp::confFile;
std::string KvBaseApp::pidfile;
std::string KvBaseApp::appName;
milog::LogLevel KvBaseApp::globalLogLevel = milog::WARN;

KvBaseApp::KvBaseApp(int argn, char **argv, const char *opt[0][2])
    : setAppNameForDb(false) {
  string corbaNS;
  string kvconfig;
  ValElementList val;

  appName = getAppName(argv[0]);

  for (int i = 0; i < argn; i++) {
    if (strcmp(argv[i], "--help") == 0) {
      printUseMsgAndExit(0);
    } else if (strcmp(argv[i], "--kv-config") == 0) {
      cout << "--kv-config\n";
      if ((i + 1) < argn) {
        i++;
        cout << "--kv-config [" << argv[i] << "]\n";
        setConfFile(argv[i]);
      }
    }

  }

  //Sets the variable conf
  getConfiguration();

  if (conf) {
    val = conf->getValue("database.set_app_name");

    if (val.size() > 0) {
      string sval = val[0].valAsString();
      if (!sval.empty() && (sval[0] == 't' || sval[0] == 'T'))
        setAppNameForDb = true;
    }
  }
}

KvBaseApp::~KvBaseApp() {
  if (conf) {
    delete conf;
  }
}

void KvBaseApp::useMessage(std::ostream &os) {
  os << appName << endl << endl;
  os << "Use: " << endl << endl;
  os << "Standard kvalobs configuration options:\n\n"
     << "\t\t--kv-config <filename> Read the configuration file <filename>\n"
     << "\t\t                       instead of the kvalobs.conf\n";
  os << "\t\t--help Print this help message and exit!\n\n";
}

void KvBaseApp::printUseMsgAndExit(int exitStatus) {
  useMessage(cout);
  exit(exitStatus);
}

std::string KvBaseApp::createPidFileName(const std::string &progname) {
  std::string path(kvPath("rundir"));
  return dnmi::file::createPidFileName(path, progname);
}

void KvBaseApp::createPidFile(const std::string &progname) {
  FILE *fd;

  pidfile = createPidFileName(progname);

  LOGINFO("Writing pid to file <" << pidfile << ">!");

  fd = fopen(pidfile.c_str(), "w");

  if (!fd) {
    LOGWARN("Can't create pidfile <" << pidfile << ">!\n");
    pidfile.erase();
    return;
  }

  fprintf(fd, "%ld\n", (long) getpid());
  fclose(fd);
}

void KvBaseApp::deletePidFile() {
  if (pidfile.empty())
    return;

  LOGINFO("Deleting pidfile <" << pidfile << ">!");

  unlink(pidfile.c_str());
}

std::string KvBaseApp::createConnectString(const std::string &dbname,
                                           const std::string &kvdbuser,
                                           const std::string &host,
                                           const std::string &port) {
  char *buf;
  stringstream ost;
  ConfSection *myConf = KvBaseApp::getConfiguration();

  if (myConf) {
    string val = myConf->getValue("database.dbconnect").valAsString("");

    if (!val.empty()) {
      LOGINFO("Using 'database.dbconnect' from configuration file");
      return val;
    }
  }

  ost << "dbname=";

  if (dbname.empty()) {
    buf = getenv("KVDB");
    if (buf)
      ost << buf;
    else
      ost << "kvalobs ";
  } else
    ost << dbname;

  ost << " ";

  if (host.empty()) {
    buf = getenv("PGHOST");

    if (buf)
      ost << "host=" << buf << " ";
  } else
    ost << "host=" << host << " ";

  if (port.empty()) {
    buf = getenv("PGPORT");

    if (buf)
      ost << "port=" << buf << " ";
  } else
    ost << "port=" << port << " ";

  if (kvdbuser.empty()) {
    buf = getenv("KVDBUSER");

    if (buf)
      ost << "user=" << buf << " ";
    else
      ost << "user=kvalobs ";
  } else
    ost << "user=" << kvdbuser << " ";

  return ost.str();
}

miutil::conf::ConfSection*
KvBaseApp::getConfiguration() {
  if (!KvBaseApp::conf) {
    KvBaseApp::conf = confLoader(KvBaseApp::appName + ".conf");

    if (conf)
      globalLogLevel = getLogLevel("", conf);
  }

  return conf;
}

milog::LogLevel KvBaseApp::getLogLevel(const std::string &section,
                                       miutil::conf::ConfSection *conf) {
  string key;

  if (!conf)
    conf = getConfiguration();

  milog::LogLevel ll = ::getLoglevel(conf, section);

  if (ll == milog::NOTSET)
    ll = milog::WARN;

  return ll;
}

std::string KvBaseApp::getConfFile(const std::string &ifNotSetReturn) {
  if (confFile.empty())
    return ifNotSetReturn;

  return confFile;
}

void KvBaseApp::setConfFile(const std::string &filename) {
  confFile = filename;
}

namespace {
ConfSection*
confLoader(const std::string &defaultConfFile) {
  ConfParser parser;
  string conffile;
  ConfSection *conf;
  ifstream fis;

  conffile = KvBaseApp::getConfFile(defaultConfFile);

  if (!conffile.empty() && conffile[0] != '/')
    conffile = kvPath("sysconfdir") + "/" + KvBaseApp::getConfFile();

  fis.open(conffile.c_str());

  if (!fis) {
    LOGERROR("Cant open the configuration file <" << conffile << ">!" << endl);
  } else {
    LOGINFO("Reading configuration from file <" << conffile << ">!" << endl);
    conf = parser.parse(fis);

    if (!conf) {
      LOGERROR(
          "Error while reading configuration file: <" << conffile << ">!" << endl << parser.getError() << endl);
    } else {
      LOGINFO("Configuration file loaded!\n");
      return conf;
    }
  }

  return 0;
}

std::string getAppName(const std::string &progname) {
  std::string name(progname);
  std::string::size_type i;

  i = name.find_last_of("/");

  if (i != string::npos)
    name.erase(0, i + 1);

  i = name.find_first_of('.');

  if (i != string::npos)
    name.erase(i);

  return name;
}

}
