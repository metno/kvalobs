/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvModelDataIteratorImpl.h,v 1.1.6.3 2007/09/27 09:02:39 paule Exp $                                                       

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
#ifndef __kvModelDataIteratorImpl_h__
#define __kvModelDataIteratorImpl_h__


#include <time.h>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <kvskel/kvService.hh>
#include <kvdb/kvdb.h>
#include <list>
#include <kvalobs/kvModelData.h>
#include "ServiceApp.h"
#include "ReaperBase.h"

class ModelDataIteratorImpl: public POA_CKvalObs::CService::ModelDataIterator,
			     public ReaperBase 
{
  
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
  ServiceApp                        &app;


  bool findData(std::list<kvalobs::kvModelData> &data, 
		const CKvalObs::CService::WhichData &wData);

public:
  ModelDataIteratorImpl(dnmi::db::Connection *dbCon,
		   CKvalObs::CService::WhichDataList *whichData,
		   ServiceApp                        &app_);

  virtual ~ModelDataIteratorImpl();


  void destroy();
  CORBA::Boolean next(CKvalObs::CService::ModelDataList_out modelData_);
  
};

#endif
