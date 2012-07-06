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

#ifndef __DataHelper_h__
#define __DataHelper_h__


#include <map>
#include <list>
#include <set>
#include <iostream>
#include <kvcpp/kvevents.h>


typedef std::map<int, std::string> ParamDefs;
typedef boost::shared_ptr<ParamDefs> ParamDefsPtr;

struct Param {
   int paramid;
   int sensor;
   int level;

   Param(const Param &p );

   Param(const kvalobs::kvData &data );
   Param( const kvalobs::kvTextData &textData );

   bool isTextData()const;
   Param operator=(const Param &rhs );
   bool operator<(const Param &rhs )const;
   bool operator==(const Param &rhs )const;
};

std::ostream&
operator<<( std::ostream &out, const Param &param );



class ObsData {
public:
   typedef std::map<miutil::miTime, std::map<Param, float > > DataList;
   typedef std::map<miutil::miTime, std::map<Param, std::string> > TextDataList;
   typedef std::set<Param > Params;
   typedef std::set<miutil::miTime> Times;

private:
   int stationid;
   int type;
   DataList data;
   TextDataList textData;
   Params header;
   Times obsTimes;

   std::string createHeader( ParamDefsPtr paramdefs );
   std::string getData( const Param &param, const miutil::miTime &obstime )const;

public:
   ObsData();
   ObsData( int stationid, int typeid_ );

   bool add( const kvalobs::kvData &data );
   bool add( const kvalobs::kvTextData &data );

   void klData( std::string &data, std::string &decoder,
                ParamDefsPtr paramdefs );

   bool operator<(const ObsData &rhs )const;
   friend std::ostream& operator<<( std::ostream &out, const ObsData &od);
};

std::ostream& operator<<( std::ostream &out, const ObsData &od);

typedef std::map<int, std::map<int, ObsData> > ObsDataList;


class DataHelper {
   ObsDataList obsData;
   std::list<ObsData*> dataList;
   std::list<ObsData*>::iterator itNext;

public:
   DataHelper();

   void addData( kvservice::KvObsDataListPtr obsData );
   void initNext();

   bool nextData( std::string &data, std::string &decoder, ParamDefsPtr paramdefs );
   friend std::ostream& operator<<( std::ostream &out, const DataHelper &od);
};

#endif
