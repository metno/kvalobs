/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvservicetypes.h,v 1.2.2.2 2007/09/27 09:02:45 paule Exp $                                                       

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
#ifndef __kvservicetypes_h__
#define __kvservicetypes_h__

#include <list>
#include <boost/shared_ptr.hpp>
#include <kvalobs/kvData.h>
#include <kvalobs/kvWorkelement.h>

#include "kvWhat.h"
#include "KvObsData.h"

namespace kvservice{
  typedef std::list<kvalobs::kvData>                 KvDataList;
  typedef std::list<kvalobs::kvData>::iterator       IKvDataList;
  typedef std::list<kvalobs::kvData>::const_iterator CIKvDataList;

  //  typedef std::list<KvDataList>                      KvObsDataList;
  typedef std::list<KvObsData>                       KvObsDataList;
  typedef KvObsDataList::iterator                    IKvObsDataList;
  typedef KvObsDataList::const_iterator              CIKvObsDataList;
  
  typedef boost::shared_ptr<KvObsDataList> KvObsDataListPtr;
  typedef boost::shared_ptr<KvDataList>    KvDataListPtr;

  typedef std::list<kvservice::KvWhat>                 KvWhatList;
  typedef std::list<kvservice::KvWhat>::iterator       IKvWhatList;
  typedef std::list<kvservice::KvWhat>::const_iterator CIKvWhatList;
  
  typedef boost::shared_ptr<KvWhatList> KvWhatListPtr;

  typedef std::list<kvalobs::kvWorkelement> KvWorkelementList;
  typedef boost::shared_ptr<KvWorkelementList> KvWorkelementListPtr;

}
#endif
