/*
 * KvDataHandler.h
 *
 *  Created on: Nov 20, 2015
 *      Author: vegardb
 */

#ifndef SRC_SERVICE_LIBS_KVCPP_SQL_KVDATAHANDLER_H_
#define SRC_SERVICE_LIBS_KVCPP_SQL_KVDATAHANDLER_H_

#include <KvGetDataReceiver.h>
#include <WhichDataHelper.h>
#include <kvdb/kvdb.h>

namespace kvservice
{
namespace sql
{
namespace internal
{

class KvDataHandler
{
public:
	explicit KvDataHandler(dnmi::db::Connection & connection1, dnmi::db::Connection & connection2, KvGetDataReceiver & dataReceiver);
	~KvDataHandler();


	void operator()(const WhichDataHelper & wd);

private:
	std::string query_(const WhichDataHelper & wd, const std::string & table);

	KvGetDataReceiver & dataReceiver_;
	dnmi::db::Connection & connection1_;
	dnmi::db::Connection & connection2_;
};

}
}
}

#endif /* SRC_SERVICE_LIBS_KVCPP_SQL_KVDATAHANDLER_H_ */
