/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: paramlist.h,v 1.1.2.2 2007/09/27 09:02:30 paule Exp $

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

#include <string>
#include <cctype>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/lexical_cast.hpp>
#include <miutil/trimstr.h>
#include <decodeutility/isTextParam.h>
#include <decodeutility/getUseInfo7.h>
#include <decodeutility/decodeutility.h>
#include <miutil/commastring.h>
#include <miutil/timeconvert.h>

#include "DataDecode.h"

using namespace std;
namespace pt=boost::posix_time;

namespace kvalobs{
namespace decoder{
namespace kldecoder{
namespace bits {

int
DataDecoder::
findParamId( const std::string &paramname )const
{
	ParamList::const_iterator it;

	it = params.find( Param( boost::to_upper_copy( paramname ), -1 ) );

	if(it==params.end())
		return -1;
	else
		return it->id();
}



bool
DataDecoder::
decodeData( KlDataArray &da,
		    KlDataArray::size_type daSize,
		    boost::posix_time::ptime &obstime,
		    const boost::posix_time::ptime &receivedTime,
		    const int typeId,
		    const std::string &sdata,
		    int line,
		    std::string &msg)
{
	string::size_type      i;
	string::size_type      iEnd;
	list<string>           dtmp;
	list<string>::iterator it;
	ostringstream          ost;
	string                 buf;

	if( !splitData( sdata, dtmp, msg ) ){
		IDLOGERROR( logid, "decodeData: " << msg << endl);
		return false;
	}

	if( dtmp.size() < 2 ) {
		msg = "Invalid dataline: no parameter data in the line. Line: '"+sdata+"'.";
		return false;
	}
	//Remember first element is the obstime
	if( daSize != (dtmp.size()-1) ){
		ost.str("");
		ost << "decodeData: expected # data elements: " << daSize << endl
				<< "Found in datastring #: " << dtmp.size() << " line: " << line;
		IDLOGERROR( logid, ost.str());
		msg=ost.str();
		return false;
	}
	it=dtmp.begin();

	obstime = pt::time_from_string_nothrow( *it );

	if( obstime.is_special() ) {
		ost.str("");
		ost << "Invalid obstime '" << *it << "'. Line: " << line;
		msg = ost.str();
		return false;
	}

	for( ++it; it != dtmp.end(); ++it )
		ost << " [" << *it<< "]";

	IDLOGDEBUG( logid,"decodeData: Data in string: " << endl <<
			"[" << sdata<< "]" << endl <<
			ost.str());


	da = KlDataArray(daSize);
	KlDataArray::size_type index = 0;

	for( it = dtmp.begin(), ++it; it!=dtmp.end(); ++it ){
		string val;
		kvControlInfo c;
		kvUseInfo     u;

		buf=*it;
		miutil::trimstr(buf);
		i=buf.find_first_of("(", 0);

		if(i==string::npos){
			val=buf;
		}else{
			val=buf.substr(0, i);
			miutil::trimstr(val);
			iEnd=buf.find_first_of(")", i);

			if(iEnd==string::npos){
				ost.str("");
				ost << "Invalid format: missing ')' in data ["+buf+"]"
						<< " at index: " << index << " line: " << line;
				msg=ost.str();
				IDLOGERROR(logid,"decodeData: " << ost.str());
				return false;
			}

			i++;
			buf=buf.substr(i, iEnd-i);

			miutil::CommaString cs(buf);

			if( cs.size() == 0 ) //empty ()
				continue;

			if( cs.size()>2 ){
				ost.str("");
				ost << "Invalid format: wrong number of values in" <<
						" optional part of data element: " << index << " line: " << line;
				msg=ost.str();
				IDLOGERROR( logid, "decodeData: " << ost.str());
				return false;
			}

			cs.get(0, buf);

			if(buf.length()==16){
				c=setControlInfo( buf );
			}else if(buf.length()>0){
				ost.str("");
				ost << "Expected 16 character in <controlinfo>: "
						<< "found " << buf.length() << " characters at index: "
						<< index << " line: " << line;
				msg=ost.str();
				warnings = true;
				IDLOGWARN(logid,"decodeData: " << ost.str());
			}

			if(cs.size()==2){
				cs.get(1, buf);

				if(buf.length()==16){
					u = setUseinfo( buf );
				}else if(buf.length()>0){
					ost.str("");
					ost << "Expected 16 character in <useinfo>: "
							<< "found " << buf.length() << " characters at index: "
							<< index << " line: " << line;
					msg=ost.str();
					warnings = true;
					IDLOGWARN( logid, "decodeData: " << ost.str());
				}
			}
		}

		if( ! receivedTime.is_special() ) {
			int f = decodeutility::getUseinfo7Code( typeId, receivedTime, obstime, types );
			//If the typeId is unknown the flag is set to 0.
			u.set( 7, (f<0?0:f) );
		}

		if(!val.empty())
			da[index]=KlData(val, c, u);

		index++;
	}

	return true;
}


bool
DataDecoder::
splitString( const std::string &header,
		     std::list<std::string> &params,
		     int maxNumberOfOtionalElements,
		     std::string &msg) const
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
		}else{	//header[iEnd]=='('
			if( maxNumberOfOtionalElements == 0 ) {
				msg = "Invalid format: No optional part allowed!";
				return false;
			}

			int n=0; //Count of commas (,).

			iEnd=header.find_first_of(",)", iEnd+1);
			while(header[iEnd]==','){
				iEnd=header.find_first_of(",)", iEnd+1);
				n++;
			}

			if(iEnd==string::npos){
				msg="Invalid format: missing ')'!";
				return false;
			}

			if( n > (maxNumberOfOtionalElements-1) ){
				ostringstream ost;
				ost << "Invalid format: Expecting max '" << maxNumberOfOtionalElements << "' elements in optional part!";
				msg= ost.str();
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

		miutil::trimstr( param );
		params.push_back(param);
	}

	return true;
}

bool
DataDecoder::
decodeHeader( const std::string &header,
		std::vector<ParamDef> &paramsList,
		std::string &message )
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

