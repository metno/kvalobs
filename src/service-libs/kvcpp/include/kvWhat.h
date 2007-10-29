/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvWhat.h,v 1.2.2.3 2007/09/27 09:02:45 paule Exp $                                                       

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
#ifndef __kvWhat_h__
#define __kvWhat_h__

#include <map>
#include <string>
#include <list>
#include <boost/shared_ptr.hpp>
#include <kvskel/kvDataNotifySubscriber.hh>
#include <puTools/miTime>

namespace kvservice{
    class KvWhat{
	KvWhat();
	
	typedef std::map<CKvalObs::CService::QcId, bool> QcId;      
		
	long                       stationID_;
	long                       typeID_;
	QcId                       qcID_;
	miutil::miTime             obsTime_;
	
    public:
	KvWhat(const KvWhat &w);
	KvWhat(const CKvalObs::CService::kvDataNotifySubscriber::What &what);
	~KvWhat(){}
	
	KvWhat& operator=(const KvWhat &w); 

	bool  qcID(CKvalObs::CService::QcId id)const 
	    { return qcID_.find(id)!=qcID_.end();}
	long  stationID()const { return stationID_;}
	long  typeID()const {return typeID_;}
	miutil::miTime obsTime()const { return obsTime_;}
    };

    typedef std::list<KvWhat>                   KvWhatList;
    typedef std::list<KvWhat>::iterator        IKvWhatList;
    typedef std::list<KvWhat>::const_iterator CIKvWhatList;
    
    typedef boost::shared_ptr<KvWhatList> KvWhatListPtr; 

}



#endif
