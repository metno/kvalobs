/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: AgregatorHandler.h,v 1.1.2.3 2007/09/27 09:02:15 paule Exp $

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
#ifndef __agregator__AgregatorHandler_h__
#define __agregator__AgregatorHandler_h__

#include "AbstractAgregator.h"
#include "proxy/Callback.h"
#include "proxy/KvalobsProxy.h"
#include <map>

namespace agregator
{
class GenerateZero;

class AgregatorHandler: public kvservice::proxy::Callback
{
	static AgregatorHandler *agHandler;
	friend class agregator::GenerateZero;

public:

	AgregatorHandler(kvservice::proxy::KvalobsProxy &proxy);
	virtual ~AgregatorHandler();

	virtual void newData(kvservice::KvDataList &data);

	void process(const kvalobs::kvData & data);

	void save(const kvalobs::kvData & d);

	void setParameterFilter(const std::vector<int> & allowedParameters)
	{
		allowedParameters_ = allowedParameters;
	}

	void addHandler(AbstractAgregator *handler);

	virtual std::list<kvalobs::kvData>
	getRelevantObsList(const kvalobs::kvData & data,
			const AbstractAgregator::TimeSpan & obsTimes);

private:
	typedef std::pair<int, AbstractAgregator *> Handler;
	typedef std::multimap<int, AbstractAgregator *> HandlerMap;

	kvservice::proxy::KvalobsProxy & proxy_;

	HandlerMap handlers;

	std::vector<int> allowedParameters_;
};
}

#endif // __agregator__AgregatorHandler_h__
