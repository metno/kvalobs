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
#include "WorkstatistikIterator.h"
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

namespace kvservice
{
class WorkstatistikIterator::Implementation
{
public:
	virtual ~Implementation()
	{
	}
	virtual bool next(kvalobs::kvWorkelement &workstatistik) =0;
	virtual CKvalObs::CService::WorkstatistikIterator_var &getCorbaObjPtr() =0;
	virtual void cleanup() =0;
};

namespace
{
class SimpleWorkstatistikIterator: public WorkstatistikIterator::Implementation
{
public:
	SimpleWorkstatistikIterator(
			const std::vector<kvalobs::kvWorkelement> & elements) :
			elements_(elements), it_(elements_.begin())
	{
	}

	virtual bool next(kvalobs::kvWorkelement &workstatistik)
	{
		if (it_ == elements_.end())
			return false;
		workstatistik = *it_;
		++it_;
		return true;
	}

	virtual CKvalObs::CService::WorkstatistikIterator_var &getCorbaObjPtr()
	{
		throw std::runtime_error("Internal error");
	}

	virtual void cleanup()
	{
		elements_.clear();
		it_ = elements_.end();
	}

private:
	std::vector<kvalobs::kvWorkelement> elements_;
	std::vector<kvalobs::kvWorkelement>::const_iterator it_;
};

class CorbaWorkstatistikIterator: public WorkstatistikIterator::Implementation
{
public:
	CorbaWorkstatistikIterator() :
			iter(CKvalObs::CService::WorkstatistikIterator::_nil()),
			dataList(new CKvalObs::CService::WorkstatistikElemList()),
			index(0)
	{}

	virtual ~CorbaWorkstatistikIterator()
	{
		cleanup();
	}

	virtual bool next(kvalobs::kvWorkelement &ws)
	{
		LogContext context("WorkstatistikIterator::next");

		if (index == dataList->length())
		{
			LOGDEBUG("Fetching data from kvalobs");
			for (int i = 0;; i++)
			{
				try
				{
					bool ok = iter->next(dataList);
					index = 0;
					if (dataList->length() == 0 or not ok)
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
		assert(index < dataList->length());

		const WorkstatistikElem &rd = dataList[index++];
		ws.set(rd.stationID,
				boost::posix_time::time_from_string_nothrow(
						std::string(rd.obstime)), rd.typeID_,
				boost::posix_time::time_from_string_nothrow(
						std::string(rd.tbtime)), rd.priority,
				boost::posix_time::time_from_string_nothrow(
						std::string(rd.processStart)),
				boost::posix_time::time_from_string_nothrow(
						std::string(rd.qaStart)),
				boost::posix_time::time_from_string_nothrow(
						std::string(rd.qaStop)),
				boost::posix_time::time_from_string_nothrow(
						std::string(rd.serviceStart)),
				boost::posix_time::time_from_string_nothrow(
						std::string(rd.serviceStop)));
		return true;
	}
	virtual CKvalObs::CService::WorkstatistikIterator_var &getCorbaObjPtr()
	{
		return iter;
	}
	virtual void cleanup()
	{
		LogContext context("WorkstatistikIterator::cleanup");

		dataList->length(0);
		index = 0;

		if (not CORBA::is_nil(iter))
		{
			LOGDEBUG("Destroying CORBA iterator object");
			try
			{
				iter->destroy();
				iter = CKvalObs::CService::WorkstatistikIterator::_nil();
			} catch (...)
			{
				LOGERROR("Unable to destroy iterator object on server.");
			}
		}
	}

private:
	CKvalObs::CService::WorkstatistikIterator_var iter;
	CKvalObs::CService::WorkstatistikElemList_var dataList;
	CORBA::ULong index;
};

}

WorkstatistikIterator::WorkstatistikIterator() :
		impl_(new CorbaWorkstatistikIterator)
{
}

WorkstatistikIterator::WorkstatistikIterator(
		const std::vector<kvalobs::kvWorkelement> & elements) :
		impl_(new SimpleWorkstatistikIterator(elements))
{
}

WorkstatistikIterator::~WorkstatistikIterator()
{
}

bool WorkstatistikIterator::next(kvalobs::kvWorkelement &ws)
{
	return impl_->next(ws);
}

WorkstatistikIterator_var&
WorkstatistikIterator::getCorbaObjPtr()
{
	return impl_->getCorbaObjPtr();
}

void WorkstatistikIterator::cleanup()
{
	impl_->cleanup();
}
}

