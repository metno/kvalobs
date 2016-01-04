/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: confsection.h,v 1.1.2.2 2007/09/27 09:02:25 paule Exp $                                                       

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
#ifndef __miutil_conf_confsection_h__
#define __miutil_conf_confsection_h__

#include <iosfwd>
#include <string>
#include <map>
#include <list>
#include <miconfparser/valelement.h>

namespace miutil {
namespace conf {

class ConfSection;

/**
 * \addtogroup miconfparser
 *
 * @{
 */

typedef std::list<ConfSection*> ConfSectionList;
typedef std::map<std::string, ConfSectionList> SectionList;
typedef SectionList::iterator ISectionList;
typedef SectionList::const_iterator CISectionList;
typedef std::map<std::string, ValElementList> ValueList;
typedef ValueList::iterator IValueList;
typedef ValueList::const_iterator CIValueList;

///Private class to implement the pimpel pattern.
class ConfSectionPimpel;

/**
 * \brief ConfSection holds the parsed result.
 *
 */
class ConfSection {
  static std::map<const ConfSection*, ConfSectionPimpel*> pimpel;
  ConfSection(const ConfSection& cs);
  ConfSection& operator=(const ConfSection &rhs);

  void printImple(std::ostream &ost, int nSpace = 0, bool pritty = false) const;

  SectionList sectionList;
  ValueList valueList;

 public:
  ConfSection();
  ConfSection(bool allowMultipleSections, const std::string &filename = "",
              int lineno = 0);

  virtual ~ConfSection();

  int getLineno() const;
  std::string getFilename() const;
  bool allowMultipleSections() const;

  bool addSection(const std::string &name, ConfSection *cs,
                  bool replaceIfExist = true);

  bool addValue(const std::string &key, const ValElementList &value,
                bool replaceIfExist = true);

  bool addValue(const std::string &key, const ValElement &value,
                bool replaceIfExist = true);

  void appendValue(const std::string &key, const ValElement &value);

  ConfSection* getSection(const std::string &cf);
  ConfSectionList getAllSection(const std::string &cf);

  ValElementList getValue(const std::string &key) const;
  ValElementList getValueRecursivt(const std::string &startInSection,
                                   const std::string &key,
                                   std::string &foundKeInSection,
                                   const ValElementList &defValue =
                                       ValElementList()) const;

  std::list<std::string> getKeys() const;
  std::list<std::string> getSubSections() const;

  void print(std::ostream &ost, bool pritty) const;

  bool ignoreThisSection() const;
  void ignoreThisSection(bool f);

  void deleteAllIgnoredSections();

  friend std::ostream&
  operator<<(std::ostream &str, const miutil::conf::ConfSection &cs);

};

/** @} */
}
}

#endif
