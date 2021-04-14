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
#ifndef __kvpullApp_h__
#define __kvpullApp_h__

#include <kvdb/kvdb.h>
#include <kvdb/dbdrivermgr.h>
#include <kvalobs/kvapp.h>
#include <puTools/miTime.h>
#include <list>
#include <string>
#include <exception>
#include <kvcpp/KvApp.h>


class GetOptEx : public std::exception {
  std::string msg;

 public:
  explicit GetOptEx(const char *msg_)
      : msg(msg_) {
  }
  explicit GetOptEx(const std::string &msg_)
      : msg(msg_) {
  }
  virtual ~GetOptEx() throw () {
  }
  const char* what() const throw () {
    return msg.c_str();
  }
};

struct Options {
  typedef std::list<int> List;
  typedef List::iterator IList;
  typedef List::const_iterator CIList;

  Options()
      : help(false),
        doQa(false),
        totime(miutil::miTime::nowTime()) {
  }

  bool help;
  bool doQa;
  List stations;
  List typeids;
  miutil::miTime fromtime;
  miutil::miTime totime;
};

std::ostream& operator<<(std::ostream &ost, const Options &opt);

class KvPullApp : public kvservice::KvApp {
  std::string dbConnect;
  std::string dbDriverId;

  /**
   * \exception GetOptEx
   */
  void readIntList(const char *opt, Options::List &list);

  /**
   * \exception GetOptEx
   */
  void readDate(const char *opt, miutil::miTime &date);

 public:
  KvPullApp(int argn, char **argv, const char *options[0][2] = 0);
  ~KvPullApp();
  /**
   * \exception GetOptEx
   */
  void getOpt(int argn, char **argv, Options &opt);

  dnmi::db::Connection *getNewDbConnection();
  void releaseDbConnection(dnmi::db::Connection *con);
};

#endif
