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
#include <limits.h>
#include <boost/lexical_cast.hpp>
#include <puTools/miTime.h>
#include <miutil/commastring.h>
#include <milog/milog.h>
#include <stdlib.h>
#include <kvalobs/kvDbGate.h>
#include <kvalobs/kvQueries.h>
#include <kvalobs/kvTypes.h>
#include <miutil/trimstr.h>
#include "KvDataContainer.h"
#include <miutil/timeconvert.h>
#include "kldecoder.h"
#include <decodeutility/decodeutility.h>
#include <kvalobs/kvPath.h>



namespace pt=boost::posix_time;
using namespace kvalobs::decoder::kldecoder;
using namespace std;
using namespace dnmi::db;
using namespace miutil;
using namespace boost;
using namespace kvalobs;

namespace {
bool
decodeKeyVal( const string &keyval, string &key, string &val ){
    string::size_type i = keyval.find_first_of("=");

    if (i == string::npos) {
    	key = keyval;
    	val.erase();
    } else {
    	key = keyval.substr(0, i);
      	val = keyval.substr(i + 1);
    }

    trimstr( key );
    trimstr( val );

    return ! key.empty();
}

}

kvalobs::decoder::kldecoder::
KlDecoder::
KlDecoder( dnmi::db::Connection   &con,
           const ParamList        &params,
           const std::list<kvalobs::kvTypes> &typeList,
           const std::string &obsType,
           const std::string &obs,
           int                    decoderId)
:DecoderBase(con, params, typeList, obsType, obs, decoderId),
 datadecoder( paramList, typeList ),
 typeID( INT_MAX ), stationID(INT_MAX), onlyInsertOrUpdate( false )

{

    decodeObsType();
}

kvalobs::decoder::kldecoder::
KlDecoder::
~KlDecoder()
{
}

void
kvalobs::decoder::kldecoder::
KlDecoder::
decodeObsType()
{
    string keyval;
    string key;
    string val;
    string::size_type i;
    string::size_type iKey;
    CommaString cstr(obsType, '/');
    long  id;
    const char *keys[] = {"nationalnr","stationid","wmonr","icaoid","call_sign",
                    "type", "add", "received_time", 0};

    LOGDEBUG("decodeObsType: '" << obsType << "'");
    typeID = INT_MAX;
    stationID = INT_MAX;
    onlyInsertOrUpdate = false;

    if(cstr.size()<2){
    	LOGERROR("decodeObsType: To few keys!");
//        msg="obsType: Invalid Format!";
        return;
    }

    for( int index = 1; index < cstr.size(); ++index ) {
        if(!cstr.get( index, keyval)){
        	LOGERROR("decodeObsType: INTERNALERROR: InvalidFormat!");
            return;
        }

        if( ! decodeKeyVal( keyval, key, val ) ) //keyval empty
        	continue;

        for( iKey=0; keys[iKey]; ++iKey ) {
        	if(  key == keys[iKey] )
        		break;
        }

         if( ! keys[iKey] ) {
        	 LOGWARN("decodeObsType: unknown key '" <<  key << "'");
             continue;
         }


         if ( val.empty() && key != "add" ) //Must have a value
             continue;

         if( key == "add" ) { //Value is optional
        	 if( val.empty() || val[0]=='t' || val[0]=='T')
        		 onlyInsertOrUpdate = true;
         } else if( key == "received_time" ) {
        	 receivedTime = pt::time_from_string_nothrow( val );
         }else if( key ==  "type"  ) {
             typeID = atoi(val.c_str());
         } else {
             stationID = DecoderBase::getStationId(key, val);

             if(stationID < 0) {
                 stationID = INT_MAX;
                 LOGERROR("Now station with stationid '" << key << "=" << val << "'" );
                 continue;
             }
         }
     }

    LOGDEBUG("decodeObsType: stationID: " << stationID <<  " typeid: " << typeID
    		<< " update: " << (onlyInsertOrUpdate?"true":"false") );
}

std::string
kvalobs::decoder::kldecoder::
KlDecoder::
name() const
{
   return string("KlDataDecoder");
}

