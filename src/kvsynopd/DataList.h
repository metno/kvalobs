/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: DataList.h,v 1.3.6.6 2007/09/27 09:02:22 paule Exp $                                                       

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
#ifndef __DataList_h__
#define __DataList_h__

#include <list>
#include <map>
#include <puTools/miTime.h>
#include <exception>
#include "Data.h"


class DataListEntry{
public:
	typedef std::list<Data>                   TDataList;
	typedef std::list<Data>::iterator        ITDataList;
	typedef std::list<Data>::const_iterator CITDataList;

	typedef std::map<int, TDataList>                    TTypeList;
	typedef std::map<int, TDataList>::iterator         ITTypeList;
	typedef std::map<int, TDataList>::const_iterator  CITTypeList;

	class TimeError : public std::exception{
	public:
		TimeError()throw(){};
		~TimeError()throw(){}
		const char* what()const throw() { return "Timestamp exist!";}
	};



private:
	miutil::miTime obsTime_;
	TTypeList      dataList;

public:
	DataListEntry(){}

	DataListEntry(const DataListEntry &de)
	:obsTime_(de.obsTime_), dataList(de.dataList){}

	DataListEntry(const miutil::miTime &t)
	:obsTime_(t){}

	~DataListEntry(){}

	DataListEntry& operator=(const DataListEntry &de);

	miutil::miTime obstime()const{ return obsTime_;}

	/**
	 * Returns the list of Data elements for typeID.
	 */
	TDataList getTypeId(int typeID)const;

	/**
	 * Return all typeID's for this list.
	 */
	std::list<int> getTypes()const;


	/**
	 * size, returns the number of Data elements for typeID.
	 */
	int size(int typeID)const;


	/**
	 * Returns the total number of elements in this DetaListEntry.
	 */
	int size()const;

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
	void add(const Data &d);

	friend std::ostream& operator<<(std::ostream& ost,
			const DataListEntry& dl);

};


class DataEntryList{
public:
	typedef std::list<DataListEntry>                   TDataEntryList;
	typedef std::list<DataListEntry>::iterator        ITDataEntryList;
	typedef std::list<DataListEntry>::const_iterator CITDataEntryList;

private:
	TDataEntryList dataList;

public:
	DataEntryList();
	DataEntryList(const DataEntryList &d);
	~DataEntryList();

	DataEntryList& operator=(const DataEntryList &rhs);

	void clear(){ dataList.clear();}


	/**
	 * Number of continues times from the obstime this data set represent.
	 */
	int nContinuesTimes()const;

	/**
	 * \brief check if we have continues times for the typeids in the
	 *        list \a ctList.
	 *
	 * \param ctList check continues times for the typeid in this list.
	 * \param hours return true if we have \a hours with continues times
	 *              for all types in \a ctList. False otherwise.
	 */
	bool hasContinuesTimes(const std::list<int> &ctList, int hours)const;


	/**
	 * Insert Data at timeIndex (d.obstime()) in the list. If the data (d),
	 * exist it is replaced.
	 *
	 * \param timeIndex, the time the Data (d) is for.
	 * \param d the Data to insert at timeIndex.
	 */
	void insert(const Data &d);

	int  size()const { return dataList.size();}

	ITDataEntryList  find(const miutil::miTime &from);
	CITDataEntryList find(const miutil::miTime &from)const;

	ITDataEntryList  begin(){ return dataList.begin();}
	CITDataEntryList begin()const{ return dataList.begin();}

	ITDataEntryList  end(){ return dataList.end();}
	CITDataEntryList end()const { return dataList.end();}

	friend std::ostream& operator<<(std::ostream& ost,
			const DataEntryList& sd);
};




std::ostream& 
operator<<(std::ostream& ost,
			   const DataListEntry& dl);


std::ostream& 
operator<<(std::ostream& ost,
 		      const DataEntryList& dl);


#endif
