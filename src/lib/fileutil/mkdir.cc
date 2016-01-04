/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: mkdir.cc,v 1.1.2.3 2007/09/27 09:02:28 paule Exp $                                                       

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
#include <errno.h>
#include <iostream>
#include "mkdir.h"

namespace {
bool mkdir_(const std::string &path);
}

using namespace std;

bool dnmi::file::mkdir(const std::string &newdir, const char *path_) {
  string path;

  if (path_)
    path = path_;

  return mkdir(newdir, path);
}

bool dnmi::file::mkdir(const std::string &newdir, const std::string &path_) {
  struct stat sbuf;
  string path(path_);
  string dirlist(newdir);
  string dir;

  while (!dirlist.empty() && dirlist[0] == '/')
    dirlist.erase(0, 1);

  if (dirlist.empty())
    return false;

  while (!path.empty() && path[path.length() - 1] == '/')
    path.erase(path.length() - 1);

  if (path.empty())
    path = ".";

  //We first check if the directory allready exist.	
  if (stat(string(path + "/" + dirlist).c_str(), &sbuf) == 0) {
    // cerr << "mkdir: Path (E): " << string(path+"/"+dirlist) ;

    if (S_ISDIR(sbuf.st_mode))
      return true;
    else
      return false;
  }

  if (stat(path.c_str(), &sbuf) != 0)
    return false;

  if (!S_ISDIR(sbuf.st_mode))
    return false;

  string::size_type i = dirlist.find("/");

  for (; i != string::npos; i = dirlist.find("/")) {
    dir = dirlist.substr(0, i);
    dirlist.erase(0, i + 1);

    if (dir.empty())
      continue;

    path += "/" + dir;

    if (!mkdir_(path))
      return false;
  }

  if (!dirlist.empty()) {
    path += "/" + dirlist;

    if (!mkdir_(path))
      return false;
  }

  return true;
}

namespace {
bool mkdir_(const std::string& path) {
  struct stat sbuf;

  //cerr << "mkdir: " << path << endl;

  if (mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0) {
    if (errno == EEXIST) {
      if (stat(path.c_str(), &sbuf) != 0)
        return false;

      if (!S_ISDIR(sbuf.st_mode))
        return false;
    } else {
      return false;
    }
  }

  return true;
}
}
