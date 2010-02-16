/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: KvDataSaver.cc,v 1.1.2.2 2007/09/27 09:02:16 paule Exp $                                                       

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
#include "KvDataSaver.h"
#include <milog/milog.h>
#include <string>
#include <exception>

using namespace std;
using namespace dnmi::db;
using namespace milog;

namespace kvservice
{
namespace proxy
{
namespace internal
{
KvDataSaver::KvDataSaver(KvalobsProxy &proxy) :
	proxy(proxy )
{
}

KvDataSaver::~KvDataSaver()
{
}

bool KvDataSaver::next(KvObsDataList & data)
{
	bool ret = true;
	LogContext context("KvDataSaver::next");
	for (IKvObsDataList it = data.begin(); it != data.end(); it++)
	{
		ret &= next(it->dataList() );
	}
	return ret;
}

bool KvDataSaver::next(KvDataList & data)
{

	LOGDEBUG( data.size() << " new pieces of data." );

	for (IKvDataList d = data.begin(); d != data.end(); d++)
	{
		if (proxy.interesting.find(d->paramID() ) == proxy.interesting.end() )
			continue;
		string insertQuery = "insert into data values " + d->toSend();

		KvalobsProxy::Lock lock(proxy.proxy_mutex);
		try
		{
			proxy.connection.exec( insertQuery );
		}
		catch( exception &ex )
		{ // Should have been: dnmi::db::SQLDuplicate &ex ) {
			try
			{
				//LOGDEBUG( ex.what() << ": Could not insert data. Retrying..." );
//				proxy.connection.beginTransaction();
				proxy.connection.exec( "delete from data " + d->uniqueKey() );
				proxy.connection.exec( insertQuery );
//				proxy.connection.endTransaction();
				//LOGDEBUG( "Data was successsfully inserted" );
			}
			catch( exception &ex )
			{
//				proxy.connection.rollBack();
				LOGERROR("Could not insert data! Error: " << ex.what() );
			}
			catch(...)
			{
//				proxy.connection.rollBack();
				LOGERROR("Could not insert data!");
			}
		}
		catch(...)
		{
			LOGERROR( "Error: Unknown reason. Could (probably) not insert data." );
		}
	}
	return true;
}
}
}
}
