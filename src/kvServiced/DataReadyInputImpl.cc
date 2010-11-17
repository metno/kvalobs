/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: DataReadyInputImpl.cc,v 1.3.2.2 2007/09/27 09:02:39 paule Exp $                                                       

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
#include <milog/milog.h>
#include <puTools/miTime.h>
#include "DataReadyInputImpl.h"
#include "DataReadyCommand.h"


using namespace miutil;
using namespace std;
using namespace milog;

DataReadyInputImpl::
DataReadyInputImpl(dnmi::thread::CommandQue &que_)
  :que(que_)
{
}

DataReadyInputImpl::
~DataReadyInputImpl()
{
}


/**
 * dataReady receives data from the kvManagerd and post it on the 'que'.
 * The serviceSubscriberThread listen on the que.
 */
CORBA::Boolean 
DataReadyInputImpl::dataReady( const CKvalObs::StationInfoList& infoList,
                               CKvalObs::CManager::CheckedInput_ptr callback,
			                      CORBA::Boolean& bussy)
{
   DataReadyCommand *cmd=0;

   bussy=false;

   LogContext context("dataReady");
   LOGDEBUG("New data from kvManagerd!");

   if(que.size()>5){
      bussy=true;
      return false;
   }

   try{
      cmd=new DataReadyCommand( infoList,
                                CKvalObs::CManager::CheckedInput::
                                _duplicate(callback));
      que.postAndBrodcast(cmd);
      return true;
   }catch(...){
      if(cmd){
         LOGERROR("Can post the data to the que! (NOMEM?)");
         delete cmd;
      }else{
         LOGFATAL("NOMEM!");
      }
      return false;
   }

}


CORBA::Boolean
DataReadyInputImpl::
dataReadyExt( const char* source,
              const CKvalObs::StationInfoList& infoList,
              CKvalObs::CManager::CheckedInput_ptr callback,
              CORBA::Boolean& bussy )
{
   DataReadyCommand *cmd=0;

   bussy=false;

   LogContext context("dataReady");
   LOGDEBUG("New data from kvManagerd!");

   if(que.size()>5){
      bussy=true;
      return false;
   }

   try{
      cmd=new DataReadyCommand( source,
                                infoList,
                                CKvalObs::CManager::CheckedInput::
                                _duplicate(callback));
      que.postAndBrodcast(cmd);
      return true;
   }catch(...){
      if(cmd){
         LOGERROR("Can post the data to the que! (NOMEM?)");
         delete cmd;
      }else{
         LOGFATAL("NOMEM!");
      }
      return false;
   }

}

