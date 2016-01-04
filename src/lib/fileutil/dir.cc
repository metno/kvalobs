/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: dir.cc,v 1.4.2.2 2007/09/27 09:02:28 paule Exp $                                                       

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
#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/stat.h>
#include <unistd.h>
#include "dir.h"
#include "file.h"

namespace {
bool match(const std::string &ma, const std::string &pat, bool casesensitive =
               true);
bool lowerch(char c1, char c2, bool casesensitive);
}

dnmi::file::Dir::Dir(const std::string &dirname_)
    : dir(0),
      entry(0),
      casesensitive(true) {
  open(dirname_);
}

dnmi::file::Dir::~Dir() {
  if (dir)
    closedir(dir);
}

bool dnmi::file::Dir::open(const std::string &dirname_, const std::string &p,
                           bool cs) {
  DIR *tmp;

  dirname = dirname_;

  while (!dirname.empty() && dirname[0] == ' ')
    dirname.erase(0, 1);

  while (!dirname.empty()
      && (dirname[dirname.length() - 1] == ' '
          || dirname[dirname.length() - 1] == '/'))
    dirname.erase(dirname.length() - 1);

  if (dirname == ".")
    dirname.erase();

  casesensitive = cs;

  if (dirname.empty())
    tmp = opendir(".");
  else
    tmp = opendir(dirname.c_str());

  if (!tmp) {
    char *s = strerror(errno);

    if (!s)
      err = "Uknown error!";
    else
      err = s;

    return false;
  }

  if (dir) {
    closedir(dir);
  }

  pattern = p;
  entry = 0;
  dir = tmp;
  err.erase();

  return true;

}

bool dnmi::file::Dir::hasNext() {
  if (!dir)
    throw DirException("No directory is open!");

  entry = readdir(dir);

  if (!pattern.empty()) {
    while (entry && !match(entry->d_name, pattern, casesensitive))
      entry = readdir(dir);
  }

  return entry != 0;
}

std::string dnmi::file::Dir::next() {
  if (!entry) {
    if (!dir)
      throw DirException("No directory is open!");

    throw DirException("Past the last file in dierctory!");
  }

  return std::string(entry->d_name);
}

void dnmi::file::Dir::next(dnmi::file::File &file) {
  std::string name = next();

  if (dirname.empty())
    file = File(name);
  else
    file = File(dirname + "/" + name);

}

bool dnmi::file::Dir::rewind() {
  if (!dir)
    throw DirException("No directory is open!");

  rewinddir(dir);
}

bool dnmi::file::isDir(const std::string &name) {
  struct stat s;

  if (stat(name.c_str(), &s) < 0)
    return false;

  if (S_ISDIR(s.st_mode))
    return true;

  return false;
}

bool dnmi::file::isFile(const std::string &name) {
  struct stat s;

  if (stat(name.c_str(), &s) < 0)
    return false;

  if (S_ISREG(s.st_mode))
    return true;

  return false;
}

long dnmi::file::fileSize(const std::string &name) {
  struct stat s;

  if (stat(name.c_str(), &s) < 0)
    return -1;

  return s.st_size;
}

bool dnmi::file::canRead(const std::string &name) {
  if (access(name.c_str(), R_OK) == 0)
    return true;

  return false;
}

bool dnmi::file::canWrite(const std::string &name) {
  if (access(name.c_str(), W_OK) == 0)
    return true;

  return false;
}

time_t dnmi::file::modTime(const std::string &name) {
  struct stat s;

  if (stat(name.c_str(), &s) < 0)
    return -1;

  return s.st_mtime;
}

bool dnmi::file::fileFactory(const std::string &filename,
                             dnmi::file::File &file) {
  File r(filename);

  if (!r.ok())
    return false;

  file = r;
  return true;
}

dnmi::file::File*
dnmi::file::fileFactory(const std::string &filename) {
  File r(filename);

  if (!r.ok())
    return 0;

  return new File(r);
}

namespace {

bool lowerch(char c1, char c2, bool casesensitive) {
  if (casesensitive)
    return c1 == c2;

  return tolower(c1) == tolower(c2);
}

bool match(const std::string &ma, const std::string &pat, bool casesensitive) {
  std::string::const_iterator ip = pat.begin();
  std::string::const_iterator im = ma.begin();
  bool wild = false;
  bool wild1 = false;

  if (ip == pat.end())
    return true;

  if (im == ma.end())
    return false;

  while (true) {
    if (ip != pat.end() && *ip == '*') {
      wild = true;
      for (; ip != pat.end() && (*ip == '*' || *ip == '?'); ip++)
        ;
    } else if (ip != pat.end() && *ip == '?') {
      wild1 = true;
      ip++;
    }

    if (ip == pat.end()) {
      if (wild1 && im == ma.end())
        return false;
      else if (wild || im == ma.end())
        return true;
    } else if (im == ma.end()) {
      return false;
    }

    if (wild1) {
      wild1 = false;
    } else if (lowerch(*im, *ip, casesensitive)) {
      ip++;
      wild = false;
    } else if (!wild)
      return false;

    im++;
  }

  //Should newer happend!
  return false;
}
}
