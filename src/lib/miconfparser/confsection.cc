/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: confsection.cc,v 1.5.2.2 2007/09/27 09:02:25 paule Exp $                                                       

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
#include <iostream>
#include <string>
#include <miconfparser/trimstr.h>
#include <miconfparser/confsection.h>

using std::string;
using std::endl;

miutil::conf::ConfSection::
ConfSection()
{
}

miutil::conf::ConfSection::
~ConfSection()
{
  ISectionList it=sectionList.begin();
  
  //  std::cerr << "DELETE.....\n";
  
  for(;it!=sectionList.end(); it++)
    delete it->second;

  //std::cerr << "DELETE.....return\n";
  
}


bool 
miutil::conf::ConfSection::
addSection(const std::string &name, 
	       ConfSection *cs,
	       bool replaceIfExist)
{
  ISectionList it=sectionList.find(name);
  
  if(!cs)
    return false;

  if(it!=sectionList.end()){
    if(replaceIfExist){
      delete it->second;
      it->second=cs;
      return true;
    }else
      return false;
  }

  sectionList[name]=cs;
  return true;
}

bool 
miutil::conf::ConfSection::
addValue(const std::string &key, 
	 const ValElementList &value,
	 bool  replaceIfExist)
{
  IValueList it=valueList.find(key);

  if(it!=valueList.end()){
    if(!replaceIfExist){
      return false;
    }
  }

  valueList[key]=value;
  return true;
}

bool 
miutil::conf::ConfSection::
addValue(const std::string &key,
	 const ValElement &value,
	 bool  replaceIfExist)
{
  IValueList it=valueList.find(key);

  if(it!=valueList.end()){
    if(!replaceIfExist){
      return false;
    }else{
      it->second.clear();
      it->second.push_back(value);
    }
  }

  ValElementList elem;
  elem.push_back(value);
  valueList[key]=elem;

  return true;

}
      
void 
miutil::conf::ConfSection::
appendValue(const std::string &key,
		 const ValElement &value)
{
  IValueList it=valueList.find(key);

  if(it!=valueList.end()){
    it->second.push_back(value);
  }else{
    ValElementList elem;
    elem.push_back(value);
    valueList[key]=elem;
  }
}


miutil::conf::ConfSection*  
miutil::conf::ConfSection::
getSection(const std::string &cs_)
{
  string            cs(cs_);
  string            sectionName;
  ConfSection       *section=this;
  string::size_type i;
  string::size_type iPrev=0;
  CISectionList     it;

  trimstr(cs, " \n\r\t.");
  
  if(cs.empty())
    return this;
    

  while(true){
    i=cs.find(".", iPrev);
    
    if(i!=string::npos){
      sectionName=cs.substr(iPrev, i-iPrev);
    }else{
      sectionName=cs.substr(iPrev);
    }

    if(sectionName.empty()){
      if(section==this)
	return 0;
      else
	return section;
    }
   
    it=section->sectionList.find(sectionName);
	
    if(it==section->sectionList.end()){
      return 0;
    }else{
      section=it->second;
    }
    
    if(i!=string::npos)
      iPrev=i+1;
    else
      return section;
  }
}

miutil::conf::ValElementList 
miutil::conf::ConfSection::
getValue(const std::string &key_)const
{
  string            key(key_);
  string            sectionPart;
  string            keyPart;
  string::size_type i;
  ConfSection *section;
  CIValueList       it;
  trimstr(key, " \n\r\t.");

  if(key.empty())
    return ValElementList();

  i=key.find_last_of(".");
  
  if(i==string::npos)
    keyPart=key;
  else{
    keyPart=key.substr(i+1);
    sectionPart=key.substr(0, i);
  }

  section=const_cast<ConfSection *>(this)->getSection(sectionPart);

  if(!section)
    return ValElementList();

  it=section->valueList.find(keyPart);
  
  if(it==section->valueList.end())
    return ValElementList();

  return it->second;
}

std::list<string> 
miutil::conf::ConfSection::
getKeys()const
{
  std::list<std::string> list;
  
  for(CIValueList it=valueList.begin();
      it!=valueList.end();
      it++)
    list.push_back(it->first);
  
  return list;
}

std::list<string> 
miutil::conf::ConfSection::
getSubSections()const
{
  std::list<std::string> list;
  
  for(CISectionList it=sectionList.begin();
      it!=sectionList.end();
      it++)
    list.push_back(it->first);

  return list;
}


void 
miutil::conf::ConfSection::
printImple(std::ostream &ost, 
	   int          nSpace, 
	   bool         pritty)const
{
  CISectionList     itSec;
  CIValueList       itVal;
  string            space((pritty?nSpace:0), ' ');
  
  //  ost << "--- nSpace=" << nSpace << " pritty=" << (pritty?"true":"false") 
  //    << endl;

  for(itVal =valueList.begin();
      itVal!=valueList.end(); 
      itVal++){
    ost << space << itVal->first << "=" << itVal->second << endl;
  }
    
  itSec=sectionList.begin();
  
  if(pritty && itSec!=sectionList.end())
    ost << endl;


  while(itSec!=sectionList.end()){
    ost << space << itSec->first << "{" << endl;
    itSec->second->printImple(ost, nSpace+3, true);
    ost  << space << "}" << endl;
    itSec++;
    
    if(pritty && itSec!=sectionList.end())
      ost<< endl;
  }
}

void
miutil::conf::ConfSection::
print(std::ostream &ost, bool pritty)const
{
  printImple(ost, 0, pritty);
}

std::ostream& 
miutil::conf::
operator<<(std::ostream &ost, const miutil::conf::ConfSection &cs)
{
  cs.print(ost, true);
  return ost;
}



