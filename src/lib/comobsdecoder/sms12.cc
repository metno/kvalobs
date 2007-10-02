/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: sms12.cc,v 1.2.2.3 2007/09/27 09:02:24 paule Exp $                                                       

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
#include <stdlib.h>
#include <boost/lexical_cast.hpp>
#include <milog/milog.h>
#include "decodeutil.h"
#include "sms12.h"
#include <miutil/trimstr.h>

/**
 * SMSDecode12.
 * Denne klassen brukes for � dekode SMS meldinger fra PIO stasjoner.
 * SMS code er 12.
 */

#define KNOPFAKTOR     1.94384449244


using namespace std;
using namespace miutil;
using namespace kvalobs::decodeutil;

namespace {

    const char *PARAM_IN[]={
      "KLCOMOBS", //Blir lagt til av ComObs, UTC tid.
      "year",
      "day_in_year",
      "termin",      //synop termin ie. hour
      "KLOBS",       //DDhhmm
      "VERSION",     //Versjon av PIO programvare
      "RRA",
      "CCA", 
      "TP",  //??
      "PO",
      "PR",
      "AA",
      "PP",
      "_irix",
      "_RRRtr",
      "_Esss",
      "EV_24",  //??
      "TA",
      "TNT",
      "TAN_12",
      "TXT",
      "TAX_12",
      "TGN_12",
      "TW",
      "UU",
      "SG",  //Sj�gang
      "DD",
      "FF",
      "ITZ",
      "FX",
      "FG",
      "NN",   //Nddff
      "_hVV",
      "_wwW1W2",
      "_RtWdWdWd",
      "_NhClCmCh",
      "_NsChshs", 
      "_VT_new1",
      "_VT_new2",
      "_VT_old1",
      "_VT_old2",
      "_knr",
      "RR",
      "PB",
      "UH",
      "signature",
      "TEXT",
      0
    };
}



kvalobs::decoder::comobsdecoder::
Sms12::
Sms12(const ParamList &paramList,
      kvalobs::decoder::comobsdecoder::ComObsDecoder &dec)
  : SmsBase(paramList, dec)
{
}

kvalobs::decoder::comobsdecoder::
Sms12::
~Sms12()
{
}