	paramsList.clear();

	if(!splitParams( header, paramStrings, message) )
		return false;

	list<string>::iterator itParamsStrings=paramStrings.begin();

	ost << "ParamStrings: " << endl;

	for(;itParamsStrings!=paramStrings.end(); itParamsStrings++)
		ost << " [" << *itParamsStrings << "]";

	IDLOGDEBUG(logid, ost.str());

	itParamsStrings=paramStrings.begin();

	for(;itParamsStrings!=paramStrings.end(); itParamsStrings++){
		param=*itParamsStrings;
		sensor=0;
		level=0;

		i=param.find_first_of("(", 0);

		if(i==string::npos){
			name=param;
		}else{
			name=param.substr(0, i);
			miutil::trimstr(name);
			iEnd=param.find_first_of(")", i);

			if(iEnd==string::npos){ //paranoia
				message="Invalid format: missing ')' in param [" +name +"]";
				return false;
			}

			i++;
			param=param.substr(i, iEnd-i);

			miutil::CommaString cs(param);

			if(cs.size() > 2 ){
				message="Invalid format: wrong number of parameteres in optional part of"+
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
				message="Invalid parameter format: paramname missing!";
				return false;
			}
		}else{
			isCode=false;
		}

		it=params.find(Param( name, -1));

		if(it==params.end()){
			paramsList.push_back(ParamDef(name, -1, sensor, level, isCode));
		}else{
			paramsList.push_back(ParamDef(name, it->id(), sensor, level, isCode));
		}
	}

