/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kldecoder.cc,v 1.7.2.7 2007/09/27 09:02:29 paule Exp $                                                       

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
#include <cctype>
#include <sstream>
#include <boost/lexical_cast.hpp>
#include <puTools/miTime>
#include <miutil/commastring.h>
#include <milog/milog.h>
#include <stdlib.h>
#include <kvalobs/kvDbGate.h>
#include <kvalobs/kvQueries.h>
#include <kvalobs/kvTypes.h>
#include <miutil/trimstr.h>
#include "kldecoder.h"
#include <decodeutility/decodeutility.h>



using namespace kvalobs::decoder::kldecoder;
using namespace std;
using namespace dnmi::db;
using namespace miutil;
using namespace boost;
using namespace kvalobs;

kvalobs::decoder::kldecoder::
KlDecoder::
KlDecoder( dnmi::db::Connection   &con,
	  	   const ParamList        &params,
	  	   const std::list<kvalobs::kvTypes> &typeList,
	       const miutil::miString &obsType,
	       const miutil::miString &obs, 
	       int                    decoderId)
  	:DecoderBase(con, params, typeList, obsType, obs, decoderId)
{
}

kvalobs::decoder::kldecoder::
KlDecoder::
~KlDecoder()
{
}

std::string
kvalobs::decoder::kldecoder::
KlDecoder::
toupper(const std::string &s_){
	string s(s_);
	
	for(string::size_type i=0; i<s.length(); i++)
		s[i]=std::toupper(s[i]);
		
	return string(s);
}

miutil::miString 
kvalobs::decoder::kldecoder::
KlDecoder::
name() const
{
    return "KlDataDecoder";
}

bool
kvalobs::decoder::kldecoder::
KlDecoder::
saveData(list<kvalobs::kvData> &data, 
		       bool &rejected, 
		       std::string &rejectedMessage)
{
	list<kvalobs::kvData>::iterator it=data.begin();
    int i=0;
    rejected=false;
   
    for(;it!=data.end(); it++){
      	if(it->obstime().undef() || it->tbtime().undef()){
			rejectedMessage="Missing obsTime or tbtime for observation!";
			rejected=true;
			continue;
      	}
	
      	if(!putKvDataInDb(*it)){
			LOGWARN("Cant save data:\n" << it->toSend());
      	}else{
			i++;
      	}
	}
     
    return i!=0 && !rejected;
}