kvalobs::decodeutil::DecodedData*
kvalobs::decoder::comobsdecoder::
Sms12::decode(long stationid,
	     int  smscode,
	     const SmsMelding::MeldingList &obs,
	     std::string &returnMessage,
	     std::list<kvalobs::kvRejectdecode> &rejected,
	     bool &hasRejected)
{
  string       param;
  miTime       klObs;
  int          nDataset=0;
  string       buf;
  ostringstream errs;
  char         sepBuf[2];
  char         sep;
  bool         ccx;
  kvalobs::decodeutil::DecodedData *smsData;

  sepBuf[1]='\0';

  hasRejected=false;

  ComObsDec.loadConf(stationid, smscode, cfInfo);

  try{
   smsData=new kvalobs::decodeutil::DecodedData(paramList, stationid, smscode);
  }
  catch(...){
    LOGFATAL("NOMEM: failed to allocate memory for SmsData!" << endl);
    return 0;
  }

  kvalobs::decodeutil::DecodedDataElem data=smsData->createDataElem();

  for(SmsMelding::CIMeldingList it=obs.begin();
      it!=obs.end();
      it++){
    data.clean();
    buf=*it;
    stripNewLine(buf);
     
    if(buf.length()<8){
      LOGERROR("Invalid format!");
      continue;
    }

    sep=separator(buf);

    if(sep<0){
      LOGERROR("Invalid dataformat. Unknown element separator.");
      continue;
    }

    sepBuf[0]=sep;
    LOGINFO("Using seperator <" + string(sepBuf) + ">.");
    CommaString cStr(buf, sep);

    initInData(&cStr, PARAM_IN);

    if(cStr.size()<8){
      LOGERROR("Invalid dataformat. Too few elements.");
      continue;
    }

    miTime obstime=getObsTime(errs, klObs, ccx);
    
    if(obstime.undef()){
      string mybuf;
      cStr.copy(mybuf);
      LOGERROR("Invalid obstime!");
      rejected.push_back(kvRejectdecode(mybuf, miTime::nowTime(), "comobs/typeid=312",
					errs.str()));
      hasRejected=true;
      continue;
    }

    data.setDate(obstime);

    errs.str("");

    if(!doPrecip(data, obstime.hour())){
      string mybuf;
      cStr.copy(mybuf);
      LOGERROR("Invalid dataformat cant do precipitation.");
      rejected.push_back(kvRejectdecode(mybuf, miTime::nowTime(), "comobs/typeid=312",
					errs.str()));
      hasRejected=true;
      continue;
    }

    for(int i=8; PARAM_IN[i]; i++){
      
      if(!getInData(PARAM_IN[i], buf)){
	continue;
      }

      trimstr(buf);
      
      if(buf.empty())
	continue;

      param=PARAM_IN[i];

      if(param=="_irix"  ||
	 param=="_RRRtr" ||
	 param=="RR"     ||
	 param=="_knr"   ||
	 param=="TEXT"){
	//Skip this. The precipitation is done in doPrecip.
	//The rest is of no interest in kvalobs.
	continue;
      }else if(param=="PP"){
	if(buf.empty())
	  continue;
	if(buf[0]=='-' || buf[0]=='+')
	  buf.erase(0, 1);
	
	if(!buf.empty())
	  data.addData(param, buf);

      }else if(param=="DD"){
	//In deg with 10 deg resolution.
	ostringstream o;
	int i;
	
	try{
	  i=boost::lexical_cast<int>(buf);
	}
	catch(...){
	  LOGERROR("Format error (DD): (" << buf 
		   << "). Not a number!");
	  continue;
	}
	
	i*=10; //To deg.
	o << i;
	data.addData(param, o.str());
      }else if(param=="FF" || 
	       param=="FX" ||
	       param=="FG"){
	ostringstream o;
	float f;
	
	try{
	  f=boost::lexical_cast<float>(buf);
	}
	catch(...){
	  LOGERROR("FORMAT error FF, FG, or FX: (" <<
		   buf << ")!");
	  continue;
	}

	f/=KNOPFAKTOR;
	o << f;
	data.addData(param, o.str());
      }else if(param=="_RtWdWdWd"){
	//Do only the WdWdWd stuff
	buf.erase(0, 1);
	doWdWdWd(data, buf);
      }else if(param=="_Esss"){
	doKvEsss(data, buf);
      }else if(param=="_NhClCmCh"){
	doKvNhClCmCh(data, buf);
      }else if(param=="_hVV"){
	dohVV(data, buf);
      }else if(param=="_NsChshs"){
	doNsChshs(data, buf);
      }else if(param== "_wwW1W2"){
	dowwW1W2(data, buf);
      }else if(param=="_VT_new1"){
	doVT(data, buf, "V1", "V2");
      }else if(param=="_VT_new2"){
	doVT(data, buf, "V3", "");
      }else if(param=="_VT_old1"){
	doVT(data, buf, "V4", "V5");
      }else if(param=="_VT_old2"){
	doVT(data, buf, "V6", "V7");
      }else{
	data.addData(param, buf);
      }
    }
  } 

  nDataset++;
  
  if(nDataset>0){
    LOGINFO("SmsMelding (312) decoded." << endl);
  }else{
    LOGINFO("No data." << endl);
  }
  
  return smsData;
}