	return true;
}

int
DataDecoder::
hexCharToInt( char c )const
{
	c = std::toupper( c );

	const int zv = int('0');
	const int nv = int('9');
	const int av = int('A');
	const int fv = int('F');

	int v = int(c);

	if (v >= zv && v <= nv)
		return v - zv;
	else if (v >= av && v <= fv)
		return v - av + 10;

	// illegal character
	return 0;
}

kvalobs::kvUseInfo
DataDecoder::
setUseinfo( const std::string &flags  )const
{
	if( flags.size() != 16 )
		return kvalobs::kvUseInfo();

	kvalobs::kvUseInfo u;

	for( int i = 0; i<flags.size(); ++i ) {
		if( flags[i] == 'x' || flags[i] == 'X' )
			continue;
		u.set( i, hexCharToInt( flags[i]) );
	}
	return u;
}

kvalobs::kvControlInfo
DataDecoder::
setControlInfo( const std::string &flags  )const
{
	if( flags.size() != 16 )
		return kvalobs::kvControlInfo();

	kvalobs::kvControlInfo cf;

	for( int i = 0; i<flags.size(); ++i ) {
		if( flags[i] == 'x' || flags[i] == 'X' )
			continue;
		cf.set( i, hexCharToInt( flags[i]) );
	}
	return cf;

}

kvalobs::serialize::KvalobsData*
DataDecoder::
decodeData( const std::string &obsData,
		    int stationid,
		    int typeId,
		    const boost::posix_time::ptime &receivedTime,
		    const std::string &logid_,
		    const std::string &decodername_ )
{
	list<kvalobs::kvData> dataList;
	list<kvalobs::kvTextData> textDataList;
	float fval;
	string tmp;
	vector<ParamDef>      params;
	KlDataArray klData;
	pt::ptime obstime;
	int nLineWithData=0; //Number of line with data.
	int nElemsInLine;
	int nExpectedData;
	int line=1;

	logid = logid_;
	decoderName = decodername_;
	messages.erase();

	istringstream istr( obsData );

	if( !getline( istr, tmp ) ) {
		ostringstream o;
		o << "Invalid format. No data. stationid: " << stationid << " typeid: " << typeId;
		messages = o.str();
		return 0;
	}else{
		if( !decodeHeader( tmp, params, messages ) ) {
			ostringstream o;
			o << "INVALID header. stationid: " << stationid << " typeid: " << typeId ;
			messages = o.str();
			return 0;
		}
	}

	if( params.size() < 1 ) {
		messages = "No parameters in header!";
		LOGINFO( "Decoder: '" << decoderName << "'. No parameters in header! Stationid: "
				<< stationid << " typeid: " << typeId );
		IDLOGINFO( logid, "No parameters in header!" << endl << "Header: " << tmp );
		return 0;
	}

	kvalobs::serialize::KvalobsData *kvData = new serialize::KvalobsData();
	string::size_type i;

	while( getline( istr, tmp ) ) {
		line++;
	    if( ! decodeData( klData, params.size(),
	    		          obstime, receivedTime,
	    		          typeId, tmp, line, messages ) ) {
	    	delete kvData;
	    	return 0;
	    }

	    ostringstream ost;
	    ost << "[" << tmp << "]" << endl;

	    for(KlDataArray::size_type index=0; index<klData.size(); index++)
	    	ost << params[index].name() << "("<< params[index].id() << ")["
	    	<< params[index].sensor() << "," << params[index].level() << "]=("
	    	<< klData[index].val()<<"," << klData[index].cinfo()<<"," << klData[index].uinfo()
	    	<< endl;

	    IDLOGDEBUG3( logid, ost.str() );
	    dataList.clear();
	    textDataList.clear();
	    nElemsInLine=0;

	    for( KlDataArray::size_type index=0; index < klData.size(); index++ ) {
	         KlData data = klData[index];

	         if( data.empty() ) {
	            continue;
	         }

	         nExpectedData++;
	         nElemsInLine++;

	         string val=data.val();

	         if( decodeutility::isTextParam( params[index].id() ) ) {
	            kvTextData d( stationid,
	                          obstime,
	                          val,
	                          params[index].id(),
	                          pt::ptime( pt::not_a_date_time ), //Will be set later.
	                          typeId );

	            kvData->insert( d );
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
	               fval = boost::lexical_cast<float>( val );
	            }
	            catch(...){
	               IDLOGERROR( logid, "Invalid value: (" << val << ") not a float!" );
	               continue;
	            }

	            kvalobs::kvData d( stationid,
	            				   obstime,
	            				   fval,
	            				   params[index].id(),
	            				   pt::ptime( pt::not_a_date_time ), //will be set later
	            				   typeId,
	            				   params[index].sensor(),
	            				   params[index].level(),
	            				   fval,
	            				   data.cinfo(),
	            				   data.uinfo(),
	            				   "" );

	            kvData->insert( d );
	         }
	    }

	    if(nElemsInLine>0)
	    	nLineWithData++;
	}

	ostringstream ost;
	ost << "# Lines:             " << line-1 << endl
		<< "# Lines with data:   " << nLineWithData << endl
		<< "# dataelements:      " << nExpectedData;


	IDLOGINFO( logid, ost.str() );
	messages = ost.str();

	return kvData;
}

}
}
}
}
