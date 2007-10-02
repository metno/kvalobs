/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: qabaseInputImpl.cc,v 1.10.2.3 2007/09/27 09:02:38 paule Exp $                                                       

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
#include "../include/qabaseInputImpl.h"
#include "../include/qabaseApp.h"
#include "../include/QaWorkCommand.h"
#include <milog/milog.h>

QaBaseInputImpl::QaBaseInputImpl(QaBaseApp &app_):app(app_)
{
}

QaBaseInputImpl::~QaBaseInputImpl()
{
}




CORBA::Boolean 
QaBaseInputImpl::newDataCB(const CKvalObs::StationInfo& data, 
			   CKvalObs::CManager::CheckedInput_ptr callback, 
			   const micutil::KeyValList& args,
			   CORBA::Boolean &bussy )
{
  milog::LogContext context("CORBA(QaBaseInputImpl::newDataCB)");

  bussy=false;
  
  if(app.getInQue().size()>2){
    LOGDEBUG6("BUSSY!\n");

    bussy=true;
    return false;
  }
  
  LOGDEBUG("New Data to process!\n");

  try{
    app.getInQue().postAndBrodcast(
     new QaWorkCommand(data, 
		       CKvalObs::CManager::CheckedInput::_duplicate(callback),
		       args)
     );
  }
  catch(...){
    return false;
  }
  
  return true;
}