miutil::miTime 
kvalobs::decoder::comobsdecoder::
Sms12::
getObsTime(std::ostringstream &ost, miutil::miTime &klObs, bool &ccx)
{
  string buf;
  int hour;
  miTime klComObs; 
  miTime obstime;
  miTime fromTime, toTime;

  milog::LogContext contect("getObsTime");
  klObs=miTime(); //Set to undef.
  ccx=false;

  getInData("CCA", buf);

  if(!buf.empty()){
    ccx=atoi(buf.c_str())!=0;
    
    if(ccx){
      LOGINFO("This is correction, ie CCA.");
    }
  }
  
  LOGDEBUG("CCA: " << (ccx?"TRUE":"FALSE"));  

  if(!getInData("KLCOMOBS", buf)){
    ost << "0:  Internal error!";
    LOGERROR(ost.str());
    return miTime();
  }
  
  klComObs=createDTFromYMDhm(buf);

  if(klComObs.undef()){
    LOGWARN("Can't decode KLCOMOBS time <" << buf << ">!");
  }
  
  if(!getInData("KLOBS", buf)){
    LOGWARN("1: Internal error!");
    LOGERROR(ost.str());
    return miTime();
  }

  if(!buf.empty()){
    klObs=createDTFromDHM(buf, klComObs);
  }

  if(klObs.undef()){
    ost <<"Observationtime inavlid (KLOBS): <" << buf << ">!" << endl;
    LOGERROR(ost.str());
    
    return miTime();
  }

  if(!getInData("termin",buf)){    //synop termin ie. hour
    ost << "4:  Internal error!";
    LOGERROR(ost.str());
    return miTime();
  }

  hour=atoi(buf.c_str());

  if(hour<0 || hour>23){
    ost << "Invalid termin (" << hour << ")!";
    LOGERROR(ost.str());
    return miTime();
  }

  if((hour%3)!=0){
    ost << "Termin not a SYNOP time, ie 0, 3, 6, 9, 12, 15, 18 or 21!" <<
      "Termin: " << hour << endl;
    LOGERROR(ost.str());
    return miTime();
  }
  
  if(klComObs.undef()){
    fromTime=miTime::nowTime();
  }else{
    fromTime=klComObs;
  }

  toTime=fromTime;
  toTime.addHour(3);
  fromTime.addDay(-6);

  if(klObs>=fromTime && klObs<=toTime){
    ost << "observationtime (KLOBS) not in valid time interval: " << klObs
	<< "lower limit: " << fromTime << endl 
	<< "upper limit: " << toTime << endl;
    LOGERROR(ost.str());
    return obstime;
  }
  
  obstime=miTime(); //Undef

  if(hour==0){
    if(klObs.hour()>=21 && klObs.hour()<24){
      obstime=klObs;
      obstime.addDay(1);
      obstime=miTime(obstime.date(), miClock(0, 0, 0));
    }else if(klObs.hour()>=0 && klObs.hour()<=3){
      obstime=klObs;
      obstime=miTime(klObs.date(), miClock(0, 0, 0));
    }
  }else{
    int l=hour-3;
    int h=hour+3;
   
    if(klObs.hour()>=l && klObs.hour()<=h){
      obstime=miTime(klObs.date(), miClock(hour, 0, 0));
    }
  }

  if(obstime.undef()){
    ost << "The observation time is not within �3 of the termin! " << endl
	<< "Observationime: " << klObs << endl 
	<< "Termin: " << hour << endl;
    LOGERROR(ost.str());
  }

  return obstime;
		 
}


bool
kvalobs::decoder::comobsdecoder::
Sms12::
doPrecip(kvalobs::decodeutil::DecodedDataElem &data,
	 int hour)
{
  string RR;
  int    iTr;
  string irix;
  string sIr;
  string sIx;
  
  if(!getInData("RR", RR)){
    LOGERROR("doPrecip (1): Internal error!");
    return false;
  }


  if(!getInData("_irix", irix)){
    LOGERROR("doPrecip (3): Internal error!");
    return false;
  }

  while(!RR.empty() && RR[0]==' ')
    RR.erase(0, 1);
 
  while(!irix.empty() && irix[0]==' ')
    irix.erase(0, 1);
 

  if(irix.length()==2){
    sIr=irix.substr(0, 1);
    sIx=irix.substr(1, 1);
    
    if(isdigit(sIx[0])){
      data.addData("IX", sIx);
    }
  }
 
  if(RR.empty() && !sIr.empty() && (sIr[0]=='3' || sIr[0]=='1')){
    RR="-1";
  }else if(!RR.empty()){
    if(RR[0]=='-' || RR[0]=='+'){
      if(RR.length()>1){
	if(RR[1]=='.')
	  RR.insert(1, "0");
      }else{
	return false;
      }
    }
  }else{
    return true;
  }

  iTr=cfInfo.tr(hour);

  /*
   *  ITR    Nedb�r parameter
   *  -----------------------
   *    1    RR_6
   *    2    RR_12
   *    3    RR_18 
   *    4    RR_24
   *    5    RR_1
   *    6    RR_2  
   *    7    RR_3
   *    8    RR_9  
   *    9    RR_15
   */ 
    
  switch(iTr){
  case 1:
    data.addData("RR_6", RR);
    break;
  case 2:
    data.addData("RR_12", RR);
    break;
  case 3:
    data.addData("RR_18", RR);
    break;
  case 4:
    data.addData("RR_24", RR);
    break;
  case 5:
    data.addData("RR_1", RR);
    break;
  case 6:
    data.addData("RR_2", RR);
    break;
  case 7:
    data.addData("RR_3", RR);
    break;
  case 8:
    data.addData("RR_9", RR);
    break;
  case 9:
    data.addData("RR_15", RR);
    break;
    
  default:
    return false;
  }
  
  ostringstream ost;
  
  ost << iTr;
  
  data.addData("ITR", ost.str());
  return true;
}

