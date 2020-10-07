
/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: kvpushApp.h,v 1.1.2.2 2007/09/27 09:02:48 paule Exp $                                                       

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
#ifndef __Option_h__
#define __Option_h__

#include <string>
#include <stdexcept>
#include <list>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <miconfparser/miconfparser.h>

struct Options {
  typedef std::list<int> List;
  typedef List::iterator IList;
  typedef List::const_iterator CIList;

  Options()
      : doQa(false), doSubscribe(false) {
  }

  bool doQa;
  bool doSubscribe;
  List stations;
  List typeids;
  std::shared_ptr<miutil::conf::ConfSection> conf; 
  boost::posix_time::ptime fromtime;
  boost::posix_time::ptime totime;
};

void ParseOpt(int argn, char **argv, Options *opt);

void Use(int exitcode, const std::string &msg="");

std::ostream& operator<<(std::ostream &ost, const Options &opt);

#endif