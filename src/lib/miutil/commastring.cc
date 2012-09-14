/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: commastring.cc,v 1.5.2.3 2007/09/27 09:02:32 paule Exp $                                                       

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
#include <string>
#include <stdexcept>
#include <algorithm>
#include <sstream>
#include <string.h>
#include "commastring.h"
#include "trimstr.h"

#ifndef __DONT_USE_NAMESPACE__
using namespace std;
#endif



miutil::
CommaString::
CommaString(unsigned int antElement, char sep)    
    :data(antElement)
{
  char b[2];
  
  b[0]=sep;
  b[1]='\0';

  separator=b;
}

miutil::
CommaString::
CommaString(unsigned int antElement, const std::string &sep)
    :data(antElement), separator(sep)
{
}

miutil::
CommaString::
CommaString(const char *commaList, char sep)
    
{
  char b[2];
  
  b[0]=sep;
  b[1]='\0';

  separator=sep;
  init(commaList, separator[0]);
}

miutil::
CommaString::
CommaString(const char *commaList, const std::string &sep)
    :separator(sep)
{
  
  init(commaList, sep);
}


miutil::
CommaString::
CommaString(const std::string &commaList, char sep)
{
  char b[2];
  
  b[0]=sep;
  b[1]='\0';

  separator=b;

  init(commaList, separator[0]);
}


miutil::
CommaString::
CommaString(const std::string &commaList, const std::string &sep)
    :separator(sep)
{
  init(commaList, sep);
}

  
int   
miutil::
CommaString::
count(const std::string &str, char sep)
{
  string::const_iterator it;
  char                   ch=0;
  int                    cnt=0;
  bool                   inString=false;

  for(it=str.begin();it!=str.end(); it++){
    if(*it=='"' && ch!='\\')
      inString=!inString;
    else if(*it==sep && !inString)
      cnt++;

    ch=*it;
  }
  
  return cnt;
}

int   
miutil::
CommaString::
count(const std::string &str, const std::string &sep)
{
  string::size_type  i;
  int                cnt=0;
  
  if(sep.length()==1)
    return count(str, sep[0]);

  i=str.find(sep);

  while(i!=string::npos){
    cnt++;
    i+=sep.length();
    i=str.find(sep, i);
  }
  
  return cnt;
}



void 
miutil::
CommaString::
init(const std::string &str, char sep)
{
  ostringstream ost;
  char          ch=0;
  bool          inString=false;
  bool          isString=false;
  int i=0;
  int n=count(str, sep);
  
  data.resize(n+1);
  
  std::string::const_iterator it=str.begin();

  for(;it!=str.end(); it++){
    if(*it=='"' && ch!='\\'){
      ch=*it;
      isString=true;
      inString=!inString;
      continue;
    }
    
    if(inString){
      ost<<*it;
      continue;
    }
 
    if(*it!=sep){
      ost << *it;
    }else{
      isString=false;
      data[i]=Elem(ost.str(), isString);
      trimstr(data[i].data);
      i++;
      ost.str("");
    }
  }

  data[i]=Elem(ost.str(), isString);
  trimstr(data[i].data);
}

void 
miutil::
CommaString::
init(const std::string &str, const std::string &sep)
{
  ostringstream ost;
  string::size_type i, ii;
  int n=count(str, sep);
  int index=0;
  
  data.resize(n+1);
  
  ii=0;
  i=str.find(sep);

  while(i!=string::npos){
    data[index]=Elem(str.substr(ii, i-ii), false);
    index++;
    ii=i+sep.length();
    i=str.find(sep, ii);
  }

  data[index]=Elem(str.substr(ii), false);
}


miutil::CommaString& 
miutil::
CommaString::
operator=(const CommaString &s)
{
    if(this==&s)
	return *this;

    data=s.data;
    separator=s.separator;

    return *this;
}

/**
 * erase setter alle dataelementene i listen til tomme verdier.
 */
void
miutil::CommaString::erase()
{
  for(unsigned int i=0; i<data.size(); i++)
    data[i].erase();
}

bool 
miutil::
CommaString::
erase(unsigned int pos)
{
  if(pos>=data.size())
    return false;

  data[pos].erase();

  return true;
}


bool  
miutil::
CommaString::
insert(unsigned int index, const char *val)
{
    return insert(index, std::string(val));
}

bool  
miutil::
CommaString::
insert(unsigned int index, const std::string &val_)
{
  std::string val(val_);
  std::string::iterator it;

  if(index>=data.size())
    return false;

  if(separator.empty())
    return false;

  trimstr(val);
  
  if(val.length()>1 && val[0]=='"' && val[val.length()-1]=='"'){
      val.erase(0,1);
      val.erase(val.length()-1, 1);

      if(separator.length()==1)
	data[index]=Elem(val, true);
      else
	data[index]=Elem(val, false);
      
      return true;
  }
  
  if(separator.length()>1){
    data[index]=Elem(val, false);
    return true;
  }
    
  for(it=val.begin(); it!=val.end() && *it!=separator[0]; it++);

  if(it!=val.end()){
    data[index]=Elem(val, true);
    return true;
  }
    
  data[index]=Elem(val, false);
  
  return true;
}

 


bool  
miutil::
CommaString::
get(unsigned int index, std::string &str)const
{
    str.erase();
    
    if(index>=data.size())
	return false;
    
    str=data[index].data;

    return true;
}

bool  
miutil::
CommaString::
copy(char *str, int size)const
{
  bool first=true;
  string val;
 
  if(size<length())
    return false;

  for(int i=0; i<data.size(); i++){
    if(data[i].isString){
      val="\"\"";
      val.insert(1, data[i].data);
    }else{
      val=data[i].data;
    }
    
    
    if(first){
      first=false;
      strcpy(str, val.c_str());
    }else{
      strcat(str, separator.c_str()); 
      strcat(str, val.c_str());
    }
  }

  return true;
}

void 
miutil::
CommaString::
copy(std::string &str)const
{
  bool first=true;
  string val;
  ostringstream ost;

  for(int i=0; i<data.size(); i++){
    if(data[i].isString){
      val="\"\"";
      val.insert(1, data[i].data);
    }else{
      val=data[i].data;
    }
    
    if(first){
      first=false;
      ost << val;
    }else{
      ost << separator; 
      ost << val;
    }
  }

  str=ost.str();
}



std::ostream& 
miutil::CommaString::print(std::ostream &ostr)const
{
  bool first=true;
  string val;

  for(int i=0; i<data.size(); i++){
    if(data[i].isString){
      val="\"\"";
      val.insert(1, data[i].data);
    }else{
      val=data[i].data;
    }
    
    if(first){
      first=false;
      ostr<< val;
    }else
      ostr << separator << val;
  }
     
  return ostr;
}

int   
miutil::
CommaString::
length()const 
{
  if(data.size()==0)
    return 0;
  
  if(separator.empty())
    return -1;

  int n=(data.size()-1)*separator.length();

  for(int i=0; i<data.size(); i++){
    n+=data[i].data.length();
   
    if(data[i].isString)
      n+=2;
  }

  return n;
}

std::string& 
miutil::CommaString::operator[](const int index)
{
  if(index>=data.size())
    throw std::range_error("ERROR: Bad index!");

  return data[index].data;
}

const std::string& 
miutil::CommaString::operator[](const int index)const
{
  if(index>=data.size())
    throw std::range_error("ERROR: Bad index!");

  return data[index].data;
}


std::ostream& 
miutil::operator<<(std::ostream &os, const CommaString &val)
{
    return val.print(os);
}
