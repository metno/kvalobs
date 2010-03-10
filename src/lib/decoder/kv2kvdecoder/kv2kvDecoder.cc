/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: kv2kvDecoder.cc,v 1.10.2.12 2007/09/27 09:02:29 paule Exp $

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
#include "kv2kvDecoder.h"
#include <decodeutility/kvDataFormatter.h>
#include <decodeutility/kvalobsdataparser.h>
#include <kvalobs/kvexception.h>
#include <kvalobs/kvDataOperations.h>
#include <kvalobs/kvQueries.h>
#include <decoderbase/decoder.h>
#include <puTools/miTime>
#include <milog/milog.h>
#include <kvdb/transactionhelper.h>
#include <memory>
#include <cstdio>
#include <algorithm>
#include <set>
#include <cmath>
#include <sstream>
#include <functional>
#include <stdexcept>

using namespace std;
using namespace kvalobs::decoder;
using namespace kvalobs::serialize;
using namespace decodeutility::kvdataformatter;

namespace kvalobs
{

namespace decoder
{

namespace kv2kvDecoder
{

typedef list<kvData> DList;
typedef list<kvTextData> TDList;

kv2kvDecoder::kv2kvDecoder(dnmi::db::Connection & con,
		const ParamList & params, const list<kvTypes> & typeList,
		const miutil::miString & obsType, const miutil::miString & obs,
		int decoderId) :
	DecoderBase(con, params, typeList, obsType, obs, decoderId), dbGate(&con),
			priority_(5), tbtime(miutil::miTime::nowTime())
{
	milog::LogContext lcontext(name());
	LOGDEBUG( "kv2kvDecoder object created" );
}

kv2kvDecoder::~kv2kvDecoder()
{
}

DecoderBase::DecodeResult kv2kvDecoder::handleError_(
		const kv2kvDecoder::DecoderError & e, miutil::miString & msg)
{
	msg = e.what();
	ostringstream ss;
	ss << msg << endl << "Data was:\n" << obs;
	try
	{
		kvRejectdecode reject(obs, tbtime, name(), msg);
		if (!this->putRejectdecodeInDb(reject))
			throw exception();
	} catch (...)
	{
		ss << endl << "Unable to save message in rejectdecode!";
	}
	LOGERROR( ss.str() );
	return e.res;
}

DecoderBase::DecodeResult kv2kvDecoder::execute(miutil::miString & msg)
{
	milog::LogContext lcontext(name());

	KvalobsData data;
	try
	{
		parse(data, obs);
	} catch (DecoderError & e)
	{
		miutil::miString e_msg = "Could not parse data";
		return handleError_(e, e_msg);
	}
	try
	{
		// Transactions may not be used here - the underlying system uses them.
		//     getConnection()->beginTransaction();
		list<kvData> dl;
		verifyAndAdapt(data, dl);
		list<kvTextData> tdl;
		data.getData(tdl, tbtime);
		save(dl, tdl);
		//     getConnection()->endTransaction();

		msg = "ok";
		return decoder::DecoderBase::Ok;
	} catch (DecoderError & e)
	{
		//     getConnection()->rollBack();
		return handleError_(e, msg);
	}
}

void kv2kvDecoder::parse(KvalobsData & data, const miutil::miString & obs) const
{
	try
	{
		serialize::internal::KvalobsDataParser::parse(obs, data);
	} catch (exception & e)
	{
		LOGERROR( e.what() );
		milog::LogContext lcontext("Fallback");
		LOGDEBUG( "Trying old parsing method" );
		try
		{
			// Fallback on old way of decoding data.
			data = KvalobsData(); // remove any sideeffects from old parse attempt
			DList dlist = decodeutility::kvdataformatter::getKvData(obs);
			data.insert(dlist.begin(), dlist.end());
		} catch (exception & e)
		{
			LOGERROR( e.what() );
			throw DecoderError(decoder::DecoderBase::Rejected,
					"Cannot parse input.");
		}
	}
}

void kv2kvDecoder::verifyAndAdapt(KvalobsData & data, list<kvData> & out)
{
	invalidatePrevious(data);

	data.getData(out, tbtime);

	for (DList::iterator it = out.begin(); it != out.end(); ++it)
	{
		kv2kvDecoder::kvDataPtr dbData = getDbData(*it);
		if (not data.overwrite())
			verify(*it, dbData);
		adapt(*it, dbData, data.overwrite());
	}
}

void kv2kvDecoder::save(const list<kvData> & dl, const list<kvTextData> & tdl)
{
	// kvTextData:
	if (not tdl.empty())
	{
		if (!putkvTextDataInDb(tdl, priority_))
		{
			for (TDList::const_iterator it = tdl.begin(); it != tdl.end(); ++it)
			{
				ostringstream ss;
				ss << "update text_data set original=\'" << it->original()
						<< '\'' << " where stationid=" << it->stationID()
						<< " and obstime=\'" << it->obstime() << '\''
						<< " and paramid=" << it->paramID() << " and typeid="
						<< it->typeID();
				try
				{
					getConnection()->exec(ss.str());
				} catch (dnmi::db::SQLException & e)
				{
					throw(DecoderError(decoder::DecoderBase::Error, e.what()));
				}
			}
		}
	}
	// kvData:
	if (not dl.empty())
	{
		//     if ( ! putKvDataInDb( dl, priority_ ) ) {
		for (DList::const_iterator it = dl.begin(); it != dl.end(); ++it)
		{
			deleteKvDataFromDb(*it);
		}
		if (!putKvDataInDb(dl, priority_))
			throw DecoderError(decoder::DecoderBase::Error,
					"Could not save data");
		//     }
	}
}

namespace
{
bool sensor_eq_(int sensor1, int sensor2)
{
	int res = sensor1 - sensor2;
	return !res or abs(res) == '0';
}

struct lt_kvTextData: public binary_function<kvTextData, kvTextData, bool>
{
	bool operator()(const kvTextData & a, const kvTextData & b) const
	{
		if (a.stationID() != b.stationID())
			return a.stationID() < b.stationID();
		if (a.typeID() != b.typeID())
			return a.typeID() < b.typeID();
		if (a.obstime() != b.obstime())
			return a.obstime() < b.obstime();
		return a.paramID() < b.paramID();
	}
};
struct lt_kvTextData_without_paramID: public binary_function<kvTextData,
		kvTextData, bool>
{
	bool operator()(const kvTextData & a, const kvTextData & b) const
	{
		if (a.stationID() != b.stationID())
			return a.stationID() < b.stationID();
		if (a.typeID() != b.typeID())
			return a.typeID() < b.typeID();
		return a.obstime() < b.obstime();
	}
};
}

void kv2kvDecoder::invalidatePrevious(KvalobsData & data)
{
	list<KvalobsData::InvalidateSpec> inv;
	data.getInvalidate(inv);
	if (inv.empty())
		return;
	invalidatePreviousData(data, inv);
	invalidatePreviousTextData(data, inv);
}

void kv2kvDecoder::invalidatePreviousData(KvalobsData & data, const list<
		KvalobsData::InvalidateSpec> & inv)
{
	list<kvData> tmp;
	data.getData(tmp, tbtime);
	typedef set<kvData, compare::lt_kvData> kvDataSet;
	kvDataSet sentInData(tmp.begin(), tmp.end());

	for (list<KvalobsData::InvalidateSpec>::const_iterator invIt = inv.begin(); invIt
			!= inv.end(); ++invIt)
	{
		DList alreadyInDb;//sentInData;
		if (!dbGate.select(alreadyInDb, kvQueries::selectDataFromType(
				invIt->station, invIt->typeID, invIt->obstime)))
			throw DecoderError(decoder::DecoderBase::Error,
					"Could not get data from database.");
		for (DList::const_iterator db = alreadyInDb.begin(); db
				!= alreadyInDb.end(); ++db)
		{
			if (compare::eq_sensor(0, db->sensor()) and db->level() == 0)
			{
				kvDataSet::const_iterator find = sentInData.find(*db);
				if (find == sentInData.end())
				{
					if (data.overwrite())
					{
						kvDataFactory f(db->stationID(), db->obstime(),
								db->typeID(), db->sensor(), db->level());
						data.insert(f.getMissing(db->paramID()));
					}
					else
					{
						kvData d = *db;
						reject(d);
						data.insert(d);
					}
				}
			}
		}
	}
}

void kv2kvDecoder::invalidatePreviousTextData(KvalobsData & data, const list<
		KvalobsData::InvalidateSpec> & inv)
{
	list<kvTextData> tmp;
	data.getData(tmp, tbtime);
	typedef set<kvTextData, lt_kvTextData> kvDataSet;
	kvDataSet sentInData(tmp.begin(), tmp.end());

	for (list<KvalobsData::InvalidateSpec>::const_iterator invIt = inv.begin(); invIt
			!= inv.end(); ++invIt)
	{
		TDList alreadyInDb;
		if (!dbGate.select(alreadyInDb, kvQueries::selectTextData(
				invIt->station, invIt->obstime, invIt->obstime)))
			throw DecoderError(decoder::DecoderBase::Error,
					"Could not get data from database.");
		for (TDList::const_iterator db = alreadyInDb.begin(); db
				!= alreadyInDb.end(); ++db)
		{
			if (db->typeID() == invIt->typeID)
			{
				kvDataSet::const_iterator find = sentInData.find(*db);
				if (find == sentInData.end())
				{
					if (data.overwrite())
					{
						kvTextData td(db->stationID(), db->obstime(), "",
								db->paramID(), tbtime, db->typeID());
						data.insert(td);
					}
					else
					{
						throw DecoderError(decoder::DecoderBase::Rejected,
								"Illegal operation: Cannot overwrite text_data");
					}
				}
			}
		}
	}
}

kv2kvDecoder::kvDataPtr kv2kvDecoder::getDbData(const kvData d)
{
	milog::LogContext lcontext("adapt");

	list<kvData> dbData;
	if (dbGate.select(dbData, kvQueries::selectData(d)))
	{
		// Lookup is done so that only a single instance will match
		if ( dbData.size() > 1 )
			throw DecoderError(decoder::DecoderBase::Error, "Too many rows returned by query");
		if (dbData.empty())
		{
			LOGDEBUG( "No match for data " << d << " in database" );
			return kv2kvDecoder::kvDataPtr();
		}
		LOGDEBUG( "Data from database: " << dbData.front() );
		return kv2kvDecoder::kvDataPtr(new kvData(dbData.front()));
	}
	throw DecoderError(decoder::DecoderBase::Error,
			"Problem with database lookup:\n" + kvQueries::selectData(d));
}

void kv2kvDecoder::verify(const kvData & d, kvDataPtr dbData) const
{
	const float delta = 0.0999;
	if (dbData.get() and abs(dbData->original() - d.original()) > delta)
	{
		ostringstream ss;
		ss << "New data is not compatible with old in database." << endl
				<< "Station: " << d.stationID() << endl << "Values: DB = "
				<< dbData->original() << ". New = " << d.original();
		throw DecoderError(decoder::DecoderBase::Rejected, ss.str());
	}
}

void kv2kvDecoder::adapt(kvData & d, kvDataPtr dbData, bool overwrite) const
{
	milog::LogContext lcontext("adapt");

	LOGDEBUG( "dbData.get():\t" << dbData.get() );
	LOGDEBUG( "overwrite:\t" << overwrite );

	if (dbData.get() and not overwrite)
		d.set(d.stationID(), d.obstime(), dbData->original(), d.paramID(),
				dbData->tbtime(), d.typeID(), d.sensor(), d.level(),
				d.corrected(), d.controlinfo(), d.useinfo(), d.cfailed());
}

}

}

}