kvalobs::decoder::DecoderBase::DecodeResult
kvalobs::decoder::kldecoder::
KlDecoder::
rejected( const std::string &msg, const std::string &logid, std::string &msgToSender  )
{
   ostringstream ost;
   ostringstream myObs;
   bool saved=true;

   boost::posix_time::ptime tbtime( boost::posix_time::microsec_clock::universal_time());

   ost << "REJECTED: Decoder: " << name() << endl
         <<"message: " << msg  << endl
         <<"obsType: " << obsType << endl
         <<"obs: [" << obs << "]";

   myObs << obsType << "\n" << obs;

   msgToSender += myObs.str();

   kvalobs::kvRejectdecode rejected( myObs.str(),
                                     tbtime,
                                     name(),
                                     msg );

   if( !putRejectdecodeInDb( rejected ) ) {
      saved = false;
      ost << endl << "Can't save rejected observation!";
   } else {
      ost << endl << "Saved to rejectdecode!";
   }

   if( ! logid.empty() ) {
      IDLOGERROR( logid, "Rejected: " << msg << endl << obs.c_str()
                  << (saved?"":"\nFailed to save to 'rejected'."));
   }

   LOGERROR( ost.str() );
   return Rejected;
}


kvalobs::decoder::DecoderBase::DecodeResult
kvalobs::decoder::kldecoder::
KlDecoder::
insertDataInDb( kvalobs::serialize::KvalobsData *theData,
	         int stationid, int typeId,
	         const std::string &logid,
	         std::string &msgToSender )
{
	using namespace boost::posix_time;
	KvDataContainer::DataByObstime data;
	KvDataContainer::TextDataByObstime textData;
	KvDataContainer::TextDataByObstime::iterator tid;
	KvDataContainer::TextDataList td;
	map<ptime,int> observations;


	KvDataContainer container( theData );
	int priority=4;

	if( receivedTime.is_special() )
		priority = 10;

	if( container.get( data, textData, stationid, typeId, pt::second_clock::universal_time()) < 0 ){
		IDLOGINFO( logid, "No Data.");
		return Ok;
	}

	for( KvDataContainer::DataByObstime::iterator it = data.begin();
		 it != data.end(); ++it  ) {

		td.clear();
		tid = textData.find( it->first );

		if( tid != textData.end() ) {
			td = tid->second;
			textData.erase( tid );
		}

		/*
		bool addDataToDb( const miutil::miTime &obstime, int stationid, int typeid_,
		                        std::list<kvalobs::kvData> &sd,
		                        std::list<kvalobs::kvTextData> &textData,
		                        int priority, const std::string &logid,
		                        bool onlyAddOrUpdateData );
*/

		if( ! addDataToDb( to_miTime( it->first ), stationid, typeId, it->second, td,
				           priority, logid, getOnlyInsertOrUpdate() ) ) {
		    ostringstream ost;

		    ost << "DBERROR: stationid: " << stationid << " typeid: " << typeId << " obstime: " << it->first;
			LOGERROR( ost.str() );
			IDLOGERROR( logid, ost.str() );
			msgToSender += "\n" + ost.str();
			return NotSaved;
		}

		observations[it->first] += it->second.size();
	}

	//Is there any left over text data.
	if( ! textData.empty() ) {
		KvDataContainer::DataList dl;
		for( KvDataContainer::TextDataByObstime::iterator it = textData.begin();
			 it != textData.end(); ++it  ) {
			if( ! addDataToDb( to_miTime( it->first ), stationid, typeId, dl, it->second,
					           priority, logid, getOnlyInsertOrUpdate() ) ) {
			    ostringstream ost;
			    ost << "DBERROR: TextData: stationid: " << stationid << " typeid: " << typeId << " obstime: " << it->first;
				LOGERROR( ost.str() );
				IDLOGERROR( logid, ost.str() );
				msgToSender += "\n" + ost.str();
				return NotSaved;
			}
			observations[it->first] += it->second.size();
		}
	}

	ostringstream ost;
	int totalObservations=0;
	if( observations.size() > 0 ) {
	    for( map<ptime,int>::const_iterator it = observations.begin();
	         it != observations.end(); ++it ) {
	        ost << "  # observations: " << to_kvalobs_string( it->first ) <<": " << it->second << endl;
	        totalObservations += it->second;
	    }
	}

	ostringstream msgOst;
	IDLOGINFO(logid, "Observations saved to DB: " << totalObservations << " stationid: " << stationid << " typeid: " << typeId << endl << ost.str() );
	LOGINFO(         "Observations saved to DB: " << totalObservations << " stationid: " << stationid << " typeid: " << typeId  );

	msgOst << "Observations saved to DB: " << totalObservations << " stationid: " << stationid << " typeid: " << typeId << endl << ost.str();
	msgToSender += msgOst.str();
	return Ok;
}


