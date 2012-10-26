/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: KvObsData.cc,v 1.2.2.5 2007/09/27 09:02:46 paule Exp $                                                       

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
#include "KvObsData.h"

using namespace kvalobs;
using namespace CKvalObs::CService;

namespace kvservice
{
  KvObsData::KvObsData()
    : stationid_( 0 )
  {
  }

  KvObsData::KvObsData( const ObsData &obsData )
    : stationid_( 0 )
  {
    operator=( obsData );
  }

  void KvObsData::operator=( const ObsData &obsData_ )
  {
    ObsData &obsData = const_cast<ObsData &>( obsData_ );

    clear();

    if ( obsData.dataList.length() ) {
      stationid_ = obsData.dataList[0].stationID;
      for ( CORBA::ULong i = 0; i < obsData.dataList.length(); i++ ) {
	DataElem &d = obsData.dataList[ i ];
	kvData data( d.stationID, boost::posix_time::time_from_string( (const char*) d.obstime ), d.original, d.paramID,
			boost::posix_time::time_from_string( (const char*) d.tbtime ), d.typeID_,
		     strlen(d.sensor) > 0 ? (int) *d.sensor : 0,
		     d.level, d.corrected, kvControlInfo((char*)d.controlinfo), 
		     kvUseInfo((char*)d.useinfo), std::string(d.cfailed) );
	dataList().push_back( data );
      }
    }
    if ( obsData.textDataList.length() ) {
      if(stationid_==0)
      	stationid_ = obsData.textDataList[0].stationID;
      	
      for ( CORBA::ULong i = 0; i < obsData.textDataList.length(); i++ ) {
	TextDataElem &d = obsData.textDataList[ i ];
	kvTextData textData( d.stationID, boost::posix_time::time_from_string((const char*) d.obstime),
			     std::string(d.original), d.paramID, 
			     boost::posix_time::time_from_string((const char*) d.tbtime), d.typeID_ );
	textDataList_.push_back( textData );
      }
    }
  }
  
  KvObsData::~KvObsData( )
  {
  }

  void KvObsData::clear()
  {
    dataList().clear();
    textDataList().clear();
    stationid_ = 0;
  }
}
