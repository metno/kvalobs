/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id$                                                       

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


#include <map>
#include <set>
#include <iostream>
#include <boost/algorithm/string.hpp>
#include <kvcpp/kvevents.h>
#include "DataHelper.h"

using namespace std;
using namespace miutil;
using namespace std;
using namespace kvservice;


Param::
Param(const Param &p )
   : paramid( p.paramid ), sensor( p.sensor ), level( p.level )
{
}

Param::
Param(const kvalobs::kvData &data )
   : paramid( data.paramID() ),
     sensor( data.sensor()-'0' ),
     level( data.level() )
{
}

Param::
Param( const kvalobs::kvTextData &textData )
   : paramid( textData.paramID() ),
     sensor( INT_MAX ),
     level( INT_MAX )
{
}

bool
Param::
isTextData()const
{
   return sensor == INT_MAX && level == INT_MAX;
}

Param
Param::
operator=(const Param &rhs )
{
   if( &rhs != this ) {
      paramid = rhs.paramid;
      sensor = rhs.sensor;
      level = rhs.level;
   }
   return *this;
}


bool
Param::
operator<(const Param &rhs )const
{

   if( paramid < rhs.paramid ||
       ( paramid == rhs.paramid && sensor < rhs.sensor) ||
       ( paramid == rhs.paramid && sensor == rhs.sensor && level < rhs.level )
     ){
      return true;
   }else {
      return false;
   }
}


bool
Param::
operator==(const Param &rhs )const
{
   if( paramid == rhs.paramid &&
       sensor == rhs.sensor &&
       level == rhs.level ){
      return true;
   }else {
      return false;
   }
}


std::ostream&
operator<<( std::ostream &out, const Param &param )
{
   out << "[" << param.paramid << "," << param.sensor << ","
       << param.level << "]";
   return out;
}




ObsData::
ObsData()
   :stationid( INT_MAX ), type( INT_MAX )
{
}

ObsData::
ObsData( int stationid, int typeid_ )
   : stationid( stationid ), type( type )
{
}

bool
ObsData::
add( const kvalobs::kvData &kvData )
{
   if( stationid == INT_MAX ) {
      stationid = kvData.stationID();
      type = kvData.typeID();
   } else if( stationid != kvData.stationID() ||
              type != kvData.typeID() ) {
      return false;
   }

   if( kvData.cfailed().find("hqc") != string::npos ) {
      hqcTimes.insert( kvData.obstime() );
      return true;
   }

   data[kvData.obstime()][Param(kvData)]=kvData.original();
   header.insert( Param( kvData ) );
   obsTimes.insert( kvData.obstime() );

   return true;
}

bool
ObsData::
add( const kvalobs::kvTextData &kvData )
{
   if( stationid == INT_MAX ) {
      stationid = kvData.stationID();
      type = kvData.typeID();
   } else if( stationid != kvData.stationID() ||
              type != kvData.typeID() ) {
      return false;
   }

   if( hqcTimes.find( kvData.obstime() ) != hqcTimes.end() ) {
      return true;
   }

   textData[kvData.obstime()][Param(kvData)] = kvData.original();
   header.insert( Param( kvData ) );
   obsTimes.insert( kvData.obstime() );

   return true;
}

std::string
ObsData::
createHeader( ParamDefsPtr paramdefs )
{
   ostringstream ost;
   bool first=true;

   for( Params::iterator it=header.begin(); it != header.end(); ++it ){
      ParamDefs::iterator hit = paramdefs->find( it->paramid );

      //Remove element that we do NOT recognize.
      if( hit == paramdefs->end() ) {
         header.erase( it );
         continue;
      }

      if( first ) {
         first = false;
      } else {
         ost << ",";
      }

      ost << hit->second;

      if( ! it->isTextData() && (it->sensor != 0 || it->level != 0) ) {
         ost << "(" << it->sensor << "," << it->level << ")";
      }
   }

   //No params?
   if( first )
      return "";

   ost << endl;
   return ost.str();
}

std::string
ObsData::
getData( const Param &param, const miutil::miTime &obstime )const
{
   if( param.isTextData() ) {
      TextDataList::const_iterator obstimeIt = textData.find( obstime );

      if( obstimeIt == textData.end() )
         return "";

      std::map<Param, std::string>::const_iterator paramIt;
      paramIt = obstimeIt->second.find( param );

      if( paramIt == obstimeIt->second.end() )
         return "";

      string data=paramIt->second;
      string tmp = boost::replace_all_copy( data, ",", "\\,");

      if( tmp != data )
         return "\"" + tmp +"\"";
      else
         return data;
   } else {
      DataList::const_iterator obstimeIt = data.find( obstime );

      if( obstimeIt == data.end() )
         return "";

      std::map<Param, float>::const_iterator paramIt;
      paramIt = obstimeIt->second.find( param );

      if( paramIt == obstimeIt->second.end() )
         return "";

      ostringstream ost;
      ost << paramIt->second;
      return ost.str();
   }
}