kvalobs::decoder::DecoderBase::DecodeResult 
kvalobs::decoder::kldecoder::
KlDecoder::
execute(std::string &msg)
{
   bool setUsinfo7 = getSetUsinfo7();
   int typeId=getTypeId(msg);
   int stationid=getStationId(msg);

   logid.clear();

   if( receivedTime.is_special() && setUsinfo7 )
	   receivedTime = pt::second_clock::universal_time();

   ostringstream o;
   o << "Decoder: " << name() << ". New observation. stationid: " <<
            stationid << " typeid: " << typeId;

   if( ! receivedTime.is_special() )
	   o << " Obs. received: " << receivedTime;

   LOGINFO( o.str() );

   if( stationid == INT_MAX ) {
      o.str("");

      o << "Missing or unknown stationid! typeid: ";

      if( typeId > 0 )
         o << typeId;
      else
         o << "<NA>";

      return rejected( o.str(), "", msg );
   }

   if( typeId<0 || typeId == INT_MAX) {
      o.str("");
      o << "Format error in type!"
            << "stationid: " << stationid << ".";

      return rejected( o.str(), "", msg );
   }

   IdlogHelper idLog( stationid, typeId, this );
   logid = idLog.logid();

   trimstr( obs );
   obs += "\n";

   o.str("");
   o << name() << endl
     <<	"------------------------------" << endl
     << "ReceivedTime: ";
   if( receivedTime.is_special() )
	   o << " NOT given or set_useinfo7 is not set.";
   else
	   o << receivedTime;

   o << endl << "Insert type : " << (getOnlyInsertOrUpdate()?"update (replenish)":"insert");

   o << endl
     << "ObstType    : " << obsType         << endl
     << "Obs         : " << obs             << endl;

   IDLOGINFO( logid, o.str() );

   serialize::KvalobsData *kvData;
   kvData = datadecoder.decodeData( obs, stationid,  typeId, receivedTime, logid, name() );

   if( ! kvData )
	   return rejected( datadecoder.messages, logid, msg );

   if( datadecoder.warnings ) {
       IDLOGWARN( logid, datadecoder.messages );
   }
   return insertDataInDb( kvData, stationid, typeId, logid, msg );
}


long 
kvalobs::decoder::kldecoder::
KlDecoder::
getStationId(std::string &msg)const
{
    return stationID;
}

bool
kvalobs::decoder::kldecoder::
KlDecoder::
getOnlyInsertOrUpdate()const
{
	return onlyInsertOrUpdate;
}


bool
kvalobs::decoder::kldecoder::
KlDecoder::
getSetUsinfo7()
{
	bool setUsinfo7 = false;
	miutil::conf::ConfSection *conf = myConfSection();
	miutil::conf::ValElementList val=conf->getValue("set_useinfo7");

	if( val.size() > 0 ) {
		string v = val[0].valAsString();
		if( v.size() > 0 && (v[0]=='t' || v[0]=='T') )
			setUsinfo7 = true;
	}

	return setUsinfo7;
}

long 
kvalobs::decoder::kldecoder::
KlDecoder::
getTypeId(std::string &msg)const
{
    return typeID;
}

