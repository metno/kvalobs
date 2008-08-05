/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: convert.cc,v 1.51.2.7 2007/09/27 09:02:24 paule Exp $                                                       

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
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <boost/lexical_cast.hpp>
#include <string.h>
#include "convert.h"
#include <milog/milog.h>
#include <puTools/miTime>
#include <dnmithread/mtcout.h>
#include <sstream>
#include <decodeutility/decodeutility.h>

#define KNOPFAKTOR     1.94384449244
//using namespace kvalobs::decoder::autoobs;
using namespace boost;
using namespace std;
using namespace miutil;

namespace kvalobs { 
   namespace decoder {
      namespace autoobs {
      
      extern SplitData      splitArray[];        //defined in SplitDef.cc
      extern NameConvertDef nameConvertDefArray[]; //defined in NameConvertDef.cc 
      extern NameDef        DontDecodeParamArray[];//defined in DontDecodeParams.cc
      extern NameDef        TextParams[];  //Defines which parameters that is 
                                           //Text data and shall in the 
                                           //text_data table.
      DataElem::DataElem(int   id, 
			                const std::string &val,
			                int   sensorno,
			                int   height,
			                int   mod)
	     :id_(id), val_(val), sensorno_(sensorno), 
	      heigth_(height),mod_(mod){
      }
      
      bool
      DataElem::fVal(float &f)const{
	     try{
	        f=lexical_cast<float>(val_);
	        return true;
	     }
	     catch(...){
	        return false;
	     }
      }
      
      DataElem& 
      DataElem::operator=(const DataElem &p){
	     if(this==&p)
	        return *this;
	
	     id_=p.id_;
	     val_=p.val_;
	     heigth_=p.heigth_;
	     mod_=p.mod_;
	     sensorno_=p.sensorno_;

	     return *this;
      }
      

      bool 
      DataConvert::SaSdEm::
      dataSa( kvData &data, const SaSdEm &sa, const kvData &saSdEmTemplate  ) 
      {
         if( sa.sa.empty() )
            return false;
         
         //int v = atoi( sa.sa.c_str() );
         float v;
         
         if( sscanf( sa.sa.c_str(), "%f", &v) != 1 )
         		return false;
         
         data = kvData( saSdEmTemplate.stationID(), saSdEmTemplate.obstime(), 
                        v /*original*/, 112  /*paramid*/, saSdEmTemplate.tbtime(),
                        saSdEmTemplate.typeID(), saSdEmTemplate.sensor(), saSdEmTemplate.level(), 
                        v /*corected*/, saSdEmTemplate.controlinfo(), saSdEmTemplate.useinfo(), 
                        saSdEmTemplate.cfailed() );
         
         return true;
         
      }
                  
      
      bool
      DataConvert::SaSdEm::
      dataSd( kvData &data, const SaSdEm &sd, const kvData &saSdEmTemplate  ) 
      {
         if( sd.sd.empty() )
            return false;
         
         int v = atoi( sd.sd.c_str() );
                  
         data = kvData( saSdEmTemplate.stationID(), saSdEmTemplate.obstime(), 
                        v /*original*/, 18  /*paramid*/, saSdEmTemplate.tbtime(),
                        saSdEmTemplate.typeID(), saSdEmTemplate.sensor(), saSdEmTemplate.level(), 
                        v /*corected*/, saSdEmTemplate.controlinfo(), saSdEmTemplate.useinfo(), 
                        saSdEmTemplate.cfailed() );
                  
         return true;
      }
      
      bool 
      DataConvert::SaSdEm::
      dataEm( kvData &data, const SaSdEm &em, const kvData &saSdEmTemplate  )
      {
         if( em.em.empty() )
            return false;
                  
         int v = atoi( em.em.c_str() );
                  
         data = kvData( saSdEmTemplate.stationID(), saSdEmTemplate.obstime(), 
                        v /*original*/, 7  /*paramid*/, saSdEmTemplate.tbtime(),
                        saSdEmTemplate.typeID(), saSdEmTemplate.sensor(), saSdEmTemplate.level(), 
                        v /*corected*/, saSdEmTemplate.controlinfo(), saSdEmTemplate.useinfo(), 
                        saSdEmTemplate.cfailed() );
                  
         return true;
      }
      
