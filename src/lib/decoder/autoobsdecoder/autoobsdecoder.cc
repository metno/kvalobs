/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: autoobsdecoder.cc,v 1.50.2.5 2007/09/27 09:02:24 paule Exp $                                                       

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
#include <map>
#include <float.h>
#include <dnmithread/mtcout.h>
#include <sstream>
#include <boost/lexical_cast.hpp>
#include <puTools/miTime.h>
#include <miutil/commastring.h>
#include <milog/milog.h>
#include "convert.h"
#include "autoobsdecoder.h"

using namespace kvalobs::decoder::autoobs;
using namespace std;
using namespace dnmi::db;
using namespace miutil;
using namespace boost;
using namespace kvalobs;

AutoObsDecoder::
AutoObsDecoder(
	dnmi::db::Connection   &con,
   const ParamList        &params,
   const std::list<kvalobs::kvTypes> &typeList,
   const miutil::miString &obsType,
   const miutil::miString &obs, 
   int                    decoderId)
  	:DecoderBase(con, params, typeList, obsType, obs, decoderId),
    checkRet(0)
{
}

AutoObsDecoder::
~AutoObsDecoder()
{
}

miutil::miString 
AutoObsDecoder::
name() const
{
	return "AutoObsDecoder";
}


std::string 
AutoObsDecoder::
getMetaSaSdEmEi( int stationid, int typeid_, const miutil::miTime &obstime )
{
   string saSdEmEi="0000";
   
   if( obsPgm.obstime.undef() || obsPgm.obstime != obstime ){
      if( ! loadObsPgmParamInfo( stationid, typeid_, obstime, obsPgm ) ){
         IDLOGDEBUG( logid, "DBERROR: SaSdEm:  000");
         return saSdEmEi;
      }
   }
   
   kvalobs::decoder::Active state;
   
   //SA
   if( obsPgm.isActive( stationid, typeid_, 112, 0, 0, obstime, state ) ) {
      if( state == kvalobs::decoder::YES )
         saSdEmEi[0]='1';
   }

   //SD
   if( obsPgm.isActive( stationid, typeid_, 18, 0, 0, obstime, state ) ) {
         if( state == kvalobs::decoder::YES )
            saSdEmEi[1]='1';
   }
   
   //Em
   if( obsPgm.isActive( stationid, typeid_, 7, 0, 0, obstime, state ) ) {
      if( state == kvalobs::decoder::YES )
         saSdEmEi[2]='1';
   }

   //Ei (E)
   if( obsPgm.isActive( stationid, typeid_, 129, 0, 0, obstime, state ) ) {
      if( state == kvalobs::decoder::YES )
         saSdEmEi[3]='1';
   }
   
   IDLOGDEBUG( logid, "SaSdEmEi: " << saSdEmEi);
   
   return saSdEmEi;
}

long 
AutoObsDecoder::
getStationId(miutil::miString &msg)
{
   miString keyval;
   miString key;
   miString val;
   miString::size_type i;
   CommaString cstr(obsType, '/');
   long  id;

   if(cstr.size()<2){
   	msg="obsType: Invalid Format!";
      return 0;
   }
	
   if(!cstr.get(1, keyval)){
      msg="INTERNALERROR: InvalidFormat!";
   	return 0;
   }
    
   i = keyval.find('=');
  
   if (i == miString::npos) {
		msg = "obsType: <id> Invalid format!";
		return 0;
   }
    
   key = keyval.substr(0, i);
   val = keyval.substr(i + 1);
    
   val.trim();
   key.trim();
    
   if (key.empty() || val.empty()) {
		msg = "obsType: Invalid format!";
		return false;
   }
    
   id=DecoderBase::getStationId(key, val);
    
   if(id>=0)
   	return id;

   //Error

   stringstream ost;
      
   if(id==-1){
   	ost << "No station with id (" << key << "=" << val << ")";
   }else{
      ost << "No coloumn in the station table with the name: " << key << 
				 " (=" << val << ")";
   }
   
   msg=ost.str();
 
   return 0;
}


