/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvDataNotifySubscriber.h,v 1.1.6.2 2007/09/27 09:02:39 paule Exp $                                                       

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
#ifndef __kvDataNotifySubscriber_h__
#define __kvDataNotifySubscriber_h__

#include <list>
#include <map>
#include <kvskel/kvService.hh>
#include "kvDataSubscriberInfo.h"
#include <boost/shared_ptr.hpp>
#include "kvSubscriberBase.h"


/**
 * KvDataSubscriber holds the information from a data subcriber.
 */ 



class KvDataNotifySubscriber: public KvSubscriberBase{

  //Definitions from kvService.hh (kvService.idl)
  //enum StatusId { All, OnlyFailed, OnlyOk };
  //enum QcId { QCAll, QC1, QC2 };
  kvalobs::KvDataSubscriberInfo   subscriberInfo_;
  CKvalObs::CService::kvDataNotifySubscriber_var subscriber_;

 public:
  KvDataNotifySubscriber(
           const kvalobs::KvDataSubscriberInfo            &subscriberInfo,
	   CKvalObs::CService::kvDataNotifySubscriber_ptr subscriber)
    :subscriberInfo_(subscriberInfo), subscriber_(subscriber)
    {
    }

  ~KvDataNotifySubscriber();

  
  /**
   *caller must release the returned referanse after use.
   */
  CKvalObs::CService::kvDataNotifySubscriber_ptr subscriber()const
    { return 
	CKvalObs::CService::kvDataNotifySubscriber::_duplicate(subscriber_);
    }

  kvalobs::KvDataSubscriberInfo subscriberInfo()const
                                { return subscriberInfo_;}
};

typedef boost::shared_ptr<KvDataNotifySubscriber>  
        KvDataNotifySubscriberPtr;

typedef std::list<KvDataNotifySubscriberPtr>       
        KvDataNotifySubscriberList;

typedef std::list<KvDataNotifySubscriberPtr>::iterator  
        IKvDataNotifySubscriberList;

typedef std::list<KvDataNotifySubscriberPtr>::const_iterator 
        CIKvDataNotifySubscriberList;


class DataNotifySubscriberFuncBase
{
 public:
  DataNotifySubscriberFuncBase(){}
  virtual ~DataNotifySubscriberFuncBase(){}
  
  virtual void func(KvDataNotifySubscriberPtr ptr)=0;
};


#endif