kvalobs::decoder::DecoderBase::DecodeResult 
kvalobs::decoder::kldecoder::
KlDecoder::
execute(miutil::miString &msg)
{
	list<kvalobs::kvData> data;
	miTime                nowTime(miTime::nowTime());
	string                tmp;
	miTime                obstime;
	miTime                tbtime(nowTime);
	int                   typeId=getTypeId(msg); 
	string                level;
	int                   stationid=getStationId(msg);
	float                 fval;
	string                val;
	int                   count=0; //Saved data records
	int                   nErrors=0; //Failed to save data records. 
	int                   nExpectedData=0;
	vector<ParamDef>      params;
	int                   lines=1;  //Total lines.
   int                   nLineWithData=0; //Number of line with data.
   int                   nElemsInLine;
	int                   priority=10;
	
	milog::LogContext lcontext(name());
	  
	LOGINFO("New observation: stationid=" << stationid << endl <<
	    	"----------------    typeid=" << typeId);
  
  	if(stationid== 0){
    	LOGERROR("Missing stationid!\nSaved in table rejectdecode!");
    	msg="Missing stationid!";
    	kvalobs::kvRejectdecode rejected(obs, 
									     tbtime, 
				     					 name(),
				     				     msg);
    
    	if(!putRejectdecodeInDb(rejected))
      		LOGERROR("Cant save rejected observation!");
    
    	return Rejected;
  	}
  
  	if(typeId<0){
    	LOGERROR("Format error in type!\nSaved in table rejectdecode!");
    	msg="Format error in type!";
    	kvalobs::kvRejectdecode rejected(obs, 
									     tbtime, 
				 						 name(),
				    					 msg);
    
    	if(!putRejectdecodeInDb(rejected))
      		LOGERROR("Cant save rejected observation!");
    
    	return Rejected;
  	}
   
  	obs.trim();
  	obs += "\n";
  
  	istringstream istr(obs);
  
  	LOGINFO(name()                           << endl <<
		    "------------------------------" << endl <<
	  		"ObstType : " << obsType         << endl << 
	  		"Obs      : " << obs             << endl);
  
  	msg = "OK!";
  
  	if (!getline(istr, tmp)) {
    	msg="Inavlid format. No data?!!";
    	return Error;
  	}else{
		/* OBS! skip the header part */
	    getline(istr,tmp);
    	if(!decodeHeader(tmp, params, msg)){
      		LOGERROR("INVALID header:" << endl << tmp);
      		return Error;
    	}
  	}

  	if(params.size()<1){
    	msg="No parameters in header!";
    	LOGINFO("No parameters in header!" << endl << "Header: " << tmp);
    	return Ok; 
  	}

  	string::size_type i;
  	
  	while(getline(istr, tmp)){
    	lines++;
    	i=tmp.find_first_of(",");
    	obstime = miTime(tmp.substr(0, i)); 
    	tmp.erase(0, (i==string::npos?i:i+1));
  
    	KlDataArray da;

    	if(!decodeData(da, params.size(), tmp, lines, msg)){
      		LOGERROR("Cant decode the data. Reason: " << endl << msg);
      		return Error;
    	}
    
    	ostringstream ost;
    	ost << "[" << tmp << "]" << endl;
    	
    	for(KlDataArray::size_type index=0; index<da.size(); index++)
     	   ost << params[index].name() << "("<< params[index].id() << ")[" <<
			       params[index].sensor() << "," << params[index].level() << "]=("<<
				    da[index].val()<<"," << da[index].cinfo()<<"," << da[index].uinfo() << 
				    endl;
    
    	LOGDEBUG3(ost.str());

      nElemsInLine=0;
      
    	for(KlDataArray::size_type index=0; index<da.size(); index++){
      		KlData data=da[index];
     
     		if(data.empty())
				continue;
            
         nExpectedData++;
         nElemsInLine++;
            
         string val=data.val();

      	if(isTextParam(params[index].id())){
				kvTextData d(stationid,
		     				 obstime,
		     				 val,
		     				 params[index].id(),
		     				 tbtime,
		     				 typeId);
	    
		
				if(!putkvTextDataInDb(d, priority)){
	  				LOGERROR("Can't save textdata. Stationid: " 
		   					 << stationid <<"\n");
	  				nErrors++;
				}else{
	  				count++;
				}
      	}else{
				if(params[index].code()){
	  				if(params[index].name()=="VV"){
	    				val=decodeutility::VV(val);
	  				}else if(params[index].name()=="HL"){
	    				val=decodeutility::HL(val);
	  				}else{
	    				LOGWARN("Unsupported as code value: " << params[index].name());
	    				continue;
	  				}
				}

				try{
	  				fval=lexical_cast<float>(val);
				}
				catch(...){
	  				LOGERROR("Invalid value: (" << val << ") not a float!");
	  				continue;
				}
	
				kvData d(stationid, 
			 			 obstime, 
				 		 fval, 
				 		 params[index].id(), 
				 		 tbtime,
				 		 typeId,
				 		 params[index].sensor(),
				 		 params[index].level(), 
				 		 fval, 
				 		 data.cinfo(), 
				 		 data.uinfo(),
				 		 "");


				//LOGDEBUG("kvData:" << endl << "The struct: " << d.toSend());

				if(!putKvDataInDb(d, priority)){
	  				LOGERROR("Can't save data. Stationid: " 
		  	 		 		 << stationid <<"\n");
	  				nErrors++;
				}else{
	  				count++;
				}
      	}
    	}
      
      if(nElemsInLine>0)
         nLineWithData++;
  	}
  
  	ostringstream ost;
   ost <<  "# Lines:             " << lines-1 << endl <<
           "# Lines with data:   " << nLineWithData << endl <<
           "# dataelements:      " << nExpectedData << endl <<
		     "# Saved datarecords: " << count <<  endl <<
	  		  "# Error in save:     " << nErrors;
           
   msg=ost.str();
   
   LOGDEBUG(msg);

  	if(lines==1 || (count==0 && nExpectedData==0)){
    	msg+="No data!";
    	return Ok;
  	}

  	if(count>0){
      if(nExpectedData!=count){
         ostringstream  ost;
         ost << "WARNING: Expected to save " << nExpectedData 
             << " dataelements, but only "  << count 
             << " dataelements was saved!";
         
    	   msg+=ost.str();
      }
    	return Ok;
  	}

  	return Rejected;

}


