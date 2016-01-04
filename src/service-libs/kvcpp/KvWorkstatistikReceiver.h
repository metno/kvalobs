/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: KvGetDataReceiver.h,v 1.2.2.3 2007/09/27 09:02:45 paule Exp $                                                       

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
#ifndef __kvcpp_KvWorkstatistikReceiver_h__
#define __kvcpp_KvWorkstatistikReceiver_h__

#include "kvservicetypes.h"

namespace kvservice {

/**
 * KvGetDataReceiver is an interface used to retrive data
 * from kvalobs. It is used by KvApp::getKvData.
 */

class KvWorkstatistikReceiver {

 public:

  virtual ~KvWorkstatistikReceiver() {
  }

  /**
   * next, this function is called for every data set!
   * 
   * \datalist the data.
   * \return true if we shall continue. False if you want too 
   *         stop retriving data from kvalobs.
   */
  virtual bool next(KvWorkelementList &datalist)=0;

};

}

#endif

