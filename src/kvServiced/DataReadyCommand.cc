/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: DataReadyCommand.cc,v 1.2.2.1 2007/09/27 09:02:39 paule Exp $                                                       

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
#include "DataReadyCommand.h"

DataReadyCommand::
DataReadyCommand(const CKvalObs::StationInfoList &stInfo, 
		 const CKvalObs::CManager::CheckedInput_ptr callback_)
  :kvalobs::StationInfoCommand(stInfo), callback(callback_)
{
}

DataReadyCommand::
DataReadyCommand( const char *source,
                  const CKvalObs::StationInfoList &stInfo,
                  const CKvalObs::CManager::CheckedInput_ptr callback_)
  :kvalobs::StationInfoCommand(stInfo), callback(callback_)
{
   if( source )
      source_ = source;
}

DataReadyCommand::
DataReadyCommand( const char *source,
                  const CKvalObs::StationInfoExtList &stInfo,
                  const CKvalObs::CManager::CheckedInput_ptr callback_)
: kvalobs::StationInfoCommand( stInfo ), callback( callback_ )
{
   if( source )
       source_ = source;
}

  
 
CKvalObs::CManager::CheckedInput_ptr 
DataReadyCommand::
getCallback()const
{
  return CKvalObs::CManager::CheckedInput::_duplicate(callback); 
}


