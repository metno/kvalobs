/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: valelement.h,v 1.1.2.2 2007/09/27 09:02:25 paule Exp $

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
#ifndef __miutil_conf_valelement_h__
#define __miutil_conf_valelement_h__

#include <iosfwd>
#include <list>
#include <miconfparser/confexception.h>
#include <stdexcept>
#include <string>

namespace miutil {
namespace conf {

bool setListChars(const std::string &listCharLeftRight="()");

//2 elements string, where element[0] is opening character and element[1] is closing
//chracters. The result may also be used as the emtty list.
std::string getListChars();

/**
 * \addtogroup miconfparser
 *
 * @{
 */

class ValElementList;

enum ValType
{
  UNDEF = 0,
  ID,
  INT,
  FLOAT,
  STRING,
  NIL
};

/**
 * \brief ValElement represent the value in a key=value.
 */
class ValElement
{
  ValType valType_;
  std::string val_;

public:
  ValElement()
    : valType_(UNDEF)
  {}

  ValElement(const ValElement& v)
    : valType_(v.valType_)
    , val_(v.val_)
  {}

  ValElement(const std::string& val);
  ValElement(const std::string& val, ValType vt);

  ValElement(long l);

  ValElement(double d);

  ~ValElement() {}

  ValElement& operator=(const ValElement& v)
  {
    if (this != &v) {
      valType_ = v.valType_;
      val_ = v.val_;
    }

    return *this;
  }

  bool isNil() const { valType_ == NIL;}
  void setAsNil();
  bool undef() const { return valType_ == UNDEF; }

  bool isEqual(const ValElement& v) const
  {
    if ((valType_ == v.valType_) && (val_ == v.val_))
      return true;

    return false;
  }

  ValType type() const { return valType_; }

  void val(const std::string& val);

  void val(long val);

  void val(double val);

  /**
   *
   *
   */
  std::string valAsString() const { return val_; }

  /**
   * return the value as an long. Throws InvalidTypeEx
   * if the value is not an float or integer. If it is an
   * float the value returned is rounded to it nearest integer
   *
   * @exception InvalidTypeEx
   */

  long valAsInt() const;

  long valAsInt(long defaultValue) const;

  /**
   * return the value as an bool. Throws InvalidTypeEx
   * if the value is not a bool. Bool is the values true and false.
   *
   * @exception InvalidTypeEx
   */

  bool valAsBool() const;
  bool valAsBool(bool defaultValue) const;

  /**
   * return the value as an float. Throws InvalidTypeEx
   * if the value is not an float or integer.
   *
   * @exception InvalidTypeEx
   */

  double valAsFloat() const;
  double valAsFloat(double deefaultValue) const;

  /**
   * return the value as an string. May throw  UndefEx exception.
   * If 'quoted' is true the string is quoted, ie "string".
   * If the valuetype is STRING and there is  quota's in the string,
   * it will be escaped with a backslash '\'.
   *
   * @param quoted If true the string is quoted, ie it will be
   *               started with the quota '"' the character and ended
   *               with the quota character '"'.
   * @exception UndefEx
   */
  std::string toString(bool quoted = false) const;

  bool operator==(const ValElement& a) const { return isEqual(a); }

  friend std::ostream& operator<<(std::ostream& ost, const ValElement& elem);

  // friend std::ostream&
  // operator<<(std::ostream &ost, const ValElementList &elemList);
};

std::ostream&
operator<<(std::ostream& ost, const ValElement& elem);

/**
 * \brief ValElementList is a list of ValElement.
 */
class ValElementList : protected std::list<ValElement>
{
  bool isList_;
  bool isNil_;
  
public:
  typedef std::list<ValElement>::iterator iterator;
  typedef std::list<ValElement>::const_iterator const_iterator;

  ValElementList():isList_(false), isNil_(true) {}


  ValElementList(double val):isList_(false), isNil_(false) { push_back(ValElement(val)); }

  ValElementList(long val):isList_(false), isNil_(false) { push_back(ValElement(val)); }

  ValElementList(const std::string& val):isList_(false), isNil_(false) { push_back(ValElement(val)); }

  ValElementList(const ValElementList& ve)
    : std::list<ValElement>(ve), isList_(ve.isList_), isNil_(ve.isNil_)
  {
  }

  bool empty() const {
    return std::list<ValElement>::empty();
  }
  
  size_type size() const noexcept{
    return std::list<ValElement>::size();
  }

  void clear() {
    std::list<ValElement>::clear();
  }

  void push_back (const std::list<ValElement>::value_type& val){
    isNil_=false;

    if( isList_ || empty()) {
      std::list<ValElement>::push_back(val);
    } else {
      //Replace the element.
     *begin() = val; 
    }
  }

  void push_front (const std::list<ValElement>::value_type& val){
    isNil_=false;

    if( isList_ || empty()) {
      std::list<ValElement>::push_front(val);
    } else {
      //Replace the element.
     *begin() = val; 
    }
    
  }
  iterator begin() noexcept {
    return std::list<ValElement>::begin();
  }

  const_iterator begin() const noexcept{
    return std::list<ValElement>::begin();
  }

  iterator end() noexcept {
    return std::list<ValElement>::end();
  }

  const_iterator end() const noexcept {
    return std::list<ValElement>::end();
  }

  reference front() {
    return std::list<ValElement>::front();
  }
  const_reference front() const{
    return std::list<ValElement>::front();
  }

  ValElementList& operator=(const ValElementList& ve)
  {
    if (this != &ve) {
      isList_=ve.isList_;
      isNil_ = ve.isNil_; 
      std::list<ValElement>::operator=(ve);
    }
    return *this;
  }

  void isList(bool v) {
    if (size() > 1 ) {
      //Ignoring v (this is a list).
      isList_=true;
      return;
    }
    isList_=v;
  } 

  bool isList()const{ 
      if(size() > 1 )
        return true;
      return isList_;
  }

  bool isNil()const{ return isNil_;}
  void setAsNil() {
    isList_=false;
    isNil_=true;
    clear();
  }

  
  std::string valAsString(const std::string& defaultVal = "",
                          int index = 0) const;
  long valAsInt(long defaultValue, int index = 0) const;
  bool valAsBool(bool defaultValue, int index = 0) const;
  double valAsFloat(double deefaultValue, int ondex = 0) const;

  /**
   * \excetion std::out_of_range if 'index' is is not in range
   *           [0, size()>.
   */
  ValElement& operator[](const int index);

  

  friend std::ostream& operator<<(std::ostream& ost,
                                  const ValElementList& elemList);
};

typedef ValElementList::iterator IValElementList;
typedef ValElementList::const_iterator CIValElementList;

std::ostream&
operator<<(std::ostream& ost, const ValElementList& elemList);

/** @} */
}
}

#endif
