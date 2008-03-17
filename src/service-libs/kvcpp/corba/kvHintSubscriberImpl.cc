/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvHintSubscriberImpl.cc,v 1.2.2.3 2007/09/27 09:02:46 paule Exp $                                                       

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
#include "../kvevents.h"
#include <milog/milog.h> 
#include "kvHintSubscriberImpl.h"

kvservice::priv::
HintSubscriber::HintSubscriber(dnmi::thread::CommandQue &que_):que(que_)
{
}

kvservice::priv::
HintSubscriber::~HintSubscriber()
{
}

void 
kvservice::priv::
HintSubscriber::kvUp()
{
  HintEvent *hintEvent;

  try{
    hintEvent=new HintEvent(true);
    que.postAndBrodcast(hintEvent);
  }
  catch(...){
    return;
  }

}

void 
kvservice::priv::
HintSubscriber::kvDown()
{
  HintEvent *hintEvent;

  try{
    hintEvent=new HintEvent(false);
    que.postAndBrodcast(hintEvent);
  }
  catch(...){
    return;
  }

}


