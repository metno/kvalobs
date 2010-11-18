/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  Copyright (C) 2010 met.no

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

#include "ScriptResultIdentifier.h"
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>
#include <ostream>

namespace qabase
{
ScriptResultIdentifier::ScriptResultIdentifier(const std::string & identifier)
{
	//X_1_0_corrected
	static const boost::regex e("(\\w+)_(\\d+)_(\\d+)_([A-Za-z]+)");

	boost::smatch match;
	if ( ! boost::regex_match(identifier, match, e) )
		throw SyntaxError("Unable to parse: " + identifier);

	parameter_ = match[1].str();
	timeIndex_ = boost::lexical_cast<int>(match[2].str());
	stationIndex_ = boost::lexical_cast<int>(match[3].str());
	std::string correctionType = match[4].str();

	if ( correctionType == "flag")
		correctionType_ = Flag;
	else if ( correctionType == "corrected" )
		correctionType_ = Corrected;
	else if ( correctionType == "missing" )
		correctionType_ = Missing;
	else
		throw UndefinedCorrectionType("Undefined correction type: " + correctionType);

}

ScriptResultIdentifier::ScriptResultIdentifier(const std::string & parameter, int timeIndex, int stationIndex, CorrectionType correctionType) :
	parameter_(parameter),
	timeIndex_(timeIndex),
	stationIndex_(stationIndex),
	correctionType_(correctionType)
{}

std::ostream & operator << (std::ostream & s, const ScriptResultIdentifier & sri)
{
	s << sri.parameter() << '_' << sri.timeIndex() << '_' << sri.stationIndex() << '_';
	switch ( sri.correctionType() )
	{
	case ScriptResultIdentifier::Flag:
		return s << "flag";
	case ScriptResultIdentifier::Corrected:
		return s << "corrected";
	case ScriptResultIdentifier::Missing:
		return s << "missing";
	case ScriptResultIdentifier::Undefined:
	default:
		return s << "undefined";
	}
	return s;
}


}
