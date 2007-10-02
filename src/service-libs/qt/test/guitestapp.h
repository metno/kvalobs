/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: guitestapp.h,v 1.1.6.1 2007/09/27 09:02:47 paule Exp $                                                       

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
#ifndef __guitestapp_h__
#define __guitestapp_h__

#include <string>
#include <map>
#include <list>
#include <kvQtApp.h>
#include <kvservicetypes.h>


typedef std::map<long, std::string>                   IdMap;
typedef std::map<long, std::string>::iterator        IIdMap;
typedef std::map<long, std::string>::const_iterator CIIdMap;

class Data{
  kvservice::KvDataList data_;
  std::string           stationName_;

 public:
  Data(const kvservice::KvDataList &dataList,
       const std::string &stationName)
    :data_(dataList),stationName_(stationName)
    {
    }

  Data(const Data &d)
    :data_(d.data_), stationName_(d.stationName_)
    {
    }

  Data& operator=(const Data &d){
    if(this!=&d){
      data_=d.data_;
      stationName_=d.stationName_;
    }
    
    return *this;
  }

  kvservice::KvDataList& data() { return data_;}
  std::string            stationName()const{ return stationName_;}
};


typedef std::list<Data>                  DataList;
typedef std::list<Data>::iterator       IDataList;
typedef std::list<Data>::const_iterator CIDataList;


class GUITestApp : public kvservice::KvQtApp
{
  Q_OBJECT;

  const static int maxData;
  IdMap stations;
  IdMap params;
  DataList    dataList;
  std::string subidKvHint;
  std::string subidKvData;
  
 public:
  GUITestApp(int argn, char **argv);
  ~GUITestApp();
  
  std::string station(long sid)const;
  std::string param(long pid)const;


  public slots:
    void kvHint(bool);
    void newData(kvservice::KvObsDataListPtr);
    
 signals:
    void newData(DataList&);
};

#endif
