/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: smsdata.cc,v 1.4.2.1 2007/09/27 09:02:24 paule Exp $                                                       

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
#include <stdio.h>
#include <milog/milog.h>
#include <miutil/timeconvert.h>
#include "smsdata.h"

using namespace std;


bool
kvalobs::decoder::comobsdecoder:: 
SmsDataElem::
addData(const std::string &param,
	const std::string &data__,
	const kvalobs::kvUseInfo &uf,
	const kvalobs::kvControlInfo &cf,
	const int  level,
	const int  sensor )
{
  long pid;
  bool isText;
 
  if(data__.empty()){
    LOGDEBUG6("SmsDataElem::addData: <" << param << "> No data!")
    return true;
  }

  if(!findParam(param, pid, isText)){
    LOGDEBUG6("SmsDataElem::addData: Unknown param! <" << param <<">");
    return false;
  }

  if(isText){
    kvalobs::kvTextData mydata(sid, 
    		to_ptime(dtObs),
			       data__,
			       pid,
			       to_ptime(tbtime),
			       tid);

    list<kvalobs::kvTextData>::iterator it=textData_.begin();

    for(;it!=textData_.end();it++){
      if(pid<it->paramID()){
	textData_.insert(it, mydata);
	break;
      }
    }

    if(it==textData_.end())
      textData_.push_back(mydata);
  }else{
    float fval;

    if(sscanf(data__.c_str(),"%f", &fval)!=1){
      LOGWARN("SmsDataElem::addData: cant convert <" << data__ << "> to float"
	      <<endl);

      return false;
    }

    kvalobs::kvData mydata(sid, 
    		to_ptime(dtObs),
			   fval,
			   pid,
			   to_ptime(tbtime),
			   tid,
			   sensor,
			   level,
			   fval,
			   cf,
			   uf,
			   "");


    list<kvalobs::kvData>::iterator it=data_.begin();

    for(;it!=data_.end();it++){
      if(pid<it->paramID()){
	data_.insert(it, mydata);
	break;
      }
    }

    if(it==data_.end())
      data_.push_back(mydata);
    
  }

  return true;
}

bool
kvalobs::decoder::comobsdecoder:: 
SmsDataElem::
findParam(const std::string &pname, long &paramid, bool &isText)const
{
  if(!params)
    return false;

  CIParamList it=params->begin();

  for(; it!=params->end(); it++){
    if(!it->valid())
      continue;
    if(pname==it->kode()){
      if(it->id()>=1000)
	isText=true;
      else 
	isText=false;

      paramid=it->id();
      return true;
    }
  }

  return false;

}


std::string
kvalobs::decoder::comobsdecoder:: 
SmsDataElem::
findParam(long paramid)const
{
  if(!params)
    return string();

  CIParamList it=params->begin();

  for(; it!=params->end(); it++){
    if(!it->valid())
      continue;
    if(paramid==it->id()){
      return it->kode();
    }
  }

  return string();


}

std::ostream& 
kvalobs::decoder::comobsdecoder::
operator<<(std::ostream &s, 
	   const kvalobs::decoder::comobsdecoder::SmsDataElem &elem)
{
  std::list<kvalobs::kvData>::const_iterator it=elem.data().begin();
  std::list<kvalobs::kvTextData>::const_iterator itt=elem.textData().begin();

  for(;it!=elem.data().end(); it++){
    s << it->stationID() << "," 
      << it->obstime()   << "," 
      << it->typeID()    << ","
      << elem.findParam(it->paramID()) << "(" << it->paramID() << ")," 
      << it->original()  << ","
      << it->level()     << "," 
      << it->sensor()    << "," 
      << it->controlinfo().flagstring() << "," 
      << it->useinfo().flagstring()     << endl;
  }

  for(;itt!=elem.textData().end(); itt++){
    s << itt->stationID() << "," 
      << itt->obstime()   << "," 
      << itt->typeID()    << ","
      << elem.findParam(itt->paramID()) << "(" << itt->paramID() << ")," 
      << itt->original()  << endl;
  }

  return s;
}

kvalobs::decoder::comobsdecoder::
SmsData::
SmsData(const ParamList &params_, long stationid, long smscode)
  :params(params_), sid(stationid), tid(smscode+300)
{
}

kvalobs::decoder::comobsdecoder::
SmsData::~SmsData()
{
}

 
void       
kvalobs::decoder::comobsdecoder::
SmsData::add(const SmsDataElem &elem)
{
  ITSmsDataElem it=data_.begin();

  for(;it!=data_.end(); it++){
    if(elem.getDate()>it->getDate()){
      data_.insert(it, elem);
      return;
    }
  }

  data_.push_back(elem);
}

