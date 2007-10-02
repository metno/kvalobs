/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvHintSubscriberImpl.cc,v 1.2.2.2 2007/09/27 09:02:46 paule Exp $                                                       

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
#include <qthread.h>
#include <qapp.h>
#include "kvQtEvents.h"
#include "kvHintSubscriberImpl.h"

kvservice::priv::
KvHintSubscriber::KvHintSubscriber()
{
}

kvservice::priv::
KvHintSubscriber::~KvHintSubscriber()
{
}

void 
kvservice::priv::
KvHintSubscriber::kvUp()
{
  KvHintEvent *hintEvent;

  try{
    hintEvent=new KvHintEvent(true);
    QApplication::postEvent(qApp, hintEvent);
  }
  catch(...){
    return;
  }

}

void 
kvservice::priv::
KvHintSubscriber::kvDown()
{
  KvHintEvent *hintEvent;

  try{
    hintEvent=new KvHintEvent(false);
    QApplication::postEvent(qApp, hintEvent);
  }
  catch(...){
    return;
  }

}