long 
kvalobs::decoder::kldecoder::
KlDecoder::
getStationId(miutil::miString &msg)
{	
	miString keyval;
    miString key;
    miString val;
    miString::size_type i;
    //CommaString cstr(obsType, '/');
    CommaString cstr(obs, '/');
    long  id;
    
    if(cstr.size()<2){
      	msg="obs: Invalid Format!";
      	return 0;
    }

    if(!cstr.get(1, keyval)){
       	msg="INTERNALERROR: InvalidFormat!";
      	return 0;
    }
    
    i = keyval.find('=');
  
    if (i == miString::npos) {
		msg = "obs: <id> Invalid format!";
		return 0;
    }
    
    key = keyval.substr(0, i);
    val = keyval.substr(i + 1);
    
    val.trim();
    key.trim();
    
    if (key.empty() || val.empty()) {
		msg = "obs: Invalid format!";
		return false;
    }
    
    id=DecoderBase::getStationId(key, val);
    
    if(id>=0)
      	return id;

    //Error

    stringstream ost;
      
    if(id==-1){
      	ost << "Now station with id (" << key << "=" << val << ")";
      
    }else{
      	ost << "Now coloumn in the station table with the name: " << key << 
			   " (=" << val << ")";
    }
    msg=ost.str();
 
    return 0;
}


long 
kvalobs::decoder::kldecoder::
KlDecoder::
getTypeId(miutil::miString &msg)const
{
    string keyval;
    string key;
    string val;
    string::size_type i;

    //CommaString cstr(obsType, '/');
	CommaString cstr(obs, '/');
    
    if(cstr.size()<3){
      	msg="To few element in, expecting 3. <obs>(" + obs + ")!";
      	return -1;  
    }

    if(!cstr.get(2, keyval))
      	return -1;

    i=keyval.find("=");
    
    if(i==string::npos){
      	msg="Invalid format <obs>(" + obs + ")!";
      	return -1;
    }

    key=keyval.substr(0, i);

    if(key!="type"){
      	msg="Invalid format, expecting <type>. <obs>(" + obs + ")!";
      	return -1;
    }
     
    val=keyval.substr(i+1);

    if(val.empty()){
      	msg="Invalid format, no value for <type>. <obs>(" + obs + ")!";
      	return -1;
    }
      
    return atoi(val.c_str());
}

bool 
kvalobs::decoder::kldecoder::
KlDecoder::
splitParams(const std::string &header, 
	    std::list<std::string> &params,
	    miutil::miString &msg)
{
  	string param;
  	string::size_type iEnd=0;
  	string::size_type i;

  	params.clear();

 	while(iEnd!=string::npos){
    	i=iEnd;
    	iEnd=header.find_first_of(",(", i);
    
    	if(iEnd==string::npos){
      		param=header.substr(i);
    	}else if(header[iEnd]==','){
      		param=header.substr(i, iEnd-i);
      		iEnd++;
    	}else{
      		//header[iEnd]=='('
      		int n=0; //Count of commas (,).
      
      		iEnd=header.find_first_of(",)", iEnd);
      
      		while(header[iEnd]==','){
				iEnd=header.find_first_of(",)", iEnd+1);
				n++;
      		}
      
      		if(iEnd==string::npos){
				msg="Invalid parameter format: missing ')'!";
				return false;
      		}
      
      		if(n!=1){
				msg="Invalid parameter format: Expecting one comma only in optional part!";
				return false;
      		}
      
      		iEnd=header.find_first_of(",", iEnd+1);
      
      		if(iEnd==string::npos){
				param=header.substr(i);
      		}else{ //iEnd==','
				param=header.substr(i, iEnd-i);
				iEnd++;
      		}
    	}

    	params.push_back(param);
  	}

  	return true;
}


