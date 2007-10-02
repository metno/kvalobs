/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvWhatListQue.h,v 1.1.6.1 2007/09/27 09:02:47 paule Exp $                                                       

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
#ifndef __kvWhatListQue_h__
#define __kvWhatListQue_h__

#include <boost/shared_ptr.hpp>
#include "MessageQue.h"
#include "kvWhat.h"

namespace kvservice{
  namespace priv{

    /*
     *  Why is this implemented as a class and not as
     *  
     *      typedef MessageQue<kvalobs::service::KvWhatListPtr> KvWhatListQue;
     *
     *  Reason: 
     *     I don't want to expose to much of the implementation details
     *     to the user of this library as it just may confuse.
     *     
     *     If I declare it as a class can I do a forward declaration in 
     *     the file kvQtApp.h and I don't need to expose this file to
     *     the public. 
     */

      class KvWhatListQue: public  MessageQue<kvservice::KvWhatListPtr> 
      {
      public:
	  KvWhatListQue(){}
	  ~KvWhatListQue(){}
	  
      };
  }
}
#endif
