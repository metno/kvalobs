/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvHintSubscriber.h,v 1.1.2.3 2007/09/27 09:02:40 paule Exp $                                                       

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
#ifndef __kvHintSubscriber_h__
#define __kvHintSubscriber_h__

#include "kvSubscriberBase.h"
#include <kvskel/kvService.hh>
#include <boost/shared_ptr.hpp>

class KvHintSubscriber : public KvSubscriberBase
{

  CKvalObs::CService::kvHintSubscriber_var subscriber_;

 public:
  KvHintSubscriber(CKvalObs::CService::kvHintSubscriber_ptr sub)
    :subscriber_(sub)
    {}
  
  CKvalObs::CService::kvHintSubscriber_ptr subscriber()const 
    { return 
	CKvalObs::CService::kvHintSubscriber::_duplicate(subscriber_);
    }

};

typedef boost::shared_ptr<KvHintSubscriber>  
        KvHintSubscriberPtr;

typedef std::list<KvHintSubscriberPtr>       
        KvHintSubscriberList;

typedef std::list<KvHintSubscriberPtr>::iterator  
        IKvHintSubscriberList;

typedef std::list<KvHintSubscriberPtr>::const_iterator 
        CIKvHintSubscriberList;



#endif 
