/*
  Kvalobs - Free Quality Control Software for Meteorological Observations

  $Id: decodeutility.h,v 1.1.2.3 2007/09/27 09:02:27 paule Exp $

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

#include <boost/algorithm/string/case_conv.hpp>
#include "isTextParam.h"

#if 0
namespace{
struct TextParam{
   const char *name;
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
}
#endif



namespace decodeutility {

bool
isTextParam( int paramid )
{
	return paramid >= 1000;
}

bool
isTextParam( const std::string &paramName, const ParamList &params )
{
	ParamList::const_iterator it = params.find( Param( paramName, -1 ) );

	if( it == params.end() ) {
		it = params.find( Param( paramName , -1 ) );
		if( it == params.end() )
			return false;
	}

	return isTextParam( it->id() );
}

#if 0
bool
isTextParam( int paramid )
{
	for(int i=0; textParams[i].name; i++){
		if(textParams[i].paramid==paramid)
			return true;
	}
	return false;
}
#endif

}

