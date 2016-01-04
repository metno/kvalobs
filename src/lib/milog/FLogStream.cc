/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: FLogStream.cc,v 1.7.6.3 2007/09/27 09:02:31 paule Exp $                                                       

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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h> 
#include <ctype.h>
#include <milog/FLogStream.h>
#include <milog/StdLayout.h>
#include <set>
#include <sstream>
#include <iostream>
using namespace std;

namespace {

/**
 * splitFileName splitter filename opp i en path komponent og
 * et filnavn. Hvis filename ikke er en absolutt sti til filenavnet
 * brukes funksjonen getcwd for � forme path.
 */
bool
splitFileName(const std::string &filename, std::string &fname,
              std::string &path);

bool
checkFormat(const char *buf, const char *name, const char *ext);
typedef std::set<std::string, std::less<std::string> > TDirSet;
typedef std::set<std::string, std::less<std::string> >::iterator ITDirSet;
}

void milog::FLogStream::write(const std::string &message) {
  if (currentSize_ > maxSize_)
    logRotate();

  if (fd) {
    fwrite(message.c_str(), message.length(), 1, fd);
    fflush(fd);
    currentSize_ += message.length();
  }
}

milog::FLogStream::FLogStream()
    : nRotate_(1),
      maxSize_(100 * 1024),
      fd(0) {
  try {
    StdLayout *l = new StdLayout();
    layout(l);
  } catch (...) {
  }
}

milog::FLogStream::FLogStream(int nRotate, int maxSize)
    : currentSize_(0),
      fd(0) {

  nRotate_ = nRotate < 1 ? 1 : nRotate;
  maxSize_ = maxSize < 102400 ? 102400 : maxSize;

  try {
    StdLayout *l = new StdLayout();
    layout(l);
  } catch (...) {
  }
}

milog::FLogStream::FLogStream(Layout *layout, int nRotate, int maxSize)
    : LogStream(layout),
      currentSize_(0),
      fd(0) {

  nRotate_ = nRotate < 1 ? 1 : nRotate;
  maxSize_ = maxSize < 102400 ? 102400 : maxSize;
}

milog::FLogStream::FLogStream(const std::string &fname, int nRotate,
                              int maxSize)
    : fname_(fname),
      fd(0) {

  nRotate_ = nRotate < 1 ? 1 : nRotate;
  maxSize_ = maxSize < 102400 ? 102400 : maxSize;

  if (open(fname)) {
    try {
      milog::StdLayout *l = new milog::StdLayout();
      layout(l);
    } catch (...) {
      fclose(fd);
    }
  }
}

milog::FLogStream::~FLogStream() {
  if (fd)
    fclose(fd);
}

bool milog::FLogStream::open(const std::string &fname) {
  string fileName;
  string pathName;
  string::size_type i;
  struct stat statbuf;

  if (fd) {
    fname_.erase();
    fclose(fd);
  }

  fd = fopen(fname.c_str(), "a");

  if (!fd)
    return false;

  if (stat(fname.c_str(), &statbuf) < 0) {
    currentSize_ = 0;
  } else {
    currentSize_ = statbuf.st_size;
  }

  fname_ = fname;
  return true;
}

void milog::FLogStream::close() {
  if (fd) {
    fclose(fd);
    fd = 0;
  }
}

bool milog::FLogStream::logRotate() {
  ostringstream ostNew;
  ostringstream ostOld;
  std::string fname(fname_);

  if (!fd)
    return false;

  if (fd == stderr)
    return true;

  ostNew << fname << "." << nRotate_;

  unlink(ostNew.str().c_str());

  for (int i = nRotate_; i > 1; i--) {
    ostNew.str("");
    ostOld.str("");
    ostNew << fname << "." << i;
    ostOld << fname << "." << i - 1;

    rename(ostOld.str().c_str(), ostNew.str().c_str());
  }

  fclose(fd);

  fd = 0;

  rename(fname.c_str(), string(fname + string(".1")).c_str());

  open(fname);

  return true;
}

namespace {
bool checkFormat(const char *buf, const char *name, const char *ext) {
  const char *p;
  int i;

  if (!buf || !name || !ext)
    return false;

  if (strncmp(buf, name, strlen(name)) != 0) {
    //    std::cout << "1\n";
    return false;
  }

  p = &buf[strlen(name)];

  if (*p == '\0' || *p != '_') {
    // std::cout << "2\n";
    return false;
  }

  p++;

  for (i = 0; *p != '\0' && i < 10 && isdigit(*p); i++, p++)
    ;

  if (i != 10) {
    // std::cout << "3\n";
    return false;
  }

  if (strcmp(p, ext) != 0) {
    //std::cout << "4\n";
    return false;
  }

  return true;
}

bool splitFileName(const std::string &filename, std::string &fname,
                   std::string &path) {
  std::string::size_type i;
  std::string::size_type start;
  char buf[PATH_MAX + 1];
  char *p;

  fname.erase();
  path.erase();

  if (filename.empty())
    return false;

  i = filename.find_last_of('/');

  if (i != string::npos) {
    fname = filename.substr(i + 1);
    path = filename.substr(0, i);
    path += "/";
  } else {
    fname = filename;
  }

  if (fname.empty())
    return false;

  if (fname == "." || fname == "..")
    return false;

  if (path.empty() || path[0] != '/') {
    p = getcwd(buf, PATH_MAX);
    buf[PATH_MAX] = '\0';

    if (p) {
      if (!path.empty())
        path.insert(0, "/");

      path.insert(0, buf);
    }
  }

  return true;
}
}
