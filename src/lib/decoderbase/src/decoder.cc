/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: decoder.cc,v 1.41.2.5 2007/09/27 09:02:27 paule Exp $                                                       

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
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <sstream>
#include <cstring>
#include <kvalobs/kvDbGate.h>
#include <kvalobs/kvQueries.h>
#include <kvalobs/kvGeneratedTypes.h>
#include <kvalobs/kvWorkelement.h>
#include <miutil/commastring.h>
#include <miutil/trimstr.h>
#include "../include/decoderbase/decoder.h"
#include "../include/decoderbase/ConfParser.h"
#include "../include/decoderbase/metadata.h"

using namespace std;
using namespace kvalobs;

namespace{
  struct TextParam{
    char *name;
    int  paramid;
  };
  
  TextParam textParams[]={
    {"signature", 1000},
    {"TEXT",      1001},
    {"KLSTART",   1021},
    {"KLOBS",     1022},
    {"WWB1",      1039},
    {"WWB2",      1040},
    {"WWB3",      1041},
    {"WWCAVOK",   1042},
    {"KLFG",      1025},
    {"KLFX",      1026},
    {0, 0}
  };
} 




    
kvalobs::decoder::
DecoderBase::
DecoderBase(
	    dnmi::db::Connection   &con_,
	    const ParamList        &params,
	    const std::list<kvalobs::kvTypes> &typeList_,
	    const miutil::miString &obsType_,
	    const miutil::miString &obs_, 
	    int                    decoderId_)
  :decoderId(decoderId_), con(con_), paramList(params), typeList(typeList_),
   obsType(obsType_),obs(obs_)
{
}

     
     
kvalobs::decoder::
DecoderBase::
~DecoderBase()
{
  list<string>::iterator it=createdLoggers.begin();
  
  for(;it!=createdLoggers.end(); it++){
      milog::Logger::removeLogger(*it);
  }
}


void 
kvalobs::decoder::
DecoderBase::
setMetaconf(const miutil::miString &metaconf_)
{
  using namespace miutil::parsers::meta;

  MetaParser parser;
  
  Meta *meta=parser.parse(metaconf_);

  LOGDEBUG6("Metaconf: " << endl << metaconf_);

  if(meta){
    if(!parser.getWarnings().empty()){
      LOGWARN("Warnings from parsing metaconf!" << endl << metaconf_ << endl
	      << "Reason: " << parser.getWarnings());
    }
    
    metaconf=*meta;
    delete meta;
  }else{
    LOGERROR("Error parsing metaconf!" << endl << metaconf_ << endl << 
	     "Reason: " << parser.getErrMsg());
  }
  


}

std::string 
kvalobs::decoder::
DecoderBase::
getObsTypeKey( const std::string &keyToFind_ ) const
{
   string keyval;
   string key;
   string val;
   string keyToFind( keyToFind_  );
   
   miutil::trimstr( keyToFind );
   
   if( keyToFind.empty() )
      return "";
   
   miutil::CommaString cstr(obsType, '/');
    
   for( int i=0; i<cstr.size(); ++i ) {
      
      if( ! cstr.get( i, keyval ) )
         continue;
      
      miutil::CommaString tmpKeyVal( keyval, '=' );
      
      if( tmpKeyVal.size() <2 )
         continue;
      
      tmpKeyVal.get( 0, key );
      tmpKeyVal.get( 1, val );
      
      miutil::trimstr( key );
      
      if( key == keyToFind ) {
         
         if( key.size() >= 2)
            return val;
      }
   }
   
   return "";

}


std::string 
kvalobs::decoder::
DecoderBase::
getMetaSaSd()const
{
   string meta=getObsTypeKey( "meta_SaSd" );

   if(meta.length() < 2)
      return "";
     
   return meta;
}
      
