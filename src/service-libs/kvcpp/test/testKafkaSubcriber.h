/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: KvObsData.h,v 1.3.2.3 2007/09/27 09:02:45 paule Exp $

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

#ifndef SRC_SERVICE_LIBS_KVCPP_TEST_TESTKAFKASUBCRIBER_H_
#define SRC_SERVICE_LIBS_KVCPP_TEST_TESTKAFKASUBCRIBER_H_

#include "lib/dnmithread/CommandQue.h"
#include "lib/decodeutility/kvalobsdata.h"
#include "service-libs/kvcpp/kvDataSubscribeInfoHelper.h"


/*
 * This file define a test interface to the broadcast methods in the
 * anonymous namespace in kvcpp/kafka/KafkaSubscribe.cpp. This make them
 * available for unit testing.
 *
 * This interface is not part of the public api and is not installed.
 */


namespace kvservice {
namespace kafka {
namespace test {
void broadcast(const ::kvalobs::serialize::KvalobsData & d,
               const KvDataSubscribeInfoHelper &info,
               dnmi::thread::CommandQue & queue);
}  // namespace test
}  // namespace kafka
}  // namespace kvservice


#endif  // SRC_SERVICE_LIBS_KVCPP_TEST_TESTKAFKASUBCRIBER_H_
