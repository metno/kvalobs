/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: pidfileutil.cc,v 1.4.6.2 2007/09/27 09:02:28 paule Exp $                                                       

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
#include <sys/utsname.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <iostream>
#include "pidfileutil.h"
#include <stdio.h>

using namespace std;

std::string dnmi::file::createPidFileName(const std::string &path_,  
                                          const std::string &progname) {
  struct utsname buf;
  std::string path(path_);
  string nodename;
  string pidfilename;

  if (uname(&buf) == 0)
    nodename = buf.nodename;

  if( path.rbegin() != path.rend() && *path.rbegin() != '/')
     path += "/";
   
  pidfilename = path + progname;

  if (!nodename.empty())
    pidfilename += "-" + nodename;

  pidfilename += ".pid";

  return pidfilename;
}

bool dnmi::file::createPidFile(const std::string &pidfile) {
  int n = sizeof(pid_t);
  string buf;
  FILE *fd;

  if( n == sizeof(int) ) {
      buf = "%d\n";
  } else if( n == sizeof(long int) ) {
      buf = "%ld\n";
  } else if( n == sizeof(long long int) ) {
      buf = "%Ld\n";
  } else {
      cerr << "Size of pid_t = " << n
           << ", that does not match the size of an int, long int or long long int.\n\n.";
      fclose( fd );
      return false;
  }

  fd = fopen(pidfile.c_str(), "w");

  cerr << "Pidfile: " << pidfile << endl;

  if (!fd) {
    cerr << "Cant create pidfile\n";
    return false;
  }

  fprintf(fd, buf.c_str(), getpid());
  fclose(fd);

  return true;
}

bool dnmi::file::deletePidFile(const std::string &pidfile) {
  if (pidfile.empty())
    return false;

  if (unlink(pidfile.c_str()) < 0)
    return false;

  return true;
}

bool dnmi::file::readPidFromPidFile(const std::string &pidfile, pid_t &pid,
                                    bool &fileExist) {
  int n = sizeof(pid_t);
  string buf;
  FILE *fd = fopen(pidfile.c_str(), "r");

  fileExist = true;

  if (!fd) {
    fileExist = false;
    return false;
  }

  if (n == sizeof(int)) {
    buf = "%d";
  } else if (n == sizeof(long int)) {
    buf = "%ld";
  } else if (n == sizeof(long long int)) {
    buf = "%Ld";
  } else {
    cerr
        << "Size of pid_t = "
        << n
        << ", that does not match the size of an int, long int or long long int.\n\n.";
    fclose(fd);
    return false;
  }

  if (fscanf(fd, buf.c_str(), &pid) == 1) {
    fclose(fd);
    return true;
  }

  fclose(fd);

  return false;
}

bool dnmi::file::isRunningPidFile(const std::string &pidfile, bool &error) {
  pid_t pid;
  bool exist;

  error = false;

  if (readPidFromPidFile(pidfile, pid, exist)) {

    if (kill(pid, 0) == 0) {
      return true;
    } else if (errno == ESRCH) {
      unlink(pidfile.c_str());
      return false;
    } else if (errno == EPERM) {
      return true;
    } else {
      error = true;
      return true;
    }
  } else {
    if (exist) {
      error = true;
      return true;
    }

    return false;
  }
}

dnmi::file::PidFileHelper::PidFileHelper(const std::string &pidfile_) {
  createPidFile(pidfile_);
}

dnmi::file::PidFileHelper::~PidFileHelper() {
  if (!pidfile.empty())
    dnmi::file::deletePidFile(pidfile);
}

void dnmi::file::PidFileHelper::forget() {
  pidfile.erase();
}

bool dnmi::file::PidFileHelper::createPidFile(const std::string &pidfile_) {
  if (dnmi::file::createPidFile(pidfile_)) {
    pidfile = pidfile_;
    return true;
  }

  return false;
}
