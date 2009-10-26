/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: DataSrcImpl.cc,v 1.14.6.2 2007/09/27 09:02:18 paule Exp $                                                       

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
#include <dnmithread/mtcout.h>
#include "DataSrcImpl.h"
#include "DecodeCommand.h"

using namespace std;

DataSrcImpl::DataSrcImpl(DataSrcApp &app_)
  :app(app_)
{
}

DataSrcImpl::~DataSrcImpl()
{
}

CKvalObs::CDataSource::Result* 
DataSrcImpl::newData(const char* data, const char* obsType)
{
  using namespace CKvalObs::CDataSource;

  DecodeCommand       *decCmd;
  DecodeCommand       *decCmdRet;
  DataSrcApp::ErrCode errCode;
  string              errMsg;
  
  Result *res;
  
  milog::LogContext logContext("CORBA(DataSrcImpl)"); 

  LOGDEBUG("newData: arrived!\n"); 
  LOGDEBUG("obsType: " << obsType << 
	   "\ndata: [" << data << "]");

  try{
    res=new Result();
  }catch(...){
    LOGFATAL("newData: Out of memmory!\n");
    throw Fatal();
  }

  decCmd=app.create(obsType, data, 60000, errCode, errMsg);

  if(!decCmd){
    if(errCode==DataSrcApp::NoDbConnection){
      res->res=CKvalObs::CDataSource::NOTSAVED;;
      res->message=errMsg.c_str();
    }else if(errCode==DataSrcApp::NoMem){
      throw Fatal();
    }else if(errCode==DataSrcApp::NoDecoder){
      res->res=CKvalObs::CDataSource::NODECODER;
      res->message=errMsg.c_str();
    }else{
      res->res=CKvalObs::CDataSource::ERROR;
      res->message=errMsg.c_str();
    }
    return res;
  }

  if(!app.put(decCmd)){
    //We are in shutdown.
    app.releaseDecodeCommand(decCmd);
    res->res=CKvalObs::CDataSource::ERROR;
    res->message="SHUTDOWN: In the proccess of shuting down!";
    LOGERROR("Shutdown in proccess!");
    return res;
  }

  while(true){
    decCmdRet=decCmd->wait(1);

    if(decCmdRet)
      break;
    
    if(app.inShutdown()){
      // We try to remove the command from the que before it
      // is received by the thread. If we can get it back we
      // release the command and returns to the caller.
      //
      // If we can't remove the command we have to wait for the
      // thread to return the command back before we can procced.
      
      LOGINFO("Shutdown in proccess!");
      dnmi::thread::CommandBase *cmd;
      cmd=app.remove(decCmd);
      
      if(!cmd)
	continue;
      
      decCmdRet=dynamic_cast<DecodeCommand*>(cmd);
        
      if(decCmdRet){
	LOGDEBUG("SHUTDOWN: Removed the command from the que.");
      }else{
	LOGERROR("SHUTDOWN: Unexpected command.");
      }

      app.releaseDecodeCommand(decCmd);
      res->res=CKvalObs::CDataSource::ERROR;
      res->message="SHUTDOWN: In the proccess of shuting down!";
      return res;
    }
  }

  kvalobs::decoder::DecoderBase::DecodeResult decodeResult=decCmd->getResult();

  if(decodeResult==kvalobs::decoder::DecoderBase::Ok){
    LOGDEBUG("Decode: OK.");
    kvalobs::kvStationInfoList &infoList=decCmd->getInfoList();
    app.sendInfoToManager(infoList);
    res->res=CKvalObs::CDataSource::OK;
  }else if(decodeResult==kvalobs::decoder::DecoderBase::NotSaved){
    LOGDEBUG("Decode: NotSaved.");
    res->res=CKvalObs::CDataSource::NOTSAVED;
  }else{
    if(decodeResult==kvalobs::decoder::DecoderBase::Error){
      LOGERROR("Unrecoverable error!");
    }else{
      LOGDEBUG("Rejected.");
    }
    
    res->res=CKvalObs::CDataSource::DECODEERROR;
  }

  res->message=decCmd->getMsg().c_str();

  app.releaseDecodeCommand(decCmd);

  return res;
}

