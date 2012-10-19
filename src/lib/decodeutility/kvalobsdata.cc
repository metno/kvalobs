/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: kvalobsdata.cc,v 1.1.2.2 2007/09/27 09:02:27 paule Exp $

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
#include "kvalobsdata.h"
#include <kvalobs/kvDataOperations.h>
#include <kvalobs/kvTextData.h>
#include <boost/any.hpp>
#include <stack>

using namespace std;

namespace kvalobs
{
namespace serialize
{

KvalobsData::KvalobsData() :
	overwrite_(false)
{
}

KvalobsData::KvalobsData(const std::list<kvData> & data, const std::list<
		kvTextData> & tdata) :
	overwrite_(false)
{
	for (list<kvData>::const_iterator it = data.begin(); it != data.end(); ++it)
		insert(*it);
	for (list<kvTextData>::const_iterator it = tdata.begin(); it != tdata.end(); ++it)
		insert(*it);
}

KvalobsData::~KvalobsData()
{
}

bool KvalobsData::empty() const
{
	return obs_.count() == 0;
}

size_t KvalobsData::size() const
{
	return obs_.count();
}

void KvalobsData::insert(const kvData & d)
{
	int sensor = d.sensor();
	if ( sensor >= '0' )
		sensor -= '0';

	obs_[d.stationID()][d.typeID()][d.obstime()][sensor][d.level()][d.paramID()].content()
			= d;
}

void KvalobsData::insert(const kvTextData & d)
{
	obs_[d.stationID()][d.typeID()][d.obstime()].textData[d.paramID()].content()
			= d;
}

void KvalobsData::getData(list<kvData> & out, const miutil::miTime & tbtime) const
{
	using namespace internal;
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

							const internal::DataContent & c = param->content();
							kvData d(s->get(), o->get(), c.original,
									param->paramID(), tbtime, t->get(),
									sensor->get(), level->get(), c.corrected,
									c.controlinfo, c.useinfo, c.cfailed);
							out.push_back(d);
						}
					}
				}
			}
		}
	}
}

void KvalobsData::getData(list<kvTextData> & out, const miutil::miTime & tbtime) const
{
	using namespace internal;
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
					out.push_back(d);
				}
			}
		}
	}
}

void KvalobsData::invalidate(bool doit, int station, int typeID,
		const miutil::miTime & obstime)
{
	obs_[station][typeID][obstime].invalidate(doit);
}

bool KvalobsData::isInvalidate(int station, int typeID,
		const miutil::miTime & obstime) const
{
	return obs_[station][typeID][obstime].invalidate();
}

void KvalobsData::getInvalidate(std::list<InvalidateSpec> & invSpec)
{
	using namespace internal;
	for (Observations::const_iterator s = obs_.begin(); s != obs_.end(); ++s)
	{
		for (StationID::const_iterator t = s->begin(); t != s->end(); ++t)
		{
			for (TypeID::const_iterator o = t->begin(); o != t->end(); ++o)
			{
				if (o->invalidate())
					invSpec.push_back(InvalidateSpec(s->get(), t->get(),
							o->get()));
			}
		}
	}
}

}

}