bool 
kvalobs::decoder::kldecoder::
KlDecoder::
splitData(const std::string &sdata, 
	    std::list<std::string> &datalist,
	    miutil::miString &msg)
{
  	string val;
  	string::size_type iEnd=0;
  	string::size_type i;

  	datalist.clear();

  	while(iEnd!=string::npos){
    	i=iEnd;
    	iEnd=sdata.find_first_of(",(", i);
    
    	if(iEnd==string::npos){
      		val=sdata.substr(i);
    	}else if(sdata[iEnd]==','){
      		val=sdata.substr(i, iEnd-i);
      		iEnd++;
    	}else{
      		iEnd=sdata.find_first_of(",)", iEnd);
      
      		while(sdata[iEnd]==',')
				iEnd=sdata.find_first_of(",)", iEnd+1);
      
      		if(iEnd==string::npos){
				msg="Invalid parameter format: missing ')'!";
				return false;
      		}
     
      		iEnd=sdata.find_first_of(",", iEnd+1);
      
      		if(iEnd==string::npos){
				val=sdata.substr(i);
      		}else{ //iEnd==','
				val=sdata.substr(i, iEnd-i);
				iEnd++;
      		}
    	}

    	datalist.push_back(val);
  	}

  	return true;
}


bool 
kvalobs::decoder::kldecoder::
KlDecoder::
decodeHeader(const std::string &header, 
	     std::vector<ParamDef> &params,
	     miutil::miString &msg)
{
	string::size_type i;
	string::size_type iEnd=0;
	string            param;
	string            name;
	string            buf;
	int               sensor;
	int               level;
	bool              isCode;
	IParamList        it;
	list<string>      paramStrings;
	ostringstream    ost;

  	params.clear();
  
  	if(!splitParams(header, paramStrings, msg))
    	return false;
  
  	list<string>::iterator itParamsStrings=paramStrings.begin();

  	ost << "ParamStrings: " << endl;
  
  	for(;itParamsStrings!=paramStrings.end(); itParamsStrings++)
    	ost << " [" << *itParamsStrings << "]";

  	LOGDEBUG(ost.str());
 
  	itParamsStrings=paramStrings.begin();

  	for(;itParamsStrings!=paramStrings.end(); itParamsStrings++){
    	param=*itParamsStrings;
    
    	trimstr(param);
    	sensor=0;
    	level=0;

    	i=param.find_first_of("(", 0);
      
    	if(i==string::npos){
      		name=param;
    	}else{
      		name=param.substr(0, i);
      		trimstr(name);
      		iEnd=param.find_first_of(")", i);
      
      		if(iEnd==string::npos){ //paranoia
				msg="Invalid format: missing ')' in param [" +name +"]";
				return false;
      		}

      		i++;     
      		param=param.substr(i, iEnd-i);
      
      		CommaString cs(param);
      
      		if(cs.size()!=2){//paranoia
				msg="Invalid format: wrong number of parameteres in optional part of"+
	  			string(" param  [") +name +"]";
				return false;
      		}

      		cs.get(0, buf);
      		sensor=atoi(buf.c_str());
      		cs.get(1, buf);
      		level=atoi(buf.c_str());
    	}

    	if(name.empty())
      		return false;
    
    	if(name[0]=='_'){
      		isCode=true;
      		name.erase(0, 1);
      
      		if(name.empty()){
				msg="Invalid parameter format: paramname missing!";
				return false;
      		}
    	}else{
      		isCode=false;
    	}

    	it=paramList.find(Param(toupper(name), -1));
    
    	if(it==paramList.end()){
      		params.push_back(ParamDef(name, -1, sensor, level, isCode));
    	}else{
      		params.push_back(ParamDef(name, it->id(), sensor, level, isCode));
    	}
  	}
  
  	return true;
}

