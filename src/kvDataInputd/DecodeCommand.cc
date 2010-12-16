/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: DecodeCommand.cc,v 1.12.6.2 2007/09/27 09:02:18 paule Exp $                                                       

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
#include "DecodeCommand.h"


DecodeCommand::~DecodeCommand()
{
}

DecodeCommand::DecodeCommand(kvalobs::decoder::DecoderBase *decoder_)
  :decoder(decoder_)
{
}


kvalobs::kvStationInfoList& 
DecodeCommand::getInfoList()
{
  return decoder->getStationList();
}
  
bool       
DecodeCommand::executeImpl()
{
  LOGDEBUG("DecodeCommand::execute: called!\n");

  result=decoder->execute(msg);

  LOGDEBUG("DecodeCommand::execute: return!\n");
  return result==kvalobs::decoder::DecoderBase::Ok;
}

void     
DecodeCommand::debugInfo(std::iostream &info)
{
  
}

  
DecodeCommand*
DecodeCommand::wait(int timeoutInSecond)
{
  DecodeCommand* cmd;


  try{
	  dnmi::thread::CommandBase * msg = resQue.get(timeoutInSecond);
	  if ( ! msg )
		  return 0;
    cmd=dynamic_cast<DecodeCommand *>(msg);

    if(!cmd){
      LOGWARN("DecodeCommand::wait: timeout?? (failed dynamic_cast)!\n");
      return 0;
    }
  }
  catch(...){
    LOGERROR("DecodeCommand::wait: failed dynamic_cast!\n");
    return 0;
  }

  return cmd;
}


void       
DecodeCommand::signal()
{
  resQue.postAndBrodcast(this);
}