      DataConvert::DataConvert(ParamList &p)
         :  paramList(p), hasRRRtr_(false), 
            hasSa( false ), hasSd( false ), hasEm( false ) 
      {
      }

      void DataConvert::setSaSdEm( const std::string &sa_sd_em)
      {
         if( sa_sd_em.length() != 3 ) {
            LOGDEBUG("DataConvert::setSaSdEm: sa_sd_em.length()!=3: (" << sa_sd_em.length() << ")" );
            return;
         }
         
         hasSa = false;
         hasSd = false;
         hasEm = false;
         
         if( sa_sd_em[0] == '1')
            hasSa = true;
         
         if( sa_sd_em[1] == '1')
            hasSd = true;
         
         if( sa_sd_em[2] == '1')
            hasEm = true;
      }
      
      bool 
      DataConvert::allSlash(const std::string &val){
         return allCh( val, '/' );
      }

      bool 
      DataConvert::allCh(const std::string &val, char ch){
        std::string::const_iterator it=val.begin();
   
        for(;it!=val.end(); it++){
           if(*it != ch)
              return false;
        }
   
        return true;
      }



      std::vector<DataElem>
      DataConvert::convert(const std::string &param_,
			                  const std::string &val,
	     	                  const miutil::miTime   &obsTime){
         SplitDef              *spDef;
      	std::string     param(param_);
      	int                   totSize;
      	std::vector<DataElem> data;
      	std::string      paramName;
      	int                   sensor=1;
      	int                   mod=-1;
      	int                   height=0;
      	int                   slashCount=0;
	
      	if( param=="SA")
      	   saSdEm_.hasSa = true;
      	else if( param=="EM" )
      	   saSdEm_.hasEm = true;
      	else if( param=="SD")
      	   saSdEm_.hasSd = true;
      	else if( param == "_Esss") {
      	   saSdEm_.hasSa = true;
      	   saSdEm_.hasEm = true;
      	}
      	
         if(val.empty()) 
            return data;
	
         for(std::string::const_iterator it=val.begin();
	          it!=val.end(); 
	          it++){
            if(*it=='/')
               slashCount++;
	      }
	
	      if(val.length()==slashCount ) 
	         return data;
	
	      if(!decodeParam(param, paramName, sensor, height, mod))
	         BadFormat("Bad parameter format. (Inavilid coding of parameter name)");

	      spDef=findSplitDef(paramName, totSize);

	      if(spDef){
	         if(val.length()!=totSize && totSize>=0)
	            throw BadFormat("Size mismatch.");
	  
	         if(!decodeSpDef(data, spDef, paramName, val, sensor, height, mod))
	            throw BadFormat("Size mismatch.");
	      }else{
	         //May throw UnknownParam
	         decodeVal(data, obsTime, paramName, val, sensor, height, mod);
	      }
	
	      return data;
      }


      
      /**
       * Parse a param on the form:
       *
       *   (xN)(YY)(Z)(_HHH)(_PP)
       *  
       *  Where N -�is sensor number N (optional).
       *       YY - is a parameter name.
       *        Z - modifies YY as follows
       *            S - sum
       *            M - middel
       *            X - max
       *            N - min
       *            D - median.
       *      HHH - height, if it deviate from standard. (optional)
       *       PP - Period, if it deviate from standard. (optional)
       *
       * ex. Maximum temperatur in 2 meter TAX. Temperatur in 10 meter, 
       * TAX_010.  
       *
       * \note sensors in kvalobs is numbred from 0.Ie a parameter without a 
       *       sensor number or a sensor number of 1 get sensor=0, a parameter
       *       with sensor number of 2 get sensor=1, etc...
       *
       */
      bool 
      DataConvert::decodeParam(const std::string &param,
		                         std::string       &name,
			                      int               &sensor,
			                      int               &height,
			                      int               &mod){
	      string par(param);
	      string::size_type i;
	      string tmp;

	      sensor=0;
	      height=0;
	      mod=0;
	      name.erase();

	      if(param.length()==0)
	         return false;
	
	      if(param[0]=='_'){
	         name=param;
	         return true;
	      }
   
	
	      if(param=="FM2"){
	         name="FM";
	         sensor=1;
	         return true;
	      }
	    
	      if(param=="FG2"){
	         name="FG_1";
	         sensor=1;
	         return true;
	      }
	
	      if(param=="FX2"){
	         name="FX_1";
	         sensor=1;
	         return true;
	      }
	

	      /* 2005.04.06
	       * Bxrge Moe
	       * Decode the ground temperature at diffrent 
	       * depth. 
	       */
	      i=param.find("TJM");
	
	      if(i!=string::npos){
	         string mypar(param);
	         int    mysensor=sensor;

	         if(mypar[0]=='x' || isdigit(mypar[0])){
	            if(mypar[0]=='x')
	               mypar.erase(0, 1);
	  
	            if(isdigit(mypar[0])){
	               mysensor=mypar[0]-'0';
	               mypar.erase(0, 1);
	            }
	    
	            if(mysensor>0)
	               mysensor--;
	         }
	  
	         if(mypar.length()==3){
	            name=mypar;
	            sensor=mysensor;
	            return true;
	         }

	         string myheight=mypar.substr(3);
	  
	         for(i=0;i<myheight.length() && isdigit(myheight[i]); i++);
	  
	         if(i==myheight.length()){
	            height=atoi(myheight.c_str());
	            name=mypar;
	            sensor=mysensor;
	            return true;
	         }

	        //Fall trough and try again to decode the param. 
	     }

	     if(par.length()>=2){
	        if(par[0]=='x' || isdigit(par[0])){
	           if(par[0]=='x')
	              par.erase(0, 1);
	  
	           if(isdigit(par[0])){
	              sensor=par[0]-'0';
	              par.erase(0, 1);
	           }
	    
	           /* 2004.11.30
	            * Bxrge Moe
	            * Correct sensor number so they ar based on 0.
	            */
	           if(sensor>0)
	              sensor--;
	       }
	     }
	
	     //Check if we have a special param name from DontDecodeParamArray.
	
	     for(int ii=0; DontDecodeParamArray[ii].name; ii++){
	        i=par.find(DontDecodeParamArray[ii].name);
	  
	        if(i==0){
	           if(par.length()==strlen(DontDecodeParamArray[ii].name)){
	              name=par;
	              return true;
	           }else if(par[strlen(DontDecodeParamArray[ii].name)]=='_'){
	              name=DontDecodeParamArray[ii].name;
	              par.erase(0,strlen(DontDecodeParamArray[ii].name)+1);
	              break;
	           }
	        }
	     }

	     if(name.empty()){
	        //We have not a special name, decode it.
	        i=par.find_first_of("_");
	  
	        if(i==string::npos){
	           name=par;
	           return true;
	        }
	
	        name=par.substr(0, i);
	        par.erase(0, i+1);
	     }
	
	     i=par.find_first_of("_");
	
	     if(i==string::npos){
	        tmp=par;
	        par.erase();
	     }else{
	        tmp=par.substr(0, i);
	        par.erase(0, i+1);
	     }

	     i=tmp.find_first_not_of("0123456789");
	
	     if(i!=string::npos)
	        return false;
	
	     if(tmp.length()==3)
	        height=atoi(tmp.c_str());
   	  else
	        mod=atoi(tmp.c_str());
	
	     if(par.length()==0)
	        return true;
 	
	     i=par.find_first_not_of("0123456789");
	
	     if(i!=string::npos)
	        return false;
	
	     mod=atoi(par.c_str());
	
	     return true;
      }
      