bool 
kvalobs::decoder::kldecoder::
KlDecoder::
decodeData(KlDataArray &da, 
	   	   KlDataArray::size_type daSize,
	   	   const std::string &sdata,
	   	   int line,
	   	   miutil::miString &msg)
{
  	string::size_type      i;
  	string::size_type      iEnd;
  	list<string>           dtmp;
  	list<string>::iterator it;
  	ostringstream          ost;
  	string                 buf;
    
  	if(!splitData(sdata, dtmp, msg)){
    	LOGERROR("decodeData: " << msg << endl);
    	return false;
  	}

  	if(daSize!=dtmp.size()){
    	ost.str("");
    	ost << "decodeData: expected #Data: " << daSize << endl 
			<< "Found in datastring #: " << dtmp.size() << " line: " << line;
    	LOGERROR(ost.str());
    	msg=ost.str();
    	return false;
  	}


  	for(it=dtmp.begin();it!=dtmp.end(); it++)
    	ost << " [" << *it<< "]";

  	LOGDEBUG("decodeData: Data in string: " << endl << 
			 "[" << sdata<< "]" << endl <<
			 ost.str());


  	da=KlDataArray(daSize);
  	KlDataArray::size_type index=0;

  	for(it=dtmp.begin(); it!=dtmp.end(); it++){
    	string val;
    	kvControlInfo c;
    	kvUseInfo     u;    
    
    	buf=*it;
       	trimstr(buf);
    	i=buf.find_first_of("(", 0);
      
    	if(i==string::npos){
      		val=buf;
   	 	}else{
      		val=buf.substr(0, i);
      		trimstr(val);
      		iEnd=buf.find_first_of(")", i);
      
      		if(iEnd==string::npos){ //paranoia
				ost.str("");
				ost << "Invalid format: missing ')' in data ["+buf+"]"
	    			<< " at index: " << index << " line: " << line;
				msg=ost.str();
				LOGERROR("decodeData: " << ost.str());
				return false;
      		}

      		i++;     
      		buf=buf.substr(i, iEnd-i);
      
      		CommaString cs(buf);
      
      		if(cs.size()==0 || cs.size()>2){//paranoia
				ost.str("");
				ost << "Invalid format: wrong number of values in" <<
	  				   " optional part of data element: " << index << " line: " << line;
						msg=ost.str();
				LOGERROR("decodeData: " << ost.str());
				return false;
    		}
      
    		cs.get(0, buf);
      
    		if(buf.length()==16){ 
				c=kvControlInfo(buf);
    		}else if(buf.length()>0){
				ost.str("");
				ost << "Expected 16 character in <controlinfo>: "
	   				<< "found " << buf.length() << " characters at index: " 
	   				<< index << " line: " << line;
				msg=ost.str();
				LOGWARN("decodeData: " << ost.str());
    		} 
      
    		if(cs.size()==2){
				cs.get(1, buf);
	
				if(buf.length()==16){
					u=kvUseInfo(buf);
				}else if(buf.length()>0){
					ost.str("");
					ost << "Expected 16 character in <useinfo>: "
	   					<< "found " << buf.length() << " characters at index: " 
	   					<< index << " line: " << line;
					msg=ost.str();
					LOGWARN("decodeData: " << ost.str());
				}
    		}
    	}
      
    	if(!val.empty())
    		da[index]=KlData(val, c, u);
        
    	index++;
  	}
  
  	return true;
} 
