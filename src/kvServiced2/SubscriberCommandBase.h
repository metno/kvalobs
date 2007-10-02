/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: SubscriberCommandBase.h,v 1.1.2.2 2007/09/27 09:02:22 paule Exp $                                                       

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
#ifndef __SubscriberCommandBase_h__
#define __SubscriberCommandBase_h__

#include <dnmithread/CommandQue.h>
#include "DataToSubscribers.h"
#include "kvDataSubscriberInfo.h"

template <typename subref>
class SubscriberCommandBase:
  public dnmi::thread::CommandBase
{

protected:
  //  DataCommand();
  SubscriberCommandBase(const SubscriberCommandBase &);
  SubscriberCommandBase& operator=(const SubscriberCommandBase &);

  DataToSubscribersPtr       data_;
  typename subref::_ptr_type subscriber;
  std::string                subscriberid_;

public:
  typedef typename subref::_ptr_type _ptr_type;
  typedef typename subref::_var_type _var_type;
  typedef          subref            _interface;

  SubscriberCommandBase(DataToSubscribersPtr data2sub)
    :data_(data2sub)
    {}

  virtual ~SubscriberCommandBase(){};

  DataToSubscribersPtr data()const{ return data_;}
  
  std::string subscriberid()const { return subscriberid_;}
  void        subscriberid(const std::string &subid){ subscriberid_=subid;}
  
  ///Reset the data field with new data. 
  void data(DataToSubscribersPtr data){data_=data;}
  
  int operator()(KvDataSubscriberInfo &sinf, 
                  typename subref::_ptr_type sub){
    subscriber=sub;
    return execute(sinf);
  }


  /**
   * This function is run in the context of an SubscriberThread.
   *
   * \return 0 on success.
   *         1 on timeout (CORBA_TRANSIENT).
   *         2 the subscriber could'nt accept the data at the momment.
   *        -1 The subscriber is dead.
   */
  virtual int execute(KvDataSubscriberInfo &sinf)=0;

  
  bool       executeImpl(){}; //Do nothing

};


#endif
