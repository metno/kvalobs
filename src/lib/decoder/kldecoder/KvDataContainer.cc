/*
  Kvalobs - Free Quality Control Software for Meteorological Observations

  $Id: kldata.h,v 1.1.2.3 2007/09/27 09:02:29 paule Exp $

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

#include "KvDataContainer.h"

using namespace kvalobs::serialize::internal;

namespace kvalobs{
namespace decoder{
namespace kldecoder{

///Deletes kvData after it is consumed.
KvDataContainer::
KvDataContainer( kvalobs::serialize::KvalobsData *kvData )
: data_( kvData )
{
}


KvDataContainer::
~KvDataContainer() {
	delete data_;
}

int
KvDataContainer::
getTextData( TextData &textData, const boost::posix_time::ptime &tbtime )const
{
	int n=0;
	textData.clear();
	const Observations &obs_ = data_->obs();

	for (Observations::const_iterator s = obs_.begin(); s != obs_.end(); ++s)
	{
		for (StationID::const_iterator t = s->begin(); t != s->end(); ++t)
		{
			for (TypeID::const_iterator o = t->begin(); o != t->end(); ++o)
			{
				for (Container<TextDataItem>::const_iterator param =
						o->textData.begin(); param != o->textData.end(); ++param)
				{
					kvTextData d(s->get(), o->get(), param->content().original,
							     param->paramID(), tbtime, t->get());
					textData[s->get()][t->get()][o->get()].push_back(d);
					n++;
				}
			}
		}
	}
	return n;
}

int
KvDataContainer::
getData( Data &data, const boost::posix_time::ptime &tbtime )const
{
	int n=0;
	data.clear();

	const Observations &obs_ = data_->obs();

	for (Observations::const_iterator s = obs_.begin(); s != obs_.end(); ++s)
	{
		for (StationID::const_iterator t = s->begin(); t != s->end(); ++t)
		{
			for (TypeID::const_iterator o = t->begin(); o != t->end(); ++o)
			{
				for (ObsTime::const_iterator sensor = o->begin(); sensor
						!= o->end(); ++sensor)
				{
					for (Sensor::const_iterator level = sensor->begin(); level
							!= sensor->end(); ++level)
					{
						for (Level::const_iterator param = level->begin(); param
								!= level->end(); ++param)
						{

							const DataContent & c = param->content();

							kvData d(s->get(), o->get(), c.original,
									param->paramID(), tbtime, t->get(),
									sensor->get(), level->get(), c.corrected,
									c.controlinfo, c.useinfo, c.cfailed);

							data[s->get()][t->get()][o->get()].push_back(d);
							n++;
						}
					}
				}
			}
		}
	}
	return n;
}

int
KvDataContainer::
get( TextData &textData, Data &data, const boost::posix_time::ptime &tbtime )const
{
	return getTextData( textData, tbtime ) + getData( data, tbtime );
}

int
KvDataContainer::
getTextData( TextDataByObstime &textData,
		     int stationid, int typeId,
		     const boost::posix_time::ptime &tbtime )const
{
	int n=0;
	textData.clear();
	const Observations &obs_ = data_->obs();


	Observations::const_iterator s = obs_.find( stationid );

	if( s == obs_.end() )
		return 0;

	StationID::const_iterator t = s->find( typeId) ;
	if( t == s->end() )
		return 0;


	for (TypeID::const_iterator o = t->begin(); o != t->end(); ++o)
	{
		for (Container<TextDataItem>::const_iterator param =
			  o->textData.begin(); param != o->textData.end(); ++param)
		{
					kvTextData d(s->get(), o->get(), param->content().original,
							     param->paramID(), tbtime, t->get());
					textData[o->get()].push_back(d);
					n++;
		}
	}

	return n;
}

int
KvDataContainer::
getData( DataByObstime &data,
         int stationid, int typeId,
         const boost::posix_time::ptime &tbtime )const
{
	int n=0;
	data.clear();

	const Observations &obs_ = data_->obs();

	Observations::const_iterator s = obs_.find( stationid );

	if( s == obs_.end() )
		return 0;


	StationID::const_iterator t = s->find( typeId);
	if( t == s->end() )
		return 0;

	for ( TypeID::const_iterator o = t->begin(); o != t->end(); ++o)
	{
		for ( ObsTime::const_iterator sensor = o->begin();
			  sensor != o->end(); ++sensor )
		{
			for ( Sensor::const_iterator level = sensor->begin();
				  level != sensor->end(); ++level)
			{
				for ( Level::const_iterator param = level->begin();
					  param != level->end(); ++param )
				{
					const DataContent & c = param->content();

					kvData d(s->get(), o->get(), c.original,
							 param->paramID(), tbtime, t->get(),
							 sensor->get(), level->get(), c.corrected,
							 c.controlinfo, c.useinfo, c.cfailed);

					data[o->get()].push_back(d);
					n++;
				}
			}
		}
	}

	return n;
}

int
KvDataContainer::
get( DataByObstime &data,
     TextDataByObstime &textData,
     int stationid, int typeId,
     const boost::posix_time::ptime &tbtime)const
{
	return getTextData( textData, stationid, typeId, tbtime ) +
		   getData( data, stationid, typeId, tbtime );
}



bool
KvDataContainer::
getData( kvalobs::kvData &data, int stationid, int typeId, int paramid, const boost::posix_time::ptime &obstime, char sensor, int level )const
{
    DataByObstime tmpData;

    if( getData( tmpData, stationid, typeId ) == 0 )
        return false;

    const DataList &theData = tmpData[obstime];

    for( DataList::const_iterator it = theData.begin();
         it != theData.end(); ++it ){
        if(it->paramID() == paramid && it->sensor() == sensor && it->level() == level ) {
            data = *it;
            return true;
        }
    }

    return false;
}

bool
KvDataContainer::
getTextData( kvalobs::kvTextData &data, int stationid, int typeId, int paramid, const boost::posix_time::ptime &obstime )const
{
    TextDataByObstime tmpData;

    if( getTextData( tmpData, stationid, typeId ) == 0 )
        return false;

    const TextDataList &theData = tmpData[obstime];

    for( TextDataList::const_iterator it = theData.begin();
            it != theData.end(); ++it ){
        if(it->paramID() == paramid  ) {
            data = *it;
            return true;
        }
    }

    return false;
}

///The total count in this collection
int
KvDataContainer::
count()const
{
    Data data;
    TextData textData;

    return getData( data ) + getTextData( textData );
}

///The total count of data for a specific stationid, typeid and obstime.
int
KvDataContainer::
count(int stationid, int typeId, const boost::posix_time::ptime &obstime )const
{
    DataByObstime data;
    TextDataByObstime textData;

    int n = getData( data, stationid, typeId ) + getTextData( textData, stationid, typeId );

    if( obstime.is_special() )
        return n;

    return data[obstime].size() + textData[obstime].size();
}



///Deletes kvData after it is consumed.
void KvDataContainer::
set( kvalobs::serialize::KvalobsData *kvData_ )
{
	delete data_;
	data_ = kvData_;
}


}
}
}