void
ObsData::
klData( std::string &data, std::string &decoder,
        ParamDefsPtr paramdefs )
{
   string headerString = createHeader( paramdefs );

   data.clear();
   decoder.clear();

   if( headerString.empty() ) {
      return;
   }

   ostringstream ost;
   int count=0;
   int nParams;
   ost << headerString;

   for( Times::iterator it=obsTimes.begin(); it != obsTimes.end(); ++it ) {
      //We do not generate a message if the hqc has been involved.
      if( hqcTimes.find( *it ) != hqcTimes.end() )
         continue;

      nParams = 0;
      for( Params::iterator pit = header.begin(); pit != header.end(); ++pit ) {
         if( nParams == 0 )
            ost << *it;

         ost << ",";

         ++nParams;
         ost << getData( *pit, *it );
      }

      if( nParams > 0 ) {
         ost << endl;
         ++count;
      }
   }

   if( count > 0 ) {
      data = ost.str();
      ost.str("");
      ost << "kldata/nationalnr=" << stationid <<"/type=" << type;
      decoder = ost.str();
   }
}

bool
ObsData::
operator<(const ObsData &rhs )const
{
   if( stationid < rhs.stationid && type < rhs.type )
      return true;
   else
      return false;
}

std::ostream&
operator<<( std::ostream &out, const ObsData &od)
{
   out << "[(";
   for( ObsData::Params::const_iterator hit=od.header.begin();
        hit != od.header.end(); ++hit) {
      if( hit != od.header.begin() )
         out << ",";
      out << *hit;
   }
   out<< ")" << endl;
   out << "(";
   for( ObsData::DataList::const_iterator dit=od.data.begin();
        dit != od.data.end(); ++dit ) {
      out << dit->first;
      for(std::map<Param, float>::const_iterator pit=dit->second.begin();
            pit != dit->second.end(); ++pit ) {
         out << ","<<pit->first << ":"<<pit->second;
      }
      out << endl;
   }
   out << ")"  << endl;
   out << "(";
   for( ObsData::TextDataList::const_iterator dit=od.textData.begin();
         dit != od.textData.end(); ++dit ) {
      out << dit->first;
      for(std::map<Param, string>::const_iterator pit=dit->second.begin();
            pit != dit->second.end(); ++pit ) {
         out << ","<<pit->first << ":"<<pit->second;
      }
      out << endl;
   }
   out << ")"  << endl;



}

DataHelper::
DataHelper()
   : itNext( dataList.end() )
{
}

void
DataHelper::
addData( KvObsDataListPtr obs )
{
   for( KvObsDataList::iterator it = obs->begin();
        it != obs->end(); ++it ) {
      for( KvObsData::kvDataList::iterator dit=it->dataList().begin();
           dit != it->dataList().end(); ++dit ) {
         if( dit->typeID() < 0 || dit->original() <= -32766 )
            continue;

         obsData[dit->stationID()][dit->typeID()].add( *dit );
      }

      for( KvObsData::kvTextDataList::iterator dit=it->textDataList().begin();
           dit != it->textDataList().end(); ++dit ) {
         obsData[dit->stationID()][dit->typeID()].add( *dit );
      }
   }
   initNext();
}


void
DataHelper::
initNext()
{
   //Index the obsData to make it easy to get the encoded data

   dataList.clear();

   for( ObsDataList::iterator it=obsData.begin();
         it != obsData.end(); ++it ) {
      for( map<int, ObsData>::iterator itt = it->second.begin();
            itt != it->second.end(); ++itt )
         dataList.push_back( &itt->second );
   }

   itNext = dataList.begin();
}



bool
DataHelper::
nextData( string &data, string &decoder, ParamDefsPtr paramdefs  )
{
   while( itNext != dataList.end() ) {
      (*itNext)->klData( data, decoder, paramdefs );
      ++itNext;

      if( ! data.empty() )
         return true;
      cerr << "DataHelper::nextData: empty data." << endl;
   }

   return false;
}

std::ostream&
operator<<( std::ostream &out, const DataHelper &dh )
{
   out << "------ [BEGIN DataHelper] ---------------" << endl;
   for( ObsDataList::const_iterator sit = dh.obsData.begin();
        sit != dh.obsData.end(); ++sit ) {
      for( map<int, ObsData>::const_iterator tit=sit->second.begin();
           tit != sit->second.end(); ++tit ) {
         out << tit->second << endl;
         out << "-------------------------------------" << endl;
      }

   }
   out << "------ [END DataHelper] ---------------" << endl;
}
