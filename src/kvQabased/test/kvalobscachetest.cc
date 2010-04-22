/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: kvalobscachetest.cc,v 1.1.2.7 2007/09/27 09:02:21 paule Exp $

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

#include <gtest/gtest.h>
#include "../runcheck/kvalobscache.h"
#include "../kvQABaseDBConnection.h"
#include "database/kvalobsdatabase.h"
#include <kvalobs/kvDataOperations.h>
#include <boost/shared_ptr.hpp>

using namespace dnmi::db;

class KvalobsCacheTest: public testing::Test
{
public:
	KvalobsCacheTest()
	{
		db = new KvalobsDatabase;
		qabaseConnection = new kvQABaseDBConnection(db->getConnection());
		cache = new KvalobsCache(*qabaseConnection);
	}
	~KvalobsCacheTest()
	{
		delete cache;
		delete qabaseConnection;
		qabaseConnection = 0;
		delete db;
	}

protected:
	KvalobsCache * cache;
	kvQABaseDBConnection * qabaseConnection;
	KvalobsDatabase * db;

};

TEST_F(KvalobsCacheTest, testSave)
{
	kvalobs::kvDataFactory f(42, "2006-05-26 06:00:00", 302);
	kvalobs::kvData a = f.getData(3, 110);
	db->getConnection()->exec("INSERT INTO data VALUES " + a.toSend());

	a.corrected(42);
	cache->insert(a);

	delete cache; // delete cache object - forcing a flushing of the cache.
	cache = 0;

	// We update useinfo here, because the cache should also update useinfo
	// before saving.
	kvalobs::kvUseInfo ui = a.useinfo();
	ui.setUseFlags(a.controlinfo());
	a.useinfo(ui);

	boost::shared_ptr<Result> res(db->getConnection()->execQuery(
			"select * from data;"));

	ASSERT_EQ( 1, res->size() );
	kvalobs::kvData data( res->next() );

	EXPECT_EQ( float( 42 ), data.corrected() );
	kvalobs::compare::exactly_equal eq;
	EXPECT_TRUE( eq( a, data ) );
}

TEST_F(KvalobsCacheTest, testOverwriteFirst)
{
	kvalobs::kvDataFactory f(42, "2006-05-26 06:00:00", 302);
	kvalobs::kvData a = f.getData(3, 110);
	db->getConnection()->exec("INSERT INTO data VALUES " + a.toSend());

	a.corrected(50);
	cache->insert(a);

	a.corrected(100);
	cache->insert(a);

	delete cache; // delete cache object - forcing a flushing of the cache.
	cache = 0;

	boost::shared_ptr<Result> res(db->getConnection()->execQuery(
			"select * from data;"));

	ASSERT_EQ( 1, res->size() );
	kvalobs::kvData data( res->next() );
	EXPECT_EQ( float( 100 ), data.corrected() );
}
