/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvservicetypes.h,v 1.4.2.2 2007/09/27 09:02:47 paule Exp $                                                       

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
#include "kvWhat.h"


namespace kvservice{
  /**
   * \addtogroup kvqt
   * @{
   */

  /**
   * A list of kvalobs::kvData. 
   */
  typedef std::list<kvalobs::kvData>                 KvDataList;
  
  /**
   * A iterator to iterate through a KvDataList. 
   */
  typedef std::list<kvalobs::kvData>::iterator       IKvDataList;
 
  /**
   * A \em const iterator to iterate through a KvDataList. 
   */
  typedef std::list<kvalobs::kvData>::const_iterator CIKvDataList;

  
  /**
   * A list of KvDataList. Every kvalobs::kvData element in a KvDataList 
   * sub list has the same \em obstime, \em stationid and \em typeid.
    */
  typedef std::list<KvDataList>                      KvObsDataList;
   /**
   * A \em iterator to iterate through a KvObsDataList. 
   */
  typedef std::list<KvDataList>::iterator            IKvObsDataList;
   /**
   * A \em const iterator to iterate through a KvObsDataList. 
   */
  typedef std::list<KvDataList>::const_iterator     CIKvObsDataList;
  

  /**
   * KvObDataListPtr is a referance counted KvObsDataList. This means
   * that applications using i dont need to worry about who is responsible to
   * delete the pointer.
   */
  typedef boost::shared_ptr<KvObsDataList> KvObsDataListPtr;
 
   /**
   * KvDataListPtr is a referance counted KvObsDataList. This means
   * that applications using i dont need to worry about who is responsible to
   * delete the pointer.
   */
   typedef boost::shared_ptr<KvDataList>    KvDataListPtr;

   
   /**
    * A list of kvservice::KvWhat elements.
    */
  typedef std::list<kvservice::KvWhat>                 KvWhatList;
  /**
   * A iteraor to iterate through a KvWhatList.
   */
  typedef std::list<kvservice::KvWhat>::iterator       IKvWhatList;
  
  /**
   * A \em const iteraor to iterate through a KvWhatList.
   */
  typedef std::list<kvservice::KvWhat>::const_iterator CIKvWhatList;
  
   /**
    * KvWhatListPtr is a referance counted KvWhatList. This means
   * that applications using i dont need to worry about who is responsible to
   * delete the pointer.
   */
  typedef boost::shared_ptr<KvWhatList> KvWhatListPtr;

  /** @} */

}
#endif