      bool
      DataConvert::decodeSpDef(std::vector<DataElem> &data,
			                      SplitDef              *spDef,
			                      const std::string     &param,
			                      const std::string     &val,
			                      int                   sensor,
			                      int                   height,
			                      int                   mod){
         IParamList it;
	      string   spVal;
	      string   id;
	      int      _RRR=-1;
	      float    RRR=-1.0; //Acumulated precipitation.
	      int      tr=-1;    //Number of  6 hours  interval, 
	      int      Rt=0;    //1/10 precipitation
	
	      for(int i=0; spDef[i].id; i++){
	         id=spDef[i].id;
	  
	         if(id.empty())
	            continue;
	         
	         if(spDef[i].index>=0)
	            spVal=val.substr(spDef[i].index, spDef[i].size);
	         else
	            spVal=val;
	   
	         if(allSlash(spVal))
	            continue;
	  
	         if(id=="_Rt"){
	            RRRtr_.rt=atoi(spVal.c_str());
	            LOGDEBUG("-- _Rt (" << spVal << ") : " <<  RRRtr_.rt);
	            continue;
	         }else if(id=="_RRR"){
	            if(spVal.length()>=3){
	               if(isdigit(spVal[0]) && isdigit(spVal[1]) && isdigit(spVal[2])){
		               if(spVal[0]=='9' && spVal[1]=='9'){
		                  RRRtr_.rt=spVal[2]-'0';
		                  RRRtr_.RRR=0;
		                  LOGDEBUG("-- _RRR (" << spVal << ") : " << RRRtr_.RRR <<
			                        " rt: " << RRRtr_.rt);
		               }else if(spVal=="000"){
		                  RRRtr_.RRR=-1;
		               }else{
		                  RRRtr_.RRR=atoi(spVal.c_str());
		                  LOGDEBUG("-- _RRR (" << spVal << ") : " << RRRtr_.RRR );
		               }
				
		               hasRRRtr_=true;
	               }
	            }
	            continue;
	         }else if(id=="_tr"){
	            RRRtr_.tr=atoi(spVal.c_str());
	            continue;
	         }else if(id=="IR"){
	            RRRtr_.ir=atoi(spVal.c_str());
	            hasRRRtr_=true;
	         }else if(id=="VV"){
	            if(spVal.empty())
	               continue;
	    
	            spVal=decodeutility::VV(spVal);
	         }else if(id=="HL"){
	            if(spVal.empty() || spVal.length()!=1)
	               continue;
	    
	            spVal=decodeutility::HL(spVal);
	         }else if(id=="FX" || id=="FG" || id=="FF"){ 
	            //In knop from AutoObs 
	            ostringstream ost;
	            float f;
	    
	            try{
	               f=lexical_cast<float>(spVal);
	            }
	            catch(...){
	               LOGERROR("Format error (FX): (" << spVal << "). Not a number!");
	               continue;
	            }
	    
	            f/=KNOPFAKTOR;  //To m/s
	            ost << f;
	            spVal=ost.str();
	         }else if(id=="DD"){
	           //In deg with 10 deg resolution  from AutoObs 
	            ostringstream ost;
	            int i;
	    
	            try{
	               i=lexical_cast<int>(spVal);
	            }
	            catch(...){
	               LOGERROR("Format error (DD): (" << spVal << "). Not a number!");
	               continue;
	            }
	    
	            i*=10;  //To deg
	            ost << i;
	            spVal=ost.str();
	         }else if(id=="TW"){
	            if(spVal.length()!=4)
	               continue;
	    
	            float tw;
	    
	            try{
	               tw=lexical_cast<float>(spVal.substr(1));
	            }
	            catch(...){
	               LOGERROR("Format error (TW): (" << spVal << "). Not a number!");
	               continue;
	            }
	    
	            if(spVal[0]=='1')
	               tw*=-1;
	    
	            tw/=10;
	    
	            try{
	               spVal=lexical_cast<string>(tw);
	            }
	            catch(...){
	              //This shall newer happend, but you never know ....
	               LOGERROR("Cant convert float (TW): " << tw << " to string!");
	               continue;
	            }
	         }else if(id=="V1" || id=="V2" || id=="V3" || id=="V4" || id=="V5" ||
		               id=="V6" || id=="V7"){

	            //2004.11.30 
	            //Bxrge Moe
	            //Many stations sends 00 when not used. I suppose 00 is not 
	            //a valid value for V1 to V7.
	            if(spVal=="00")
	               continue;
	         }else if(id=="SA"){ //Esss, sss->SA
	            saSdEm_.sa = spVal;
	            continue;
	         }else if( id == "SD" ) {
	            if( allCh(spVal, '0') )
	               spVal = "-1";
	            
	            saSdEm_.sd = spVal;
	            continue;
	         }else if( id == "EM" ) {
	            saSdEm_.em = spVal;
	            continue;
	         }

	         it=paramList.find(Param(id, -1));
	  
	         if(it==paramList.end()){
	            ostringstream ost;
	            ost << "Unknown parameter: " << id << " (" << param 
		             << " -->";
	    
	            for(int ii=0; spDef[ii].id; ii++){
	               ost << " " << spDef[ii].id;
	            }
	            
               ost << ")";

	            throw UnknownParam(ost.str());
	         }
	  
	         data.push_back(DataElem(it->id(), spVal, sensor));
	      }	
	
	      return true;
      }
      
