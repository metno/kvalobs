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
#include <algorithm>
#include <miconfparser/trimstr.h>
#include <miconfparser/confsection.h>

using std::string;
using std::endl;

namespace miutil {
namespace conf {

std::map<const ConfSection*,ConfSectionPimpel*> ConfSection::pimpel;

class ConfSectionPimpel {
public:
   bool allowMultipleSections;
   int lineno;
   std::string filename;
   bool ignore;

   ConfSectionPimpel( bool allowMultipleSections_,
                      int lineno_=0,
                      const std::string &filename_="" )
      : allowMultipleSections( allowMultipleSections_),
        lineno( lineno_ ), filename( filename_ ), ignore(false) {}
};
}
}

miutil::conf::
ConfSection::
ConfSection()
{
   pimpel[this] = new ConfSectionPimpel( false );
//   std::cerr << "ConfSection: CTOR(): allowMultipleSections: "
//             << (allowMultipleSections?"true":"false")<< std::endl;
}

miutil::conf::
ConfSection::
ConfSection( bool allowMultipleSections_,
             const std::string &filename_ ,
             int lineno_ )
{
   pimpel[this] = new ConfSectionPimpel( allowMultipleSections_,
                                         lineno_, filename_ );
//   std::cerr << "ConfSection: CTOR( bool ): allowMultipleSections: "
//             << (allowMultipleSections?"true":"false")<< std::endl;
}

miutil::conf::
ConfSection::
~ConfSection()
{
   std::map<const ConfSection*,ConfSectionPimpel*>::iterator pit;

   pit = pimpel.find( this );
   if( pit != pimpel.end() ) {
      delete pit->second;
      pimpel.erase( pit );
   }

   ISectionList it=sectionList.begin();

   //  std::cerr << "DELETE.....\n";

   for(;it!=sectionList.end(); it++) {
      for( ConfSectionList::iterator cit=it->second.begin();
            cit != it->second.end(); ++cit )
         delete *cit;
   }

   //std::cerr << "DELETE.....return\n";

}

bool
miutil::conf::
ConfSection::
ignoreThisSection()const
{
    std::map<const ConfSection*,ConfSectionPimpel*>::iterator pit;

    pit = pimpel.find( this );
    return pit!=pimpel.end()?pit->second->ignore:false;
}

void
miutil::conf::
ConfSection::
ignoreThisSection( bool f )
{
    std::map<const ConfSection*,ConfSectionPimpel*>::iterator pit;

    pit = pimpel.find( this );
    if( pit != pimpel.end() )
        pit->second->ignore = f;
}



int
miutil::conf::
ConfSection::
getLineno()const
{
   std::map<const ConfSection*,ConfSectionPimpel*>::iterator pit;

   pit = pimpel.find( this );
   return pit!=pimpel.end()?pit->second->lineno:0;
}

std::string
miutil::conf::ConfSection::
getFilename()const
{
   std::map<const ConfSection*,ConfSectionPimpel*>::iterator pit;

   pit = pimpel.find( this );
   return pit!=pimpel.end()?pit->second->filename:"";
}

bool
miutil::conf::
ConfSection::
allowMultipleSections()const
{
   std::map<const ConfSection*,ConfSectionPimpel*>::iterator pit;

   pit = pimpel.find( this );
   return pit!=pimpel.end()?pit->second->allowMultipleSections:false;
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
         for( std::list<ConfSection*>::iterator cit=it->second.begin();
               cit != it->second.end(); ++cit )
            delete *cit;
         it->second.clear();
         it->second.push_back( cs );
         return true;
      }else if( allowMultipleSections() ) {
         it->second.push_back( cs );
         return true;
      } else {
         std::cerr << "addSection: allowMultipleSections: " << (allowMultipleSections()?"true":"false") << endl;
         return false;
      }
   }

   sectionList[name].push_back( cs );
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
         if( !it->second.empty() )
            section=*it->second.begin();
      }

      if(i!=string::npos)
         iPrev=i+1;
      else
         return section;
   }
}

miutil::conf::ConfSectionList
miutil::conf::ConfSection::
getAllSection(const std::string &cf_)
{
   string            cs(cf_);
   string            sectionName;
   string            curSectionNamePath;
   ConfSection*      section=this;
   ConfSectionList   sectionList;
   string::size_type i;
   string::size_type iPrev=0;
   CISectionList     it;

   trimstr(cs, " \n\r\t.");

   if(cs.empty())
      return ConfSectionList();


   while(true){
      i=cs.find(".", iPrev);

      if(i!=string::npos){
         sectionName=cs.substr(iPrev, i-iPrev);
      }else{
         sectionName=cs.substr(iPrev);
      }

      if( curSectionNamePath.empty() )
         curSectionNamePath = sectionName;
      else if( ! sectionName.empty() )
         curSectionNamePath += "." + sectionName;

      if(sectionName.empty()){
         if(section==this)
            return ConfSectionList();
         else
            return sectionList;
      }

      it=section->sectionList.find(sectionName);

      if(it==section->sectionList.end()){
         return ConfSectionList();
      }else{
         if( !it->second.empty() ) {
            section=*it->second.begin();
            sectionList = it->second;
         }
      }


      if(i!=string::npos)
         iPrev=i+1;
      else
         return sectionList;
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

miutil::conf::ValElementList
miutil::conf::
ConfSection::
getValueRecursivt( const std::string &startInSection,
                   const std::string &key,
                   std::string &foundKeyInSection,
                   const ValElementList &defValue )const
{
    string::size_type pos;
    ValElementList val;
    std::string myKey;
    string section = startInSection;
    int n = count( section.begin(), section.end(), '.');

    if( n != 0 || ! section.empty() )
        ++n;

    for( int i=n; i >= 0; --i ) {
        if( section.empty() )
            myKey = key;
        else
            myKey = section + "." + key;

        val = getValue( myKey );

        if( ! val.empty()  ) {
            foundKeyInSection = section;
            return val;
        }

        pos = section.find_last_of( '.' );

        if( pos == string::npos )
            section.erase();
        else
            section.erase( pos );
    }

    return defValue;
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
deleteAllIgnoredSections()
{
    ISectionList     itSec = sectionList.begin();

    while( itSec != sectionList.end()){
        ConfSectionList::iterator cit = itSec->second.begin();

        while( cit != itSec->second.end() ) {
           if( (*cit)->ignoreThisSection() ) {
               delete *cit;
               cit = itSec->second.erase( cit );
           } else {
               (*cit)->deleteAllIgnoredSections();
               ++cit;
           }
        }

        SectionList::iterator itTmp=itSec;
        ++itSec;
        if( itTmp->second.empty() ) {
            sectionList.erase( itTmp );
        }
     }
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
      for( ConfSectionList::const_iterator cit = itSec->second.begin();
           cit != itSec->second.end(); ++cit ) {

         ost << space << itSec->first << "{" << endl;
         (*cit)->printImple(ost, nSpace+3, true);
         ost  << space << "}" << endl;
      }

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




