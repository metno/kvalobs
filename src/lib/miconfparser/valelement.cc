/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: valelement.cc,v 1.7.6.2 2007/09/27 09:02:25 paule Exp $                                                       

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
#include <ctype.h>
#include <math.h>
#include <sstream>
#include <stdio.h>
#include <sstream>
#include <boost/lexical_cast.hpp>
#include <miconfparser/trimstr.h>
#include <miconfparser/valelement.h>

using namespace std;

namespace {
  void compress(std::string &str);
}


miutil::conf::ValElement::
ValElement(long l)
  :valType_(INT), val_(""){
  char tmp[128];
  
  sprintf(tmp, "%ld", l);
  val_=tmp;
}

miutil::conf::ValElement::
ValElement(double d)
  :valType_(FLOAT), val_(""){
  char tmp[256];
  sprintf(tmp, "%f", d);
  val_=tmp;
}
	
miutil::conf::ValElement::
ValElement(const std::string &val)
  :valType_(STRING), val_(val){
  trimstr(val_);
  compress(val_);
}


miutil::conf::ValElement::
ValElement(const std::string &val, ValType type)
  :valType_(type), val_(val){
  trimstr(val_);
  compress(val_);
}



long
miutil::conf::ValElement::
valAsInt()const
{
  long l=0;

  if(valType_==INT){
    if( sscanf(val_.c_str(), "%ld", &l) != 1 )
        throw InvalidTypeEx();
  }else if(valType_==FLOAT){
    double d=0.0;
    sscanf(val_.c_str(), "%lf", &d);
    d+=0.5;
    l=static_cast<long>(floor(d));
  }else {
    try {
        l = boost::lexical_cast<long>( val_ );
    }catch( const boost::bad_lexical_cast &ex ) {
        throw InvalidTypeEx();
    }
  }
  
  return l;

}

long
miutil::conf::ValElement::
valAsInt( long defaultValue )const
{
    try {
        return valAsInt();
    }
    catch (  ... ) {
        return defaultValue;
    }
}

double
miutil::conf::ValElement::
valAsFloat()const
{
  double d=0;

  if(valType_==INT){
    long l=0;
    if( sscanf(val_.c_str(), "%ld", &l) != 1 )
        throw InvalidTypeEx();
    d=static_cast<double>(l);
  }else if(valType_==FLOAT){
    sscanf(val_.c_str(), "%lf", &d);
  }else{
      try {
          d = boost::lexical_cast<double>( val_ );
      }catch( const boost::bad_lexical_cast &ex ) {
          throw InvalidTypeEx();
      }
  }

  return d;
}

double
miutil::conf::ValElement::
valAsFloat( double defaultValue )const
{
    try {
        return valAsFloat();
    }
    catch (  ... ) {
        return defaultValue;
    }
}


std::string 
miutil::conf::ValElement::
toString(bool quoted)const
{
  if(valType_==UNDEF)
    throw UndefEx();
  
  if(valType_!=STRING && !quoted){
    return val_;
  }

  string val=val_;

  if(valType_==STRING){
    string::size_type i;
    
    i=val.find("\"");
    
    while(i!=string::npos){
      val.insert(i, "\\");
      i+=2;
      i=val.find("\"", i);
    }
  }

  if(valType_==STRING && !quoted){
    string::size_type i;
        
    if(!val.empty()){
      for(i=0; i<val.length(); i++){
	if(ispunct(val[i]) || val[i]==' '){
	  quoted=true;
	  break;
	}
      }
	  
      if(!quoted){
	if(isalpha(val[0]) || val[0]=='_')
	  return val;
      }

      quoted=true;
    }
  }
    

  if(quoted){
    val.insert(0, "\"");
    val.append("\"");
  }

  return val;

}

	
void 
miutil::conf::ValElement::
val(long val)
{ 
  char buf[128];
  valType_=INT;
  sprintf(buf, "%ld", val);
  val_=buf;
}

void 
miutil::conf::ValElement::
val(double val)
{ 
  char buf[128];
  
  valType_=FLOAT;
  sprintf(buf, "%f", val);
  val_=buf;
}

void 
miutil::conf::ValElement::
val(const std::string &val){ 
  valType_=STRING;
  val_=val;
  trimstr(val_);
  compress(val_);
}


miutil::conf::ValElement& 
miutil::conf::ValElementList::
operator[](const int index)
{

  if(index<0 || index>=this->size()){
    ostringstream ost;
    ost << "Index (" << index << ") must be in range [0," << size() << ">";
    throw std::out_of_range(ost.str());
  }
 
  std::list<ValElement>::iterator it=begin();

  for(int i=0; it!=end(); i++, it++){
    if(i==index)
      return *it;
  }

  //Should never happend!!!

  throw std::out_of_range("Index out of range!");
}

 


std::ostream& 
miutil::conf::
operator<<(std::ostream &ost, const ValElementList &elemList)
{
  CIValElementList it=elemList.begin();

  if(elemList.empty()){
    ost << "<<<EMPTY>>>";
    return ost;
  }
    
  if(elemList.size()>1){
    ost << "(" << *it;
    it++;

    for(;it!=elemList.end(); it++)
      ost << "," << *it;
    
    ost << ")";
  }else
    ost << *it;

  return ost;
}



std::ostream& 
miutil::conf::
operator<<(std::ostream &ost, const miutil::conf::ValElement &elem)
{
  try{
    ost << elem.toString();
  }
  catch(...){
    ost << "<<<UNDEF>>>";
  }
  
  return ost;
}


namespace {
  void compress(std::string &str)
  {
    string::size_type i;
    
    miutil::conf::trimstr(str);
    i=str.find_first_of("\n\t\r");
    
    while(i!=string::npos){
      str[i]=' ';
      i=str.find_first_of("\n\t\r", i+1);
    }
    
    i=str.find(" ");

    while(i!=string::npos){
      i++;
      
      while(i<str.length() && str[i]==' ')
	str.erase(i, 1);

      if(i>=str.length())
	break;
      
      i=str.find(" ", i);
    }
  }
}

      

