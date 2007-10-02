/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: StationInfoParse.h,v 1.5.6.2 2007/09/27 09:02:23 paule Exp $                                                       

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
#ifndef __StationInfoParse_h__
#define __StationInfoParse_h__

#include <iostream>
#include <miconfparser/miconfparser.h>
#include <milog/milog.h>
#include <string>
#include <list>
#include "StationInfo.h"

//class StationInfo;
//StationInfo::TLongList;


class StationInfoParse
{
  struct DefaultVal{
    std::string copyto;
    bool        copy;
    std::string owner;
    std::list<std::string> precipitation;
    std::string list;
    milog::LogLevel loglevel;

    StationInfo::TDelayList delay; 
    
    DefaultVal(): copy(false), loglevel(milog::INFO) {
    }

    DefaultVal(const DefaultVal &dv)
      :copyto(dv.copyto), copy(dv.copy),owner(dv.owner),
	 precipitation(dv.precipitation), list(dv.list),
	 loglevel(dv.loglevel)
    {
    }
    
    DefaultVal& operator=(const DefaultVal &dv){
      if(&dv!=this){
	copyto=dv.copyto; 
	copy=dv.copy;
	owner=dv.owner;
	precipitation=dv.precipitation;
	list=dv.list;
	delay=dv.delay;
	loglevel=dv.loglevel;
      }
      return *this;
    }

    bool valid()const;
   };
   
   bool doWmoDefault(miutil::conf::ConfSection *stationConf);

   std::string doDefList(miutil::conf::ValElementList &vl,
			 int wmono);

   std::string doDefOwner(miutil::conf::ValElementList &vl,
			  int wmono);
   
   std::list<std::string> doDefPrecip(miutil::conf::ValElementList &vl,
				      int wmono);

   milog::LogLevel doDefLogLevel(miutil::conf::ValElementList &vl,
				 int wmono);

   bool doDefCopy(miutil::conf::ValElementList &vl,
		  int wmono);

   std::string doDefCopyto(miutil::conf::ValElementList &vl,
			   int wmono);

   StationInfo::TDelayList doDefDelay(miutil::conf::ValElementList &vl, 
				      int wmono);
   
   bool doStationid(const std::string &key,
		    miutil::conf::ValElementList &vl, 
		    StationInfo &st);
   
   bool doDelay(const std::string &key,
		miutil::conf::ValElementList &vl, 
		StationInfo &st);
   

   bool doPrecip(const std::string &key,
		 miutil::conf::ValElementList &vl, 
		 StationInfo &st);
  
  
   bool doTypePri(const std::string &key,
		  miutil::conf::ValElementList &vl, 
		  StationInfo &st);


   StationInfo* parseSection(miutil::conf::ConfSection *stationConf, 
			     int wmono);
   
   DefaultVal defVal;

 public:
  StationInfoParse(){}
  ~StationInfoParse(){}
  
  bool parse(miutil::conf::ConfSection *stationConf,
	     std::list<StationInfoPtr> &stationList);
  
};


#endif

