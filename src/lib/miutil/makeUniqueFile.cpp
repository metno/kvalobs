/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: kvPath.h,v 1.1.2.2 2007/09/27 09:02:30 paule Exp $

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
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <chrono>
#include <sstream>
#include <boost/filesystem.hpp>
#include "makeUniqueFile.h"

namespace fs = boost::filesystem;


namespace {

std::string timestamp() {
  struct tm stm;
  auto now=std::chrono::system_clock::now();
  std::time_t t = std::chrono::system_clock::to_time_t(now);
  gmtime_r(&t, &stm);
  char timeString[256];
  std::strftime(timeString, 256, "%Y%m%dT%H%M%S", &stm);
  return timeString;
}

}



namespace miutil {

std::string makeUniqueFile(const std::string &prefix, const std::string &endsWith, const std::string &dir) {
  auto ts = timestamp();
  fs::path p(prefix);

  if( p.is_relative() ) {
    fs::path  d(dir);
    p=d/p;
  }

  fs::path d=p.parent_path();

  try {
    fs::create_directories(d);
  }
  catch( const std::exception &e){
    std::ostringstream o;
    o << "Failed to create directory '" << d << "'. " << e.what();
    throw std::runtime_error(o.str());
  }

  auto fname=p.string();
  std::ostringstream o;
  int fd;

  for( int i=0; i<10000; ++i) {
    o.str("");
    o << fname << "_" << ts << "_" << i << endsWith;
    auto file=o.str();
    fd=open(file.c_str(), O_WRONLY|O_CREAT|O_EXCL, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    if( fd<0 ) {
      if( errno != EEXIST ) {
        const int BUF_SIZE = 512;
        char msg[BUF_SIZE];
        auto ignored_return_value_=strerror_r(errno, msg, BUF_SIZE);
        msg[BUF_SIZE-1]='\0';
        std::ostringstream o;
        o << "Failed to creat unique file '" << file << "'. " << msg;
        throw std::runtime_error(o.str());
      }
      continue;
    }
    close(fd);
    return file;
  }
  o.str("");
  o << "Failed to creat unique file '" << fname << "*"<< endsWith << "'. " << o.str();
  throw std::runtime_error(o.str());
}
} 
