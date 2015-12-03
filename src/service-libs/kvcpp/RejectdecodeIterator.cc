/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: RejectdecodeIterator.cc,v 1.2.2.3 2007/09/27 09:02:46 paule Exp $

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
#include "RejectdecodeIterator.h"
#include <typeinfo>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <miutil/timeconvert.h>
#include <milog/milog.h>
#include <ctime>

#define NDEBUG
#include <cassert>

using namespace std;
using namespace kvalobs;
using namespace CKvalObs::CService;
using namespace milog;

namespace
{
FLogStream *fs = 0;
StdErrStream *trace = 0;
LogLevel getLogLevel(const char *str);
bool setLoglevel(const string &ll, const string &tl);
}

namespace kvservice
{
class RejectDecodeIterator::Implementation
{
public:
	virtual ~Implementation() {}
	virtual bool next(kvalobs::kvRejectdecode &reject) =0;
	virtual CKvalObs::CService::RejectedIterator_var &getCorbaObjPtr() =0;
	virtual void cleanup() =0;
};

namespace
{
class CorbaRejectDecodeIterator: public RejectDecodeIterator::Implementation
{
	CKvalObs::CService::RejectedIterator_var rejectIter;
	CKvalObs::CService::RejectdecodeList_var rejectedList;
	CORBA::ULong index;
public:
	CorbaRejectDecodeIterator() :
			rejectIter(RejectedIterator::_nil()), rejectedList(
					new RejectdecodeList()), index(0)
	{
	}

	~CorbaRejectDecodeIterator()
	{
		cleanup();
	}

	virtual bool next(kvalobs::kvRejectdecode &reject)
	{
		LogContext context("RejectDecodeIterator::next");

		if (index == rejectedList->length())
		{
			LOGDEBUG("Fetching data from kvalobs");
			for (int i = 0;; i++)
			{
				try
				{
					bool ok = rejectIter->next(rejectedList);
					index = 0;
					if (rejectedList->length() == 0 or not ok)
					{
						LOGDEBUG("No more data available");
						return false;
					}
					break;
				} catch (CORBA::TRANSIENT &e)
				{
					if (i < 2)
					{
						LOGWARN("CORBA TRANSIENT exception - retrying...");
						timespec ts;
						ts.tv_sec = 0;
						ts.tv_nsec = 500000000;
						nanosleep(&ts, NULL);
						continue;
					}
					else
					{
						LOGERROR("CORBA TRANSIENT exception - giving up");
						return false;
					}
				} catch (CORBA::Exception &e)
				{
					LOGERROR(
							"Unhandled CORBA exception: " << typeid( e ).name());
					return false;
				}
			}
		}
		assert(index < rejectedList->length());

		const Rejectdecode &rd = rejectedList[index++];
		reject.set(std::string(rd.message),
				boost::posix_time::time_from_string_nothrow(
						(const char *) rd.tbtime), std::string(rd.decoder),
				std::string(rd.comment), bool(rd.is_fixed));
		return true;
	}
	virtual CKvalObs::CService::RejectedIterator_var &getCorbaObjPtr()
	{
		return rejectIter;
	}
	virtual void cleanup()
	{
		LogContext context("RejectDecodeIterator::cleanup");

		rejectedList->length(0);
		index = 0;

		if (not CORBA::is_nil(rejectIter))
		{
			LOGDEBUG("Destroying CORBA iterator object");
			try
			{
				rejectIter->destroy();
				rejectIter = RejectedIterator::_nil();
			} catch (...)
			{
				LOGERROR("Unable to destroy iterator object on server.");
			}
		}
	}
};

class SimpleRejectDecodeIterator: public RejectDecodeIterator::Implementation
{
public:
	SimpleRejectDecodeIterator(const std::vector<kvalobs::kvRejectdecode> & rejected) :
		rejected_(rejected), it_(rejected_.begin())
	{}
	virtual bool next(kvalobs::kvRejectdecode &reject)
	{
		if ( it_ == rejected_.end() )
			return false;
		reject = * it_;
		++ it_;
		return true;
	}
	virtual CKvalObs::CService::RejectedIterator_var &getCorbaObjPtr()
	{
		throw std::runtime_error("Internal error");
	}
	virtual void cleanup()
	{
		rejected_.clear();
		it_ = rejected_.end();
	}

private:
	std::vector<kvalobs::kvRejectdecode> rejected_;
	std::vector<kvalobs::kvRejectdecode>::const_iterator it_;
};
}

RejectDecodeIterator::RejectDecodeIterator() :
		impl_(new CorbaRejectDecodeIterator)
{
}

RejectDecodeIterator::RejectDecodeIterator(
		const std::vector<kvalobs::kvRejectdecode> & rejected) :
				impl_(new SimpleRejectDecodeIterator(rejected))
{
}

RejectDecodeIterator::~RejectDecodeIterator()
{
}

bool RejectDecodeIterator::next(kvRejectdecode &reject)
{
	return impl_->next(reject);
}

RejectedIterator_var & RejectDecodeIterator::getCorbaObjPtr()
{
	return impl_->getCorbaObjPtr();
}

void RejectDecodeIterator::cleanup()
{
	impl_->cleanup();
}
}

