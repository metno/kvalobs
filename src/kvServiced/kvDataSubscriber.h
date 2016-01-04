/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: kvDataSubscriber.h,v 1.1.6.2 2007/09/27 09:02:39 paule Exp $                                                       

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
#ifndef __kvDataSubscriber_ijk_h__
#define __kvDataSubscriber_ijk_h__

#include <list>
#include <map>
#include <kvskel/kvService.hh>
#include "kvDataSubscriberInfo.h"
#include <boost/shared_ptr.hpp>
#include "kvSubscriberBase.h"

/**
 * KvDataSubscriber holds the information from a data subcriber.
 */

class KvDataSubscriber : public KvSubscriberBase {

  //Definitions from kvService.hh (kvService.idl)
  //enum StatusId { All, OnlyFailed, OnlyOk };
  //enum QcId { QCAll, QC1, QC2 };
  kvalobs::KvDataSubscriberInfo subscriberInfo_;
  CKvalObs::CService::kvDataSubscriber_var subscriber_;

 public:
  KvDataSubscriber(const kvalobs::KvDataSubscriberInfo &subscriberInfo,
                   CKvalObs::CService::kvDataSubscriber_ptr subscriber)
      : subscriberInfo_(subscriberInfo),
        subscriber_(subscriber) {
  }

  ~KvDataSubscriber();

  /**
   *caller must release the returned referanse after use.
   */
  CKvalObs::CService::kvDataSubscriber_ptr subscriber() const {
    return CKvalObs::CService::kvDataSubscriber::_duplicate(subscriber_);
  }

  kvalobs::KvDataSubscriberInfo subscriberInfo() const {
    return subscriberInfo_;
  }
};

typedef boost::shared_ptr<KvDataSubscriber> KvDataSubscriberPtr;

typedef std::list<KvDataSubscriberPtr> KvDataSubscriberList;

typedef std::list<KvDataSubscriberPtr>::iterator IKvDataSubscriberList;

typedef std::list<KvDataSubscriberPtr>::const_iterator CIKvDataSubscriberList;

class DataSubscriberFuncBase {
 public:
  DataSubscriberFuncBase() {
  }
  virtual ~DataSubscriberFuncBase() {
  }

  virtual void func(KvDataSubscriberPtr ptr)=0;
};

#endif