#if 0
kvalobs::decoder::DecoderBase::DecodeResult
kvalobs::decoder::kldecoder::
KlDecoder::
execute(std::string &msg)
{
   list<kvalobs::kvData> dataList;
   list<kvalobs::kvTextData> textDataList;
   pt::ptime             nowTime( pt::second_clock::universal_time() );
   string                tmp;
   pt::ptime             obstime;
   pt::ptime             tbtime( pt::second_clock::universal_time() );
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

   warnings=false;
   logid.clear();

   if( receivedTime.is_special() && setUsinfo7 )
	   receivedTime = pt::second_clock::universal_time();

   LOGINFO( "Decoder: " << name() << ". New observation. stationid: " <<
            stationid << " typeid: " << typeId);

   if( stationid == INT_MAX ) {
      ostringstream o;

      o << "Missing stationid! typeid: ";

      if( typeId > 0 )
         o << typeId;
      else
         o << "<NA>";

      return rejected( o.str(), "" );
   }

   if( typeId<=0 || typeId == INT_MAX) {
      ostringstream o;
      o << "Format error in type!"
            << "stationid: " << stationid << ".";

      return rejected( o.str(), "");
   }

   IdlogHelper idLog( stationid, typeId, this );
   logid = idLog.logid();

   trimstr( obs );
   obs += "\n";

   istringstream istr(obs);

   IDLOGINFO( logid,
              name()                           << endl <<
              "------------------------------" << endl <<
              "ObstType : " << obsType         << endl <<
              "Obs      : " << obs             << endl );

   msg = "OK!";

   if( !getline( istr, tmp ) ) {
      ostringstream o;
      o << "Invalid format. No data. stationid: " << stationid << " typeid: " << typeId;

      return rejected( o.str(), logid );
   }else{
      if( !decodeHeader( tmp, params, msg ) ) {
         ostringstream o;
         o << "INVALID header. stationid: " << stationid << " typeid: " << typeId ;
         return rejected( o.str(), logid );
      }
   }

   if( params.size() < 1 ) {
      msg = "No parameters in header!";
      LOGINFO( "Decoder: " << name() << ". No parameters in header! Stationid: "
               << stationid << " typeid: " << typeId );
      IDLOGINFO( logid, "No parameters in header!" << endl << "Header: " << tmp );
      return Ok;
   }

   string::size_type i;

   while( getline( istr, tmp ) ) {
      lines++;
      i = tmp.find_first_of( "," );
      obstime = pt::time_from_string_nothrow( tmp.substr( 0, i ) );

      if( obstime.is_special() ) {
         ostringstream err;
         err << "Invalid obstime. Line: " << tmp << endl
               << "stationid: " << stationid << " typeid: " << typeId;

         return rejected( err.str(), logid );
      }

      tmp.erase(0, ( i == string::npos?i:i+1 ) );

      KlDataArray da;

      if( !decodeData( da, params.size(), obstime, tmp, lines, msg ) ) {
         ostringstream o;
         o << "Cant decode data. Line: " << tmp << endl
               << "Reason: " << msg <<  endl
               << "Stationid: " <<  stationid << " typeid: " << typeId;
         return rejected( o.str(), logid );
      }

      ostringstream ost;
      ost << "[" << tmp << "]" << endl;

      for(KlDataArray::size_type index=0; index<da.size(); index++)
         ost << params[index].name() << "("<< params[index].id() << ")["
         << params[index].sensor() << "," << params[index].level() << "]=("
         << da[index].val()<<"," << da[index].cinfo()<<"," << da[index].uinfo()
         << endl;

      IDLOGDEBUG3( logid, ost.str() );
      dataList.clear();
      textDataList.clear();
      nElemsInLine=0;

      for( KlDataArray::size_type index=0; index < da.size(); index++ ) {
         KlData data = da[index];

         if( data.empty() )
            continue;

         nExpectedData++;
         nElemsInLine++;

         string val=data.val();

         if( isTextParam( params[index].id() ) ) {
            kvTextData d( stationid,
                          obstime,
                          val,
                          params[index].id(),
                          tbtime,
                          typeId );

            textDataList.push_back( d );
         }else{
            if( params[index].code() ) {
               if( params[index].name() == "VV" ) {
                  val = decodeutility::VV(val);
               }else if( params[index].name() == "HL" ) {
                  val = decodeutility::HL( val );
               }else{
                  warnings=true;
                  IDLOGWARN( logid, "Unsupported as code value: " << params[index].name() );
                  continue;
               }
            }

            try{
               fval = lexical_cast<float>( val );
            }
            catch(...){
               warnings = true;
               IDLOGERROR( logid, "Invalid value: (" << val << ") not a float!" );
               continue;
            }

            kvData d( stationid,
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
                      "" );

            dataList.push_back( d );
         }
      }


      if( addDataToDb( pt::to_miTime( obstime ), stationid, typeId, dataList, textDataList, priority, idLog.logid(), onlyInsertOrUpdate) ) {
         count += dataList.size() + textDataList.size();
      }

      if(nElemsInLine>0)
         nLineWithData++;
   }

   ostringstream ost;
   ost << "# Lines:             " << lines-1 << endl
         << "# Lines with data:   " << nLineWithData << endl
         << "# dataelements:      " << nExpectedData << endl
         << "# Saved datarecords: " << count <<  endl
         << "# Error in save:     " << nErrors;

   msg = ost.str();

   IDLOGINFO( logid, msg );

   if( lines==1 || ( count == 0 && nExpectedData == 0 ) ){
      msg += "No data!";
      return Ok;
   }

   if( count > 0 ){
      if( nExpectedData != count ) {
         ostringstream  ost;
         ost << "WARNING: Expected to save " << nExpectedData
               << " dataelements, but only "  << count
               << " dataelements was saved!";
         warnings = true;
         IDLOGWARN( logid, ost.str() );
         msg += ost.str();
      }
      if( warnings ) {
         LOGWARN("Data saved with warnings. stationid: " << stationid << " typeid: " << typeId );
      } else {
         LOGINFO("Data saved. stationid: " << stationid << " typeid: " << typeId );
      }

      return Ok;
   }

   return Rejected;
}
#endif