const kvalobs::kvTypes* 
kvalobs::decoder::
DecoderBase::
findType(int typeid_)const
{
  std::list<kvalobs::kvTypes>::const_iterator it=typeList.begin();

  for(; it!=typeList.end(); it++){
    if(it->typeID()==typeid_)
      return &(*it);
  }
  
  return 0;
}

void 
kvalobs::decoder::
DecoderBase::
addStationInfo(long stationid,
	       const miutil::miTime &obstime,
	       long typeid_,
	       const miutil::miTime &tbtime,
	       int priority)
{
  IkvStationInfoList it=stationInfoList.begin();

  for(;it!=stationInfoList.end(); it++){
    if(it->stationID()==stationid &&
       it->obstime()==obstime     &&
       it->typeID()==typeid_){
    
      return;
    }
  }
  
    
  kvDbGate gate(&con);
  miutil::miTime undefTime;

  if(!gate.insert(kvWorkelement(stationid, 
				obstime, 
				typeid_, 
				tbtime, 
				priority,
				undefTime,
				undefTime, 
				undefTime, 
				undefTime, 
				undefTime), 
		  true)){
    LOGERROR("addStationInfo: can't save kvWorkelement into the" << endl <<
	     "the table 'workque' in  the database!\n" <<
	     "[" << gate.getErrorStr() << "]");
    return;
  }
  
  
  LOGDEBUG("addStation: station added to the table 'workque'." 
	   << endl << 
	   "-- Stationid: " << stationid << endl <<
	   "-- obstime:   " << obstime << endl <<
	   "-- typeid:    " << typeid_ << endl << 
	   "-- priority:  " << priority );
  
  stationInfoList.push_back(kvStationInfo(stationid, obstime, typeid_));
}


bool 
kvalobs::decoder::
DecoderBase::
isGenerated(long stationid, long typeid_)
{
  kvDbGate gate(&con);

  for(list<GenCachElem>::iterator it=genCachElem.begin();
      it!=genCachElem.end();
      it++){
    if(it->stationID()==stationid && it->typeID()==typeid_){
      return it->generated();
    }
  }

  list<kvGeneratedTypes> stList;

  if(!gate.select(stList, kvQueries::selectIsGenerated(stationid, typeid_))){
    LOGDEBUG("isGenerated: DBERROR: " << gate.getErrorStr());
    throw dnmi::db::SQLException(gate.getErrorStr());
  }

  genCachElem.push_back(GenCachElem(stationid, typeid_, !stList.empty()));

  return !stList.empty();
}

int 
kvalobs::decoder::
DecoderBase::
getDecoderId()const
{ 
  return decoderId;
}


long                 
kvalobs::decoder::
DecoderBase::
getStationId(const miutil::miString &key, 
	     const miutil::miString &value)
{
  using namespace dnmi::db;
  using namespace std;
  
  std::string query("SELECT stationid FROM station WHERE ");
  query+=key+"="+value;


  LOGDEBUG("DecoderBase::getStationId: query(" << query << ")\n");
  
  Result *res;
  long   stationId=-1;

  try{
    res=con.execQuery(query);
    
    if(res){
      /* LOGDEBUG("Size: " << res->size() << endl <<
	       res->fieldName(0) <<  << endl <<
	       "=======================\n");*/
      
      if(res->hasNext()){
	DRow row=res->next();
	//LOGDEBUG(row[0] << endl);
	stationId=atol(row[0].c_str());
      }
      delete res;
    }
    
    return stationId;
  }
  catch(SQLException &ex){
    LOGERROR("Exception: " << ex.what() << endl);
    return -2;
  }

} 

bool  
kvalobs::decoder::
DecoderBase::
deleteKvDataFromDb(const kvalobs::kvData &sd)
{
   kvDbGate gate(&con);
   std::list<kvalobs::kvData> dataList;
   miutil::miString query(kvQueries::selectData(sd));
   std::ostringstream ost;

   ost << "delete from " << sd.tableName() << " " << query;
       
   try{
     LOGDEBUG("deleteKvDataFromDb: (delete): " << ost.str() << "\n"); 
     con.exec(ost.str());
   }
   catch(...){
     return false;
   }

   return true;
}


