/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: kvalobsdataparser.h,v 1.1.2.2 2007/09/27 09:02:27 paule Exp $                                                       

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
#ifndef KVALOBS_SERIALIZE_INTERNALKVALOBSDATAPARSER_H
#define KVALOBS_SERIALIZE_INTERNALKVALOBSDATAPARSER_H

#include <libxml++/parsers/saxparser.h>
#include <string>
//#include <stack>
#include <deque>
#include <map>
#include "kvalobsdata.h"

namespace kvalobs {

namespace serialize {

class Stack : public std::deque<std::string> {
 public:
  Stack() {
  }
  value_type &top() {
    return back();
  }
  void push(const value_type &v) {
    push_back(v);
  }
  void pop() {
    pop_back();
  }

  friend std::ostream & operator<<(std::ostream &o, const Stack &s);
};

/**
 * Parses data generated by \c KvalobsDataSerializer
 *
 * @author Vegard Bnes
 */
class KvalobsDataParser : protected xmlpp::SaxParser {
 public:
  static void parse(const std::string & xml, KvalobsData & d);

 protected:
  KvalobsDataParser(KvalobsData & d);

  virtual ~KvalobsDataParser();

  virtual void on_start_element(const Glib::ustring &name,
                                const AttributeList &attributes);
  virtual void on_end_element(const Glib::ustring &name);
  virtual void on_characters(const Glib::ustring & characters);
  std::ostream& printPath(std::ostream &o);
 private:
  void insertData();
  void insertTextData();

  KvalobsData & data_;

  typedef std::map<std::string, std::string> Context;
  Context context_;
  //std::deque<std::string> currentContext_;
  Stack currentContext_;
};

namespace internal {
typedef KvalobsDataParser KvalobsDataParser;
}

}
;

}
;

#endif