      void
      DataConvert::decodeVal(std::vector<DataElem> &data,
			                    const miutil::miTime  &obsTime,        
			                    const std::string     &param,
			                    const std::string     &val_,
			                    int                   sensor,
			                    int                   height,
			                    int                   mod){
         IParamList it;
	      std::string name;
	      miTime      ts;
	      string      val(val_);

	      name=convertName(param);
	
	      it=paramList.find(Param(name, -1));
	
	      if(it==paramList.end())
	         throw UnknownParam(std::string("Unknown Parameter: ")+param+" --> "
			                      +name);
	
            if(name=="RR_1" || name=="RR_2" || name=="RR_3" ||
               name=="RR_6" || name=="RR_9" || name=="RR_12" ||
               name=="RR_15" ||name=="RR_18" || name=="RR_24" ||
               name=="RR_X"){
               RRRtr_.RR_N=true;
            }else if(name=="KLFG" || name=="KLFX"){
	            ostringstream ost;
	  
	            int minDiff;
	            //May throw BadFormat. We leave it to the caller to
	            //catch the exception.
	            ts=convertToMiTimeFromHHMM(obsTime,val);
	  
	            if(!ts.undef()){
	               char myBuf[8];
	               sprintf(myBuf, "%02d%02d", ts.hour(), ts.min());
	               val=myBuf;
	            }
	         }else if(name=="PP"){
	            if(val.size()>0){
	               if(val[0]=='-' || val[0]=='+'){
	                  val.erase(0, 1);
	               }
	            }
	         }else if(name=="SA" || name=="SD" || name=="EM"){
	            if( val.size() <= 0 )
	               return;
	            
	            if( name=="SA" ) {
	               saSdEm_.hasSa = true;
	               saSdEm_.sa = val;
	            }else if( name == "SD") {
	               saSdEm_.hasSd = true;
	               saSdEm_.sd = val;
	            } else {
	               saSdEm_.hasEm = true;
	               saSdEm_.em = val;
	            }
	            
	            //We take care of the values when 
	            //SaSdEm is computed and inserted into the database.
	            //This hapends in the mainloop in autoobsdecoder.cc
	            return;
	         }

	         data.push_back(DataElem(it->id(), val, sensor, height, mod));
         }

	
         miutil::miTime
         DataConvert::convertToMiTimeFromHHMM(const miutil::miTime &obst_,
			                                     const std::string &val_){
	         ostringstream ost;
	         string        val(val_);
         	char          buf[100];
         	miClock       ti;
         	miTime        obst(obst_);

         	if(val.empty())
               return miTime(); //Undef
	
	         if(val.length()!=4 ){
	            ost << "Invalid timeformat, length <" << val.length() 
	                << ">. Expecting length: 4. The value is '" << val << "'!"; 
	            throw BadFormat(ost.str());
	         }
	
	         //Create a time with the format hh:mm:ss, from hhmm.
	         val.insert(2, ":");
	         val.append(":00");
	         ti.setClock(val);

	         if(ti.undef())
	            throw BadFormat("Invalid time format. Expecting 'hh:mm:ss'!");
      
	            return miTime(obst.date(), ti);
            }

