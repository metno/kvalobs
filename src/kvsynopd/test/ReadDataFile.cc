/*
  Kvalobs - Free Quality Control Software for Meteorological Observations

  $Id: synop.h,v 1.12.2.5 2007/09/27 09:02:18 paule Exp $

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

#include <iostream>
#include <fstream>
#include <test/ReadDataFile.h>
#include <miutil/commastring.h>
#include <miutil/trimstr.h>
#include <Data.h>

using namespace std;

bool
readDataFile( const std::string &filename, DataEntryList &data, const miutil::miTime &fromtime )
{
	string file=string(TESTDATADIR) + "/" + filename;
	ifstream fin;
	string line;
	miutil::miTime obstime;

	data.clear();

	fin.open( file.c_str() );

	if( ! fin.is_open() )
		return false;

	while( getline( fin, line ) ) {

		string::size_type i = line.find( "#@#"); //Comment

		if( i != string::npos )
			line.erase( i );

		miutil::trimstr( line );

		if( line.empty() )
			continue;


		miutil::CommaString dataValues( line, '|' );

		if( dataValues.size() != 12 )
			continue;

		obstime = miutil::miTime( dataValues[1] );

		if( ! fromtime.undef() && obstime > fromtime )
			continue;

		Data d( atoi( dataValues[0].c_str() ), obstime,
				dataValues[2], atoi( dataValues[3].c_str() ) ,
				atoi( dataValues[5].c_str() ), atoi( dataValues[6].c_str() )+'0',
				atoi( dataValues[7].c_str() ), dataValues[9], dataValues[10] );

		data.insert( d );
	}
/*
	for( DataEntryList::CITDataEntryList itd=data.begin(); itd!=data.end(); ++itd ) {
		std::list<int> types = itd->getTypes();

		for( std::list<int>::iterator tit=types.begin(); tit!=types.end(); ++tit ) {
			DataListEntry::TDataList dl = itd->getTypeId( *tit );

			for( DataListEntry::CITDataList dit=dl.begin(); dit!=dl.end(); ++dit )
				cerr << *dit << endl;
		}
 	}
*/
	return true;
}

bool
loadSynopDataFromFile( const std::string &filename,
					   StationInfoPtr      info,
					   SynopDataList       &sd,
					   const miutil::miTime &fromtime )
{
	DataEntryList rawdata;

	sd.clear();

	if( !info ) {
		cerr << "loadSynopDataFromFile: StationInfoPtr == 0 " << endl;
		return false;
	}

	if( ! readDataFile( filename, rawdata, fromtime ) ) {
		cerr << "Failed to read datafile <" << filename << ">." << endl;
		return false;
	}

	loadSynopData( rawdata, sd, info );

	//cerr << sd << endl;
	return true;
}