long 
AutoObsDecoder::
getTypeId(miutil::miString &msg)
{
	string keyval;
   string key;
   string val;
   string::size_type i;

   CommaString cstr(obsType, '/');
    
   if(cstr.size()<3)
      return 3;  //autoobs, this is default.

   if(!cstr.get(2, keyval))
      return -1;

   i=keyval.find("=");
    
   if(i==string::npos)
      return -1;

   key=keyval.substr(0, i);

   if(key!="type")
      return -1;
     
   val=keyval.substr(i+1);

   if(val.empty())
      return -1;

   return atoi(val.c_str());
}


char 
AutoObsDecoder::
checkObservationTime(int typeId,
							miutil::miTime tbt, 
							miutil::miTime obt)
{
	
  	const kvalobs::kvTypes *kvType=findType(typeId);

  	if(!kvType){
    	IDLOGWARN(logid, "Unknown typeid: " << typeId);
    	return 0;
  	} 
  
  	if(firstObsTime.undef())
		firstObsTime=obt;

  //It is only the first obsTime in a typeId==302 message that
  //is subject to testing for 'to late' or 'to early'
  //message. The rest must return the same result as the first 
  //obstime. 'checkRet' is used for this.
  	if(typeId==302 && firstObsTime!=obt)
    	return checkRet;

  	int diff = miTime::minDiff(tbt,obt);
  
  	if(diff > kvType->lateobs()){ //tbt>obt  (diff>=0)
    	//Used by checkObservationTime.
    	checkRet=4;
    	return 4;
  	}
 
  	if(diff < (-1*kvType->earlyobs())){ //tbt<obt (diff<0)
    	checkRet=3;
    	return 3;
  	}
  
  	checkRet=0;
  	return 0;
}


/**
 * \brief The entry point for the \em autoobsdecoder.
 *
 * It is two sources of data that this decoder decode data from,
 * ComObs and AutoObs.
 *
 * \note For Automatic Wheater Stations (AWS) with visual
 * observations, so called HYBRID stations (AWS HYBR), the 
 * observations is split into two data streams ie. we use two 
 * different typeid. The datastream is split with typeid=3 
 * for the automatic part in the observations and with typeid=6
 * for the visual part. We now it is a visual observation if the paramname
 * in the header start with a underscore (_).
 *
 * \note For pluviometer, nedb�rstasjoner med vippe teknologi, og med 
 * temperatur sensor skal temperatur m�lingene ha typeid=3. Pluviometer 
 * stasjonene kommer in med typeid=4.
 *
 * \todo This decoder is a ugly beast that is craying for an clean up.It should
 * be split up in two decoders, one for AutoObs and one for ComObs. 
 */