            float 
            DataConvert::
            RRRtr::RR(int &paramid, const miutil::miTime &obstime){
               float rr;
	            int   RRR_;

	            paramid=-1;
	
	            if(RRR==INT_MAX && tr<0){
	               if(ir==3){
	                  if(obstime.hour()==6 || obstime.hour()==18){
	                     paramid=109; //RR_12
	                     tr=2;
	                     return -1.0;
	                  }else if(obstime.hour()==0 || obstime.hour()==12){
	                     paramid=108;
	                     tr=1;
	                     return -1.0;
	                  }
	               }
	               return  FLT_MAX;
	            }
	
	            if(RRR>=0){
	               RRR_=RRR;
	  
	               if(RRR_>0){
	                  if(rt>=5)
	                     RRR_-=1;
	               }
	
	               rr=static_cast<float>(RRR_)+static_cast<float>(rt)/10;
	            }else{
	               rr=-1.0;
	            }
	
	            switch(tr){
	               case 1: paramid=108; //RR_6
	                  break;
	               case 2: paramid=109; //RR_12
	                  break;
	               case 3: paramid=126; //RR_18
	                  break;
	               case 4: paramid=110; //RR_24
	                  break;
	               case 5: paramid=106; //RR_1
	                  break;
	               case 6: paramid=119; //RR_2 
	                  break;
               	case 7: paramid=107; //RR_3
	                  break;
	               case 8: paramid=120; //RR_9
	                  break;
	               case 9: paramid=125; //RR_15.
	                  break;
	               case 0: paramid=117; //RR_X
                     break;
	               default:
	                  return FLT_MAX;
	            }
	
	            return rr;
            }

