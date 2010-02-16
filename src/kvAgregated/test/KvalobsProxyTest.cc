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

#include <gtest/gtest.h>
#include <proxy/KvalobsProxy.h>
#include <proxy/ProxyDatabaseConnection.h>
#include <kvcpp/mock/MockKvApp.h>
#include <kvalobs/kvDataOperations.h>
#include <stdexcept>

using namespace kvservice::proxy;

using namespace testing;

class KvalobsProxyTest : public Test
{
protected:
	ProxyDatabaseConnection db;
	std::vector<int> stations;

	typedef testing::NiceMock<testing::MockKvApp> MockKvApp;
	MockKvApp * kvApp;
	KvalobsProxy * proxy;

	kvservice::KvDataList sampleData;
	kvalobs::kvDataFactory factory;

	KvalobsProxyTest() : db(":memory:", true), factory(1, "2010-02-11 00:00:00", 2) {}

	virtual void SetUp()
	{
		kvApp = new MockKvApp;
		proxy = new KvalobsProxy(db, stations);
		for ( int i = 1; i <= 3; ++ i )
			proxy->addInteresting(i);
		// this should cause oldestInProxy to be ignored when searching cache/kvalobs:
		proxy->setOldestInProxy(miutil::miTime());

		sampleData.push_back(factory.getData(1,1));
		sampleData.push_back(factory.getData(1,2));
		sampleData.push_back(factory.getData(1,3));

		kvApp->setupDefaultReturnValues();
	}

	virtual void TearDown()
	{
		delete proxy;
		delete kvApp;
	}


};

TEST_F(KvalobsProxyTest, createWithoutKvApp)
{
	delete kvApp;
	kvApp = 0;
	ASSERT_THROW(KvalobsProxy proxy(db, stations), std::runtime_error);
}

TEST_F(KvalobsProxyTest, sameDataShouldOnlyBeSentOnce)
{
	EXPECT_CALL(* kvApp, sendDataToKv(_,"kv2kvDecoder")).
			Times(1);

	proxy->sendData(sampleData);
	// This should not cause KvApp::sendDataToKv to be invoked, since we have already done this:
	proxy->sendData(sampleData);
}

TEST_F(KvalobsProxyTest, canOverwriteInvalidData)
{
	EXPECT_CALL(* kvApp, sendDataToKv(_,"kv2kvDecoder")).
			Times(2);

	kvservice::KvDataList modifiedList = sampleData;
	std::for_each(modifiedList.begin(), modifiedList.end(), kvalobs::reject);

	proxy->sendData(modifiedList);

	// This should overwrite old invalid data:
	proxy->sendData(sampleData);

	kvservice::KvDataList proxyStore;
	// Get a random row from database
	proxy->getData(proxyStore, 1, "2010-02-10 00:00:00", "2010-02-11 00:00:00", 3, 2, 0,0);

	ASSERT_EQ(1u, proxyStore.size());

	EXPECT_TRUE(kvalobs::valid(proxyStore.front())) << "Invalidated data in database " << proxyStore.front();
}

TEST_F(KvalobsProxyTest, canOverwriteInvalidData2)
{
	EXPECT_CALL(* kvApp, sendDataToKv(_,"kv2kvDecoder")).
			Times(2);

	kvservice::KvDataList modifiedList = sampleData;
	std::for_each(modifiedList.begin(), modifiedList.end(), kvalobs::reject);

	proxy->sendData(modifiedList);

	// This should overwrite old invalid data:
	proxy->sendData(sampleData);

	// No invocation of KvApp::sendDataToKv should be made:
	proxy->sendData(sampleData);
}