bool  
kvalobs::decoder::
DecoderBase::
putKvDataInDb(const kvalobs::kvData &sd_,
	      int priority)
{
  kvalobs::kvData sd(sd_);
  kvDbGate gate(&con);
  ostringstream ost;
  
  try{
    if(isGenerated(sd.stationID(), sd.typeID())){
      LOGDEBUG("GENERATEDDATA: stationid: " << sd.stationID() << " typeid: " 
	       << sd.typeID() << " obstime: " << sd.obstime() );
      
      sd.typeID(-1*sd.typeID());
    }
  }
  catch(...){
    //COMMENT:
    //do nothing. The log message from isGenerated is enough for 
    //the momment.
  }


  if(!gate.insert(sd, true)){
    LOGDEBUG("putKvDataInDb: can't save kvData to the database!\n" <<
	     "[" << gate.getErrorStr() << "]");

    return false;
  }

  
  addStationInfo(sd.stationID(), 
		 sd.obstime(), 
		 sd.typeID(), 
		 sd.tbtime(),
		 priority);
  
  return true;
}


bool 
kvalobs::decoder::
DecoderBase::
putKvDataInDb(const std::list<kvalobs::kvData> &sd_, 
	      int priority)
{
  std::list<kvalobs::kvData> sd(sd_);
  std::list<kvalobs::kvData>::iterator it=sd.begin();
  long sid=-1;
  long tid;
  long myTid;
  miutil::miTime obsTime;
  miutil::miTime tbTime;
  kvDbGate gate(&con);
  ostringstream ost;

  if(it==sd.end())
    return true;
  
  sid=it->stationID();
  tid=it->typeID();
  obsTime=it->obstime();
  tbTime=it->tbtime();
  myTid=tid;

  try{
    if(myTid>0 && isGenerated(sid, tid)){
      LOGDEBUG("GENERATEDDATA: stationid: " << it->stationID() 
	       << " typeid: " 
	       << it->typeID() << " obstime: " << it->obstime() );
      
      myTid=-1*tid;
    }
  }
  catch(...){
    return false;
  }

  
  while(it!=sd.end()){
    if(sid!=it->stationID() ||
       tid!=it->typeID() || 
       obsTime!=it->obstime()){
      
      addStationInfo(sid,obsTime, myTid, tbTime, priority);
    
      sid=it->stationID();
      tid=it->typeID();
      obsTime=it->obstime();
      tbTime=it->tbtime();
      myTid=tid;
      
      try{
	if(myTid>0 && isGenerated(sid, tid)){
	  LOGDEBUG("GENERATEDDATA: stationid: " << it->stationID() 
		   << " typeid: " 
		   << it->typeID() << " obstime: " << it->obstime() );
	  
	  myTid=-1*tid;
	}
      }
      catch(...){
	return false;
      }
    } 
    
    it->typeID(myTid);

    if(!gate.insert(*it, true)){
      LOGDEBUG6("putKvDataInDb: can't save kvData to the database!\n" <<
	       "[" << gate.getErrorStr() << "]");
      
      return false;
    }
    
    it++;
  }
  
  addStationInfo(sid,obsTime, myTid, tbTime, priority);
  
  return true;
}
  
bool 
kvalobs::decoder::
DecoderBase::
putkvTextDataInDb(const kvalobs::kvTextData &td_, int priority)
{
  kvalobs::kvTextData td(td_);
  kvDbGate gate(&con);

  try{
    if(isGenerated(td.stationID(), td.typeID())){
      LOGDEBUG("GENERATEDDATA: stationid: " << td.stationID() << " typeid: " 
	       << td.typeID() << " obstime: " << td.obstime() );
      
      td.typeID(-1*td.typeID());
    }
  }
  catch(...){
    //COMMENT:
    //do nothing. The log message from isGenerated is enough for 
    //the momment.
  }

  
  if(!gate.insert(td, true)){
    LOGDEBUG("putkvTextDataInDb: can't save kvTextData to the database!\n" <<
	     "[" << gate.getErrorStr() << "]");
 
    
    return false;
  }
  
  addStationInfo(td.stationID(), 
		 td.obstime(), 
		 td.typeID(), 
		 td.tbtime(),
		 priority);
  
  return true;
}