            bool
            DataConvert::
            hasSaSdEm( SaSdEm &saSdEm )
            {
               string sa;
               
               LOGDEBUG("hasSaSdEm: hasSa=" << (hasSa?"t":"f") << " hasSd=" << (hasSd?"t":"f") << " hasEm=" << (hasEm?"t":"f") << endl
                        << " saSdEm_.hasEm=" << (saSdEm_.hasEm?"t":"f") << " saSdEm_.hasSa=" << (saSdEm_.hasSa?"t":"f") 
                        << " saSdEm_.hasSd=" << (saSdEm_.hasSd?"t":"f") << endl
                        << "saSdEm_.sa=" << saSdEm_.sa << " saSdEm_.em=" << saSdEm_.em 
                        << " saSdEm_.sd=" << saSdEm_.sd);
            
               if( ! hasSa && ! hasSd && ! hasEm && 
                   ! saSdEm_.hasSa  && ! saSdEm_.hasSd && ! saSdEm_.hasEm )
                  return false;
               
               
               if( hasEm || saSdEm_.hasEm ) {
                  if( saSdEm_.em.empty() ) {
                     if( saSdEm_.sa == "998" )
                        saSdEm_.em = "1";
                     else if ( hasEm && saSdEm_.hasEm)
                        saSdEm_.em = "-1";
                  }
               }

               
               if( hasSa || saSdEm_.hasSa ) {
                  if( ( saSdEm_.sa.empty() && hasSa && saSdEm_.hasSa ) || 
                      saSdEm_.sa == "998" || 
                      saSdEm_.sa == "000" )
                     sa = "-1";
                  else if( saSdEm_.sa == "997" )
                     sa = "0";
                  else if( saSdEm_.sa == "999" )
                     sa = "-3";
                  else
                     sa = saSdEm_.sa;
               }
                              
               saSdEm_.sa = sa;
               saSdEm = saSdEm_;
               
               return true;
            }
      
            SplitDef*
            findSplitDef(const  std::string &name, int &totSize) {
	            const char *p=name.c_str();
	
	            for(int i=0; splitArray[i].name; i++){
	               if(strcmp(splitArray[i].name, p)==0){
	                  totSize=0;
	                  SplitDef *arr=splitArray[i].def;
	    
	                  for(int k=0; arr[k].id; k++){
	                     if(arr[k].index<0){
		                     totSize=-1;
		                     return splitArray[i].def;
	                     }else
		                     totSize+=splitArray[i].def[k].size;
	                  }
	    
	                  return splitArray[i].def;
	               }
	            }
	
	            return 0;
            }

            std::string 
            convertName(const std::string &nameToConvert){
	            for(int i=0; nameConvertDefArray[i].oldName; i++){
	               if(strcmp(nameToConvert.c_str(), nameConvertDefArray[i].oldName)==0){
	                  return std::string(nameConvertDefArray[i].newName);
	               }
	            }

	            return std::string(nameToConvert);
            }
         }
      }
}
