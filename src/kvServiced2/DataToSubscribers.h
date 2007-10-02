/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: DataToSubscribers.h,v 1.1.2.2 2007/09/27 09:02:22 paule Exp $                                                       

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
#ifndef __DataToSubscribers_h__
#define __DataToSubscribers_h__

#include <list>
#include <boost/shared_ptr.hpp>
#include <kvalobs/kvTextData.h>
#include <kvalobs/kvData.h>
#include <kvalobs/kvStationInfo.h>

/**
 * One DataToSubscribers record contains data for one obtime and stationid. 
 */
struct DataToSubscribers{
public:
  
  std::list<kvalobs::kvData>     dataList;
  std::list<kvalobs::kvTextData> textDataList;
  long                           stationid;
  int                            typeid_;
  miutil::miTime                 obstime;
  DataToSubscribers(){}
  DataToSubscribers(const std::list<kvalobs::kvData>     &dataList_,
		    const std::list<kvalobs::kvTextData> &textDataList_,
		    const kvalobs::kvStationInfo &si)
    :dataList(dataList_), 
     textDataList(textDataList_),
     stationid(si.stationID()),
     typeid_(si.typeID()),
     obstime(si.obstime())
    {};
  
  DataToSubscribers(const DataToSubscribers &d)
    :dataList(d.dataList), 
     textDataList(d.textDataList),
     stationid(d.stationid),
     typeid_(d.typeid_),
     obstime(d.obstime)
    {};
  

  DataToSubscribers& operator=(const DataToSubscribers &rhs){
    if(&rhs!=this){
      dataList=rhs.dataList;
      textDataList=rhs.textDataList;
      stationid=rhs.stationid;
      typeid_=rhs.typeid_;
      obstime=rhs.obstime;
    }
    return *this;
  }
};

typedef boost::shared_ptr<DataToSubscribers> DataToSubscribersPtr;

#endif
