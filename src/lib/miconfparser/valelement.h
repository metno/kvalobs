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
#include <string>
#include <list>
#include <stdexcept>
#include <miconfparser/confexception.h>

namespace miutil{
namespace conf{

/**
 * \addtogroup miconfparser
 *
 * @{
 */

class ValElementList;

enum ValType{UNDEF=0,INT, FLOAT, STRING};

/**
 * \brief ValElement represent the value in a key=value.
 */
class ValElement{
    ValType    valType_;
    std::string val_;

public:
    ValElement()
:valType_(UNDEF){
    }

    ValElement(const ValElement &v)
    :valType_(v.valType_), val_(v.val_){
    }

    ValElement(const std::string &val);
    ValElement(const std::string &val, ValType vt);

    ValElement(long l);

    ValElement(double d);

    ~ValElement(){
    }

    ValElement &operator=(const ValElement &v){
        if(this!=&v){
            valType_=v.valType_;
            val_=v.val_;
        }

        return *this;
    }


    bool undef()const{
        return valType_==UNDEF;
    }

    bool isEqual(const ValElement &v)const{
        if((valType_==v.valType_) &&
                (val_==v.val_))
            return true;

        return false;
    }

    ValType type()const{
        return valType_;
    }

    void val(const std::string &val);

    void val(long val);

    void val(double val);

    /**
     *
     *
     */
    std::string valAsString()const{
        return val_;
    }

    /**
     * return the value as an long. Throws InvalidTypeEx
     * if the value is not an float or integer. If it is an
     * float the value returned is rounded to it nearest integer
     *
     * @exception InvalidTypeEx
     */

    long valAsInt()const;

    long valAsInt( long defaultValue )const;

    /**
     * return the value as an bool. Throws InvalidTypeEx
     * if the value is not a bool. Bool is the values true and false.
     *
     * @exception InvalidTypeEx
     */

    bool valAsBool()const;
    bool valAsBool( bool defaultValue )const;

    /**
     * return the value as an float. Throws InvalidTypeEx
     * if the value is not an float or integer.
     *
     * @exception InvalidTypeEx
     */

    double valAsFloat()const;
    double valAsFloat( double deefaultValue )const;

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
    std::string toString(bool quoted=false )const;




    bool operator==(const ValElement &a)const{
        return isEqual(a);
    }

    friend std::ostream&
    operator<<(std::ostream &ost, const ValElement &elem);


    friend std::ostream&
    operator<<(std::ostream &ost, const ValElementList &elemList);

};

/**
 * \brief ValElementList is a list of ValElement.
 */
class ValElementList : public std::list<ValElement>{

public:
    ValElementList(){}

    ValElementList( double val )
    {
        push_back( ValElement( val ) );
    }

    ValElementList( long  val )
    {
        push_back( ValElement( val ) );
    }

    ValElementList( const std::string  &val )
    {
        push_back( ValElement( val ) );
    }

    ValElementList(const ValElementList &ve):
        std::list<ValElement>(ve){}

    ValElementList& operator=(const ValElementList &ve){
        if(this!=&ve){
            std::list<ValElement>::operator=(ve);
        }
        return *this;
    }

    std::string valAsString( const std::string &defaultVal="", int index=0 )const;
    long valAsInt( long defaultValue, int index=0 )const;
    bool valAsBool( bool defaultValue, int index=0 )const;
    double valAsFloat( double deefaultValue, int ondex=0 )const;



    /**
     * \excetion std::out_of_range if 'index' is is not in range
     *           [0, size()>.
     */
    ValElement& operator[](const int index);
};

typedef ValElementList::iterator        IValElementList;
typedef ValElementList::const_iterator  CIValElementList;

std::ostream& operator<<(std::ostream &ost, const ValElementList &elemList);

/** @} */
}

}

#endif
