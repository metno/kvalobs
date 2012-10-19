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



#include <iostream>
#include "GetData.h"
#include "App.h"

using namespace std;


GetData::
GetData() 
{
}

bool 
GetData::
next( kvservice::KvObsDataList &datalist )
{
   cerr << "next: data from kvalobs\n";
  
   //it er en iterator. Ireratorer brukes for � traversere 
   //en container, i dette tilfellet KvDataList. KvDataList 
   //er en liste av kvData element.
   kvservice::IKvObsDataList it;
  
   for(it=datalist.begin(); 
       it!=datalist.end();  
       it++){               

      if(it->dataList().size()>0){
         cout << "stationID: " << it->stationid() 
              << " obstime: " << it->dataList().front().obstime()
              << " parameters: " << it->dataList().size() << endl;
      }
    
      App::printObsDataList( datalist );
      cerr << "--------------------------\n";
   }
   
   return true;
}