bool 
kvalobs::decoder::
DecoderBase::
putkvTextDataInDb(const std::list<kvalobs::kvTextData> &td_, int priority)
{
  std::list<kvalobs::kvTextData> td(td_);
  std::list<kvalobs::kvTextData>::iterator it=td.begin();
  long sid=-1;
  long tid;
  long myTid;
  miutil::miTime obsTime;
  miutil::miTime tbTime;

  if(it==td.end())
    return true;

  kvDbGate gate(&con);

  for(;it!=td.end(); it++){
    if(sid==-1 || 
       sid!=it->stationID() ||
       tid!=it->typeID() || 
       obsTime!=it->obstime()){
      
      if(sid!=-1){
	addStationInfo(sid, obsTime, tid, tbTime, priority);
      }

      sid=it->stationID();
      tid=it->typeID();
      obsTime=it->obstime();
      tbTime=it->tbtime();
      myTid=tid;
      
      try{
	if(myTid>0 && isGenerated(sid, tid)){
	  LOGDEBUG("GENERATEDDATA: stationid: " << it->stationID() << 
		   " typeid: " << it->typeID() << " obstime: " <<
		   it->obstime());
	  
	  myTid=-1*tid;
	}
      }
      catch(...){
	//COMMENT:
	//do nothing. The log message from isGenerated is enough for 
	//the momment.
      }
    }
    
    it->typeID(myTid);
    
    if(!gate.insert(*it, true)){
      LOGDEBUG("putkvTextDataInDb: can't save kvTextData to the database!\n" 
	       << "[" << gate.getErrorStr() << "]");
      
      return false;
    }
  } 

  addStationInfo(sid,obsTime,tid,tbTime, priority);
  
  return true;
}



bool  
kvalobs::decoder::
DecoderBase::
putRejectdecodeInDb(const kvalobs::kvRejectdecode &sd)
{

  kvDbGate gate(&con);
  
  if(!gate.insert(sd, true)){
    LOGDEBUG("putRejectdecodeInDb: can't save kvRejectdecode to the database!\n" <<
	     "[" << gate.getErrorStr() << "]");
    
    
    return false;
  }
  
  return true;
}


bool  
kvalobs::decoder::
DecoderBase::
putkvStationInDb(const kvalobs::kvStation &st)
{
  kvDbGate gate(&con);
  
  if(!gate.insert(st, true)){
    LOGDEBUG("kvStationInDb: can't save kvStation to the database!\n" <<
	     "[" << gate.getErrorStr() << "]");
    
    return false;
  }
  
  return true;
}




bool 
kvalobs::decoder::
DecoderBase::
isTextParam(const std::string &paramname)
{
  for(int i=0; textParams[i].name; i++){
    if(textParams[i].name==paramname)
      return true;
  }
  
  return false;
}

bool 
kvalobs::decoder::
DecoderBase::
isTextParam(int paramid)
{
  for(int i=0; textParams[i].name; i++){
    if(textParams[i].paramid==paramid)
      return true;
  }

  return false;
}

