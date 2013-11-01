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

#ifndef __DATADECODER_H__
#define __DATADECODER_H__

#include <string>
#include <list>
#include <vector>
#include <boost/date_time/posix_time/ptime.hpp>
#include <milog/milog.h>
#include <kvalobs/paramlist.h>
#include <decodeutility/kvalobsdata.h>
#include "paramdef.h"
#include "kldata.h"

namespace kvalobs{
namespace decoder{
namespace kldecoder{
namespace bits {

struct DataDecoder {
	const ParamList &params;
	const std::list<kvalobs::kvTypes> &types;
	std::string logid;
	bool warnings;
	std::string decoderName;
	std::string messages;

	std::string toupper(const std::string &s)const;

	DataDecoder( const ParamList &paramList, const std::list<kvalobs::kvTypes> &typeList )
		:params( paramList ), types( typeList ), warnings(false){}

	//Return -1 if the paramname do not exist.
	int findParamId( const std::string &paramname )const;

	bool splitData(const std::string &data,
			       std::list<std::string> &datalist,
			       std::string &msg)const {
		return splitString( data, datalist, 2, msg );
	}


	bool decodeData( KlDataArray &da,
			         KlDataArray::size_type daSize,
			         boost::posix_time::ptime &obstime,
					 const boost::posix_time::ptime &receivedTime,
					 const int typeId,
	                 const std::string &sdata,
	                 int line,
	                 std::string &msg);

	bool splitParams( const std::string &header,
			          std::list<std::string> &params,
		              std::string &msg)const {
		return splitString( header, params, 2, msg);
	}


	bool splitString( const std::string &header,
	                  std::list<std::string> &params,
	                  int maxNumberOfOtionalElements,
	                  std::string &msg)const;

	///throws logical_error if duplicate params is defined.
	void updateParamList( std::vector<ParamDef> &paramsList, const ParamDef &param );
	bool decodeHeader( const std::string &header,
	                   std::vector<ParamDef> &params,
	                   std::string &message);
	int hexCharToInt( char c) const;
	kvalobs::kvUseInfo setUseinfo( const std::string &flags  )const;
	kvalobs::kvControlInfo setControlInfo( const std::string &flags )const;

	kvalobs::serialize::KvalobsData*
		decodeData( const std::string &obsData,
					int stationid,
					int typeId,
				    const boost::posix_time::ptime &receivedTime,
				    const std::string &logid,
				    const std::string &decodername );
};

}
}
}
}



#endif