kvalobs::decoder::DecoderBase::DecodeResult
AutoObsDecoder::
execute(miutil::miString &msg)
{
  	const int             VISUEL_TYPEID=6;
  	const int             AWS_TYPEID=3;
  	std::vector<DataElem> elems;
  	string                tmp;
  	CommaString           data;
  	CommaString           header;
  	miTime                obstime;
  	miTime                tbtime(miTime::nowTime());
  	int                   typeId=getTypeId(msg); 
  	int                   useTypeid;
  	int                   typeidWithSave=-1;
  	string                level;
  	int                   stationid=getStationId(msg);
  	float                 fval;
  	int                   count=0;
  	int                   nExpectedParams=0;
  	int                   priority=4;
  	int                   line=0;
  	std::list<kvData>  dataList;
  	std::list<kvData>::iterator itDataList;
  	list<kvTextData>            textDataList;
  	list<kvTextData>::iterator  itTextDataList;

  	warnings = false;
  	logid.clear();

  	milog::LogContext lcontext("AutoObsDecoder");
  
  	if( stationid == 0 ){
  	   ostringstream o;
  	   ostringstream omsg;
  	   if( typeId <= 0 )
  	      o << "<NA>";
  	   else
  	      o << typeId;

    	omsg << "Decoder: " << name() << ". Format invalid. Missing stationid! typeid: " << o.str() << endl;
    	msg+="\n--- Missing stationid!";

    	ostringstream ost;
    	ost << "<obsType: [" << obsType << "]> observation: [" << obs << "]";

    	kvalobs::kvRejectdecode rejected(ost.str(), 
													tbtime, 
													name(),
				     								msg);
    
    	if(!putRejectdecodeInDb(rejected)) {
      	omsg << "Cant save rejected observation! typeid: "
      	         << o.str() << endl << "Observation:" << endl << obs;
    	} else {
    	  omsg << "Saved rejected observation! typeid: " <<  o.str();
    	}

    	LOGERROR( omsg.str() );
    	return Rejected;
  	}
  
  	if(typeId<0){
  	   ostringstream omsg;

  	   omsg << "Decoder: " << name() << ". Format invalid. Missing typeid! stationid: "
  	        << stationid << endl;
    	msg+="--- Format error in type!";

    	ostringstream ost;
    	ost << "<obsType: [" << obsType << "]> observation: [" << obs << "]";

    	kvalobs::kvRejectdecode rejected(ost.str(),
    												tbtime, 
				     								name(),
				     								msg);
    
    	if(!putRejectdecodeInDb(rejected)) {
    	   omsg << "Cant save rejected observation! stationid: "
    	        << stationid << endl << "Observation:" << endl << obs;
    	} else {
    	   omsg << "Saved rejected observation! stationid: " <<  stationid;
    	}

    	LOGERROR( omsg );
    
    	return Rejected;
  	}
  
  	IdlogHelper idlogHelper( stationid, typeId, this );
  	logid = idlogHelper.logid();

  	LOGINFO("Decoder: " << name() << ". New observation: stationid=" << stationid << " typeid: " << typeId << endl);
  	IDLOGINFO( logid, "New observation.");

  	obs.trim();
  	obs += "\n";
  
  	istringstream istr(obs);
  
  	IDLOGINFO(logid,
  	          "ObstType : " << obsType         << endl <<
  	          "Obs      : " << obs             << endl);
  
  	msg = "OK!";
  
  	if (!getline(istr, tmp)) {
    	msg = "ERROR: invalid format!";
    	kvalobs::kvRejectdecode rejected(obs, 
													tbtime, 
													name(),
				     								"Invalid format."
				     								"First line must identify the station.");
    
    	if( !putRejectdecodeInDb( rejected ) ) {
      	LOGERROR("Decoder: " << name() << ". Invalid format. Cant save rejected observation! stationid: "
      	          << stationid << " typeid: " << typeId << endl
      	          << "Observation:" << endl << obs );
      	IDLOGERROR( logid, "Invalid format. Cant save rejected observation!"
      	            << "Observation:" << endl << obs );
    	} else {
    	   LOGERROR("Decoder: " << name() << ". Invalid format. Saved rejected observation! stationid: "
    	             << stationid << " typeid: " << typeId );
    	   IDLOGERROR( logid, "Invalid format. Cant save rejected observation!"
    	               << "Observation:" << endl << obs );
    	}
    
    	return Rejected;
  	}
  
  	header.init(tmp);
  	DataConvert converter( paramList, logid );

  	while(getline(istr, tmp)){
    	dataList.clear();
    	textDataList.clear();

    	IDLOGDEBUG(logid, "Data: " << tmp << endl);
    	line++;
    
    	data.init(tmp);
    
    	if((data.size() - 1) != header.size()){
    	   warnings=true;
      	IDLOGWARN(logid, "Invalid data format: header.size=" << header.
	      			size() << endl << "                    data.size-1=" <<
	       			data.size() - 1 << endl);
      	continue;
    	}	
    
    	obstime = miTime(data[0]);
    	IDLOGDEBUG( logid, "  Data: obstime:  " << obstime << endl);
    
    	converter.resetRRRtr();
    	converter.resetSaSdEm();
   
    	for (int i = 0; i < header.size(); i++) {
      	useTypeid=typeId;
      
      	if(typeId==AWS_TYPEID){ //Is this a AWS station.
				if(!header[i].empty() && header[i].at(0)=='_'){
	  				//It is a VISUAL observation that shall be split
	  				//out into another stream, typeid.
	   			useTypeid=VISUEL_TYPEID;
				}
      	}

      	//We use the first typeid that is not an VISUAL typeid
      	//when we save the data. This does not mean that all the data
      	//is saved with this typeid, but the logic that try to identify
      	//the message use this typeid as a starting point.
      	if( typeidWithSave < 0 ) {
      	   if( useTypeid != VISUEL_TYPEID )
      	      typeidWithSave = useTypeid;
      	}

      	try {
				try{
	  				elems = converter.convert(header[i], data[i + 1], obstime);
				}
				catch(UnknownParam &ex){
	  				IDLOGERROR( logid, "Exception: UnknownParam: " << ex.what() << endl <<
		   					"---------: data: " << data[i + 1] << endl);
	  				warnings = true;
	  				continue;
				}
				catch(BadFormat &ex){
	  				IDLOGERROR( logid, "Exception: BadFormat" << ex.what() << endl <<
		   					"---------: data: " << data[i + 1] << endl);
	  				continue;
	  				warnings = true;
				}
				catch(...){
				   warnings = true;
	  				IDLOGERROR( logid, "Unknown EXCEPTION: from converter.convert!");
	  				continue;
				}
	
				ostringstream logs;
				logs << "Data in: " << header[i] << "  " << data[i + 1] <<endl;
	
				nExpectedParams+=elems.size();
	
				for (int k = 0; k < elems.size(); k++) {
	  				logs << "-- Paramid: " << elems[k].id() 
	       			  << " ("<< findParamIdInList(paramList, elems[k].id()) << ") " 
	       			  << elems[k].sVal() << "(s=" << elems[k].sensorno()
	       			  << ", l=" << elems[k].height() << ")" << endl;
	  
	  				if(isTextParam(findParamIdInList(paramList, elems[k].id()))){
	    				kvTextData d(stationid,
			 							 obstime,
			 							 elems[k].sVal(),
			 							 elems[k].id(),
			 							 tbtime,
			 							 useTypeid);
	    
	    				logs << " (TEXTDATA)" << endl;
	    				textDataList.push_back(d);

	  				}else	if(elems[k].fVal(fval)){
	      			kvData d(stationid, 
		       					obstime, 
		       					fval, 
		       					elems[k].id(), 
		       					tbtime,
		       					useTypeid,
		       					elems[k].sensorno(),
		       					elems[k].height(), 
		       					fval, 
		       					kvControlInfo(), 
		       					kvUseInfo(),
		       					"");
	    
	      			d.useinfo(7, checkObservationTime(typeId,tbtime, obstime));
	      
	      			dataList.push_back(d);
	    			}else{
	    			   warnings = true;
	      			IDLOGWARN( logid, "Cant convert param value to float <"
		      				  << elems[k].sVal() << ">");
	    			}
	  			}
	
				IDLOGDEBUG( logid, logs.str());
      	}
      	catch(std::exception & ex) {
      	   warnings = true;
				IDLOGERROR( logid, "Exception: " << ex.what() << endl <<
		 	    				"---------: data: " << data[i + 1] << endl);
      	}
    	}
    

    	DataConvert::RRRtr RRRtr;

    	if(converter.hasRRRtr(RRRtr)){
      	int   paramid;
      	float rr=RRRtr.RR(paramid, obstime);
      
      	if(rr!=FLT_MAX && paramid>0){
				try{
	  				kvData d(stationid, 
		   					obstime, 
		   					rr, 
		   					paramid, 
		   					tbtime,
		   					useTypeid,
		   					0,
		   					0, 
		   					rr, 
		   					kvControlInfo(), 
		   					kvUseInfo(),
		   					"");
	      
	      		d.useinfo(7, checkObservationTime(typeId, tbtime, obstime));
	      		IDLOGDEBUG( logid, "RRRtr: " << paramid << " -- RR: " << rr );

	      		dataList.push_back(d);

	      		paramid=12; //ITR
	      		kvData dd(stationid, 
								 obstime, 
								 RRRtr.tr, 
								 paramid,
								 tbtime,
								 useTypeid,
								 0,
								 0, 
								 RRRtr.tr, 
								 kvControlInfo(), 
								 kvUseInfo(),
								 "");
	      
	      		dd.useinfo(7, checkObservationTime(typeId, tbtime,	obstime));
	      
	      		dataList.push_back(dd);
				}
				catch(std::exception & ex) {
				   warnings = true;
	  				IDLOGERROR( logid, "Exception: " << ex.what() << endl <<
		   					   "---------: DataConvert::RRRtr: paramid"<< paramid << endl);
				}
 
      	}	
    	}
      
      std::string         sSaSdEm;
      DataConvert::SaSdEmEi saSdEm;

      converter.setSaSdEmEi( getMetaSaSdEmEi( stationid, useTypeid, obstime) );
      
      if( converter.hasSaSdEmEi( saSdEm ) ) {
         //Create a template to use
         //to hold all common parameters for SA, SD and EM.
         kvData saSdEmTmp(stationid, obstime, 
                          -32767 /*original*/, 0  /*paramid*/, tbtime,
                          useTypeid, 0 /*sensor*/, 0 /*level*/, 
                          -32767 /*corected*/, kvControlInfo(), kvUseInfo(), "");
			      
         saSdEmTmp.useinfo(7, checkObservationTime(typeId, tbtime, obstime));
         kvData saSdEmData;
			      
         if( DataConvert::SaSdEmEi::dataSa( saSdEmData, saSdEm, saSdEmTmp ) ) {
            nExpectedParams++;
            dataList.push_back(saSdEmData );
         }
				      
         if( DataConvert::SaSdEmEi::dataSd( saSdEmData, saSdEm, saSdEmTmp ) ) {
            dataList.push_back(saSdEmData );
            nExpectedParams++;
         }
         
         if( DataConvert::SaSdEmEi::dataEm( saSdEmData, saSdEm, saSdEmTmp ) ) {
            dataList.push_back(saSdEmData );
            nExpectedParams++;
         }

         if( DataConvert::SaSdEmEi::dataEi( saSdEmData, saSdEm, saSdEmTmp ) ) {
            dataList.push_back(saSdEmData );
            nExpectedParams++;
         }
		   
     	}

		if( addDataToDb( obstime, stationid, typeidWithSave, dataList, textDataList, priority, logid ) ) {
		   count += dataList.size() + textDataList.size();
		}
  	}
  
  	if(count>0){
    	if(count==nExpectedParams){
      	IDLOGDEBUG( logid, "Return from decoder: Ok!\n");
    	}else{
      	IDLOGWARN(logid, "Expected: " <<  nExpectedParams << " got: " << count );
    	}

    	if( warnings ) {
    	   LOGWARN("Decoder: " << name() << ". Data saved with warnings. stationid: " << stationid << " typeid: " << typeId);
    	} else {
    	  LOGINFO("Decoder: " << name() <<". Data saved. stationid: " << stationid << " typeid: " << typeId);
    	}

   	return Ok;
  	}else if(nExpectedParams==0){
    	LOGINFO("Decoder: " << name() << ". No data in message! stationid: " << stationid << " typeid: " << typeId );
    	return Ok;
  	}else{
    	LOGERROR("Decoder: " << name() <<". NotSaved: Data not saved! stationid: " << stationid << " typeid: " << typeId );
    	return NotSaved;
  	}
}

