/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: Waiting.cc,v 1.3.6.4 2007/09/27 09:02:23 paule Exp $                                                       

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
#include <time.h>
#include <sstream>
#include <kvalobs/kvDbGate.h>
#include <milog/milog.h>
#include "Waiting.h"
#include "tblWaiting.h"

using namespace std;
using namespace kvalobs;

bool 
Waiting::
addToDb(dnmi::db::Connection *con)
{
  if(!con)
    return false;

  kvDbGate gate(con);
  TblWaiting data(info_->wmono(), obstime_, delay_);
 
  gate.busytimeout(120);
  
  if(!gate.replace(data)){
    LOGERROR("Waiting::addToDb: " << gate.getErrorStr());
    return false;
  }

  return true;
}

bool 
Waiting::
removeFrom(dnmi::db::Connection *con)
{
  if(!con)
    return false;
  
  kvDbGate gate(con);
  TblWaiting data(info_->wmono(), obstime_, delay_);
  
  gate.busytimeout(120);
  
  if(!gate.remove(data)){
    LOGERROR("Waiting::removeFrom: " << gate.getErrorStr());
    return false;
  }

  return true;
}

bool 
Waiting::
inDb(dnmi::db::Connection *con)
{
  if(!con)
    return false;

  kvDbGate gate(con);
  list<TblWaiting> data;
  ostringstream o;

  gate.busytimeout(120);

  o << "WHERE wmono=" << info_->wmono() << " AND obstime=\'" 
    <<obstime_ << "\'";

  if(gate.select(data, o.str())){
    if(data.empty())
      return false;
    else
      return true;
  }
  
  LOGERROR("Waiting::inDb: " << gate.getErrorStr());
  return false;
}
