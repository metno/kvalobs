/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: Callback.h,v 1.1.2.4 2007/09/27 09:02:16 paule Exp $

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
#ifndef __kvservice__proxy__Callback__
#define __kvservice__proxy__Callback__

#include "CallbackCollection.h"
#include <kvcpp/kvservicetypes.h>

namespace kvservice
{
namespace proxy
{

class Callback
{
public:
	Callback(CallbackCollection & owner)
	{
		owner.add(this);
	}

	virtual ~Callback()
	{
	}

	virtual void newData(KvDataList &data) =0;

	virtual void newData(KvObsDataList &data)
	{
		typedef KvObsDataList::reverse_iterator RIKvObsDataList;
		for (RIKvObsDataList odl = data.rbegin(); odl != data.rend(); odl++)
		{
			KvDataList &dl = odl->dataList();
			newData(dl);
		}
	}
};
}
}

#endif // __kvservice__proxy__Callback__
