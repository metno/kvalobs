/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvDataIteratorImpl.h,v 1.2.6.4 2007/09/27 09:02:39 paule Exp $                                                       

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
#ifndef __kvDataIteratorImpl_h__
#define __kvDataIteratorImpl_h__

#include <time.h>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <kvService.hh>
#include <kvdb/kvdb.h>
#include <list>
#include <kvalobs/kvData.h>
#include <kvalobs/kvTextData.h>
#include "ServiceApp.h"
#include "ReaperBase.h"

class DataIteratorImpl: 
	public POA_CKvalObs::CService::DataIterator,
	public PortableServer::RefCountServantBase,
	public ReaperBase{
  
  class InvalidWhichData : public std::exception{
    std::string reason;
  public:
    explicit  InvalidWhichData(const std::string &reason_):reason(reason_){}
    virtual ~ InvalidWhichData()throw(){}
    
    const char *what()const throw(){ return reason.c_str();}
  };

  dnmi::db::Connection              *dbCon;
  CKvalObs::CService::WhichDataList *whichData;
  CORBA::Long                       iData;
  miutil::miTime                    currentEndTime;
  miutil::miTime                    endTime;
  miutil::miTime                    startTimeOfGetData;
  ServiceApp                        &app;
  boost::mutex                      mutex;

  /**
   * filterData use the status field in wData (WhichData) to 
   * decide if we shall return a data record. wData must be the same
   * as is used to get the data out of the database with the
   * function findData.
   *
   * It takes a kvData list and an iterator into the list for
   * the first element we shall check against, in addition 
   * to WhichData.
   *
   * It returns an iterator to the first element that match the status
   * field in wData. The search includes the start element.
   *
   * It return dataList.end()  when the end of dataList is reached.
   *
   * \param start an iterator to the first element to test against.
   * \param dataList the dataList that holds the element to filter.
   * \param wData holds the filter criteria field status.
   *
   * \return an iterator to an kvData that match the statusfield in wData or
   *         the dataList.end() iterator.
   */
  std::list<kvalobs::kvData>::iterator
    filterData(std::list<kvalobs::kvData>::iterator start,
	       std::list<kvalobs::kvData> &dataList,
	       const CKvalObs::CService::WhichData &wData);

  bool findData(std::list<kvalobs::kvData> &data, 
		std::list<kvalobs::kvTextData> &textData,
		const CKvalObs::CService::WhichData &wData);

  void insertTextData(CKvalObs::CService::ObsDataList *obsDataList, 
		      const CKvalObs::CService::TextDataElemList  &textData);


public:
  DataIteratorImpl(dnmi::db::Connection *dbCon,
		   CKvalObs::CService::WhichDataList *whichData,
		   ServiceApp                        &app_);

  virtual ~DataIteratorImpl();

  void destroy();
  CORBA::Boolean next(CKvalObs::CService::ObsDataList_out obsData_);
  
  ///Overrided from reaperBase.
  virtual void cleanUp();
  virtual void addRef() { _add_ref(); }
  virtual void removeRef(){ _remove_ref(); }
  
};

#endif
