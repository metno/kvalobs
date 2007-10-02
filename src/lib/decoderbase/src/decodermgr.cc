/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: decodermgr.cc,v 1.11.6.4 2007/09/27 09:02:27 paule Exp $                                                       

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
#include <fileutil/dir.h>
#include "../include/decoderbase/decodermgr.h"
#include <milog/milog.h>
#include <dnmithread/mtcout.h>
#include <miutil/trimstr.h>

using namespace dnmi::file;
using namespace kvalobs::decoder;
using namespace std;
using namespace miutil;

/**
 * returns true when all decoder has a decoderCount of 0.
 */

bool 
DecoderMgr::readyForUpdate()
{
}

void 
DecoderMgr::updateDecoders()
{
  Dir                dir;
  std::string        name;
  DSO                *dso;
  decoderFactory     fac;
  releaseDecoderFunc releaseFunc;
  getObsTypes        obsTypes;
  DecoderItem        *decoder;
  int                id=0;

  decoders.clear();
  
  if(!dir.open(decoderPath, "*.so")){
    LOGERROR("DecoderMgr: Can't open directory <" << decoderPath << ">, " 
	 << dir.getErr() << endl);
    return;
  }
  
  while(dir.hasNext()){
    name=dir.next();
    dso=0;
    fac=0;
    releaseFunc=0;
    obsTypes=0;
    
    LOGDEBUG("DecoderMgr: loading <" << name << ">\n");

    try{
      dso=new DSO(decoderPath+"/"+name);
      fac=(decoderFactory) (*dso)["decoderFactory"];
      obsTypes=(getObsTypes)(*dso)["getObsTypes"];
      releaseFunc=(releaseDecoderFunc)(*dso)["releaseDecoder"];
      decoder=new DecoderItem(fac, releaseFunc, dso, modTime(name));
      decoder->obsTypes=obsTypes();
      decoder->decoderId=id;
      id++;
      decoders.push_back(decoder);
    }
    catch(dnmi::file::DSOException &ex){
      LOGDEBUG("DecoderMgr: can't load <" << name 
	   << ">\n reason: " << ex.what() << endl);
      delete dso;
    }
    catch(...){
      LOGERROR("DecoderMgr: can't load <" << name 
	   << ">\n reason: out of memmory\n");
      delete dso;
      return;
    }
  }
}
     

DecoderMgr::DecoderMgr(const miutil::miString &decoderPath_)
{
  setDecoderPath(decoderPath_);
}


void
DecoderMgr::setDecoderPath(const miutil::miString &decoderPath_)
 
{
  decoderPath=decoderPath_;
  string::reverse_iterator rit=decoderPath.rbegin();

  if(rit!=decoderPath.rend() && *rit=='/')
    decoderPath.erase(decoderPath.length()-1);


  updateDecoders();
}



DecoderMgr::~DecoderMgr()
{
  IDecoderList it=decoders.begin();

  for( ; it!=decoders.end(); it++)
    delete *it;
  
}

kvalobs::decoder::DecoderBase*
DecoderMgr::findDecoder(dnmi::db::Connection   &connection,
			const ParamList        &params,
			const std::list<kvalobs::kvTypes> &typeList,
			const miutil::miString &obsType_,
			const miutil::miString &obs,
			miutil::miString &errorMsg)
{
	miString myObsType(obsType_);
	miString decoderName;
  	miString::size_type i;
  	IDecoderList it=decoders.begin();
  	std::list<miutil::miString>::iterator itOT;
  	DecoderBase *dec;
  	miString    metaconf;

  	i=myObsType.find_first_of("/\n");

  	if(i!=miString::npos){
    	decoderName=myObsType.substr(0, i);
    	trimstr(decoderName);
  	}

  	i=myObsType.find("<?xml");
  
  	if(i!=miString::npos){
      	metaconf=myObsType.substr(i);
      	myObsType.erase(i);
  	}

	if(decoderName.empty()){
		decoderName=myObsType;
		trimstr(decoderName);
	}


  for(; it!=decoders.end(); it++){
      itOT=(*it)->obsTypes.begin();
    
    for(;itOT!=(*it)->obsTypes.end(); itOT++){
      if(*itOT==decoderName){
	  dec=(*it)->factory(connection, 
			     params,
			     typeList,
			     (*it)->decoderId,
			     myObsType,
			     obs);

	  if(!dec){
	    errorMsg="ERROR, Cant create a decoder. (out of memmory?)";
	    return 0;
	  }
	
	  (*it)->decoderCount++;

	  if(!metaconf.empty())
	    dec->setMetaconf(metaconf);
	    
	  return dec;
      }
    }
  }

  LOGDEBUG("DecoderMgr: Unknown observation type <" << decoderName << ">!\n");
  errorMsg="Unknown observation type <"+decoderName+">!";
  return 0;
}

void    
DecoderMgr::releaseDecoder(DecoderBase *decoder)
{
  IDecoderList it=decoders.begin();
  
  for(; it!=decoders.end(); it++){
    if((*it)->decoderId==decoder->getDecoderId()){
      (*it)->decoderCount--;
      (*it)->releaseFunc(decoder);
      break;
    }
  }
  
  if(it==decoders.end())
    LOGINFO("DecoderMgr: Unknown decoderId (" << decoder->getDecoderId() 
	     << "  Decoder <" << decoder->name() << ">\n");
}


void 
DecoderMgr::obsTypes(std::list<std::string> &list)
{
  CIDecoderList it=decoders.begin();
  std::list<miutil::miString>::const_iterator cit;

  list.clear();

  for(; it!=decoders.end(); it++){
    cit=(*it)->obsTypes.begin();
    
    for(;cit!=(*it)->obsTypes.end(); cit++)
      list.push_back(*cit);
  }
}