bool 
kvalobs::decoder::
DecoderBase::
loadConf(int sid, int tid,
	 kvalobs::decoder::ConfParser &parser)
{
  string                    kvpath;
  miutil::conf::ConfParser  myparser;
  miutil::conf::ConfSection *conf;
  ostringstream              fname;
  ifstream    fis;

  if(getenv("KVALOBS")){
    kvpath=getenv("KVALOBS");
    if(!kvpath.empty() && kvpath[kvpath.length()-1]=='/')
      kvpath.erase(kvpath.length()-1);
  }


  fname << kvpath << "/etc/decode/" << name() << "/" << sid << "-" << tid
	<< ".conf";
  
  fis.open(fname.str().c_str());

  if(!fis){
    LOGERROR("Cant open the configuration file <" << fname << ">!" << endl);
  }else{
      LOGINFO("Reading configuration from file <" << fname << ">!" << endl);
      conf=myparser.parse(fis);
      
      if(!conf){
	LOGERROR("Error while reading configuration file: <" << fname 
		 << ">!" << endl << myparser.getError() << endl);
      }else{
	LOGINFO("Configuration file loaded!\n");
	parser.parse(*conf);
	
	return true;
      }
  }

  return false;
}



milog::FLogStream*
kvalobs::decoder::
DecoderBase::
openFLogStream(const std::string &filename)
{
  using namespace std;

  milog::FLogStream *fs;
  ostringstream     ost;
  list<string>      pathlist;
  bool              error=false;
  
  try{
    fs=new milog::FLogStream(2);
  }
  catch(...){
    LOGERROR("OUT OF MEMMORY, cant allocate FLogStream!");
    return 0;
  }
  
  if(getenv("KVALOBS")){
    string path=getenv("KVALOBS");
    if(!path.empty() && path[path.length()-1]=='/')
      path.erase(path.length()-1);

    if(path.empty())
      error=true;
    else
      pathlist.push_back(path);
  }else{
    error=true;
  }

  if(error){
    LOGERROR("Cant open logfile! "  << 
	     "MISSING/EMPTY environment variable KVALOBS " << endl <<
	     "Logging all activity to: /dev/null");
    fs->open("/dev/null");
    
    return fs;
  }

  pathlist.push_back("var");
  pathlist.push_back("log");
  pathlist.push_back("decoders");
  pathlist.push_back(name());
  
  error=false;

  ost.str("");
 
  for(list<string>::iterator it=pathlist.begin();
      it!=pathlist.end() && !error;
      it++){
    ost << "/" << *it;

    if(mkdir(ost.str().c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)<-1){
      if(errno==EEXIST){
	continue;
      }else{
	error=true;
      }
    }
  }
 
  if(error){
    LOGERROR("A directory in the logpath maybe missing or a dangling link!"
	     << "Path: " <<ost.str() << endl <<
	     "Logging all activity to: /dev/null");
     fs->open("/dev/null");
     return fs;
  }

  ost << "/" << filename;

  if(!fs->open(ost.str())){
    LOGERROR("Failed to create logfile: " << ost.str() << endl <<
	     "Using /dev/null!");
    fs->open("/dev/null");
  }else{
    LOGINFO("Logfile (open): " << ost.str());
  }

  return fs;
}



void
kvalobs::decoder::
DecoderBase::
createLogger(const std::string &logname)
{
  milog::FLogStream *ls=openFLogStream(logname+".log");

  if(!ls)
    return;

  createdLoggers.push_back(logname);
  
  milog::Logger::createLogger(logname, ls);
  milog::Logger::logger(logname).logLevel(milog::DEBUG);
}

void
kvalobs::decoder::
DecoderBase::
removeLogger(const std::string &logname)
{
  list<string>::iterator it=createdLoggers.begin();

  for(;it!=createdLoggers.end(); it++){
    if(*it==logname){
      milog::Logger::removeLogger(logname);
      createdLoggers.erase(it);
      return;
    }
  }
}

void 
kvalobs::decoder::
DecoderBase::
loglevel(const std::string &logname, milog::LogLevel loglevel)
{
  //We only changes the loglevel if we have created the logger.

  if(logname.empty())
    return;

  list<string>::iterator it=createdLoggers.begin();

  for(;it!=createdLoggers.end(); it++){
    if(*it==logname){
      milog::Logger::logger(logname).logLevel(loglevel); 
      return;
    }
  }
}
