/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: DataList.cc,v 1.5.2.5 2007/09/27 09:02:22 paule Exp $                                                       

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
#include "DataList.h"


DataListEntry& 
DataListEntry::operator=(const DataListEntry &de)
{
  if(this!=&de){
    obsTime_=de.obsTime_;
    dataList=de.dataList;
  }
  return *this;
}

DataListEntry::TDataList 
DataListEntry::
getTypeId(int typeID)const
{
  CITTypeList it=dataList.find(typeID);
 
  if(it!=dataList.end())
    return it->second;
  
  return TDataList();
}

int 
DataListEntry::
size(int typeID)const
{
  CITTypeList it=dataList.find(typeID);
  
  if(it!=dataList.end())
    return it->second.size();
  
  return 0;
}


int 
DataListEntry::
size()const
{
  CITTypeList it=dataList.begin();
  int s=0;

  for(;it!=dataList.end(); it++)
    s+=it->second.size();
  
  return s;
}


/**
 * add, put the Data, d, in a list with data of typeID.
 * If this is the first data that is added to the entry
 * the obsTime is set to be the obstime of data.obstime().
 * All subsecuent adds must have the same obstime, if not an 
 * TimeErrorException is raised. 
 *
 * \return true if the data was successful added. false
 *         if there allready is a element of data in the entry.
 *
 * \exception TimeError if the obstime field dont match
 *            the obsTime_ field of this entry.
 */
void 
DataListEntry::
add(const Data &d)
{
  ITTypeList it;
  ITDataList dit;

  if(obsTime_.undef()){
    obsTime_=d.obstime();
  }else if(obsTime_!=d.obstime()){
    throw TimeError();
  }

  it=dataList.find(d.typeID());

  if(it!=dataList.end()){
    for(dit=it->second.begin();
	dit!=it->second.end();
	dit++){
      if(d.paramID() == dit->paramID() &&
	 d.sensor()  == dit->sensor()  &&
	 d.level()   == dit->level())
	break;
    }
     
    if(dit!=it->second.end()){
      *dit=d;
    }else{
      it->second.push_back(d);
    };
  }else{
    dataList[d.typeID()].push_back(d);
  }
}


std::ostream& 
operator<<(std::ostream& ost, const DataListEntry& dl)
{
	ost << "[" << dl.obstime() << ":";
	
	for(DataListEntry::CITTypeList it=dl.dataList.begin();
		 it!=dl.dataList.end();
		 it++)
		ost <<" " << it->first << "/" << it->second.size();
		
	ost << "]";
	
	return ost;
}
  		

DataEntryList::
DataEntryList()
{
}

DataEntryList::
DataEntryList(const DataEntryList &d)
  :dataList(d.dataList)
{
}

DataEntryList::
~DataEntryList()
{
}
  
DataEntryList& 
DataEntryList::
operator=(const DataEntryList &rhs)
{
  if(&rhs!=this){
    dataList=rhs.dataList;
  }

  return *this;
}


/**
 * Insert Data at timeIndex in the list. If the date (d),
 * exist it is replaced.
 *
 * \param d the Data to insert at timeIndex.
 * \exception TimeError if the obstime field dont match
 *            the obsTime_ field of this entry.
 */
void 
DataEntryList::
insert(const Data &d)
{
  ITDataEntryList it=dataList.begin();

  for(;it!=dataList.end(); it++){
    if(it->obstime()<=d.obstime())
      break;
  }
  
  if(it==dataList.end()){
    DataListEntry de(d.obstime());
    de.add(d);
    dataList.push_back(de);
  }else if(it->obstime()==d.obstime()){
     it->add(d);
  }else{
    DataListEntry de(d.obstime());
    de.add(d);
    it=dataList.insert(it, de);
  }
}



DataEntryList::ITDataEntryList  
DataEntryList::
find(const miutil::miTime &from)
{
  ITDataEntryList it=dataList.begin();

  if(from.undef())
    return dataList.end();

  for(;it!=dataList.end(); it++){
    if(it->obstime()<=from)
      return it;
  }

  return dataList.end();
}

DataEntryList::CITDataEntryList 
DataEntryList::
find(const miutil::miTime &from)const
{
  CITDataEntryList it=dataList.begin();

  if(from.undef())
    return dataList.end();

  for(;it!=dataList.end(); it++){
    if(it->obstime()<=from)
      return it;
  }

  return dataList.end();
}

int 
DataEntryList::
nContinuesTimes()const
{
  CITDataEntryList it=dataList.begin();
  miutil::miTime  prevT;
  miutil::miTime  testT;
  int             n;
  
  if(it==dataList.end())
    return 0;
  
  n=1;
  prevT=it->obstime();
  it++;

  for(;it!=dataList.end(); it++){
    testT=it->obstime();
    testT.addHour(1);

    if(testT!=prevT){
      break;
    }else{
      prevT=it->obstime();
      n++;
    }
  }

  return n;
}


bool 
DataEntryList::
hasContinuesTimes(const std::list<int> &ctList, int hours)const
{
  CITDataEntryList it=dataList.begin();
  miutil::miTime  prevT;
  miutil::miTime  testT;
  int             n;
  
  if(it==dataList.end())
    return false;
  
  n=1;
  prevT=it->obstime();
  it++;

  for(;it!=dataList.end(); it++){
    testT=it->obstime();
    testT.addHour(1);

    if(testT!=prevT){
      break;
    }else{
      std::list<int>::const_iterator ctit=ctList.begin();
      DataListEntry::TDataList  dl;
      bool                      ok=true;

      for(;ok && ctit!=ctList.end(); ctit++){
	dl=it->getTypeId(*ctit);

	//If dl is empty -> No datalement for this typeid at this obstime.
	if(dl.empty())
	  ok=false;
      }

      if(!ok)
	break;

      prevT=it->obstime();
      n++;
    }
  }

  return n>=hours;
}


std::ostream&
operator<<( std::ostream& ost,
		    const DataEntryList& dl)
{
	for( DataEntryList::CITDataEntryList it=dl.begin(); it != dl.end(); ++it ) {
		ost << *it << std::endl;
	}

	return ost;
}
