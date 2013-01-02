/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: kvGate.cc,v 1.2.6.2 2007/09/27 09:02:30 paule Exp $

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

/* Created by DNMI/PU: j.schulze@met.no
 at Wed Aug 28 14:09:50 2002 */

using namespace std;

bool kvGate::select(list<kvAlgorithms>& list_, std::string q)
{
	std::string query = kvAlgorithms::selectAllQuery() + q;

	Result *res = 0;

	try
	{
		res = con.execQuery(query);

		while (res->hasNext())
		{
			DRow & row = res->next();
			list_.push_back();
			list[list.size() - 1].set(row);
		}
		delete res;
		return true;
	} catch (SQLException & ex)
	{
		delete res;
		CERR("EXCEPTION: KvDataProxy::getData: " << ex.what() << endl);
	} catch (...)
	{
		delete res;
		CERR("EXCEPTION: KvDataProxy::getData: Unknown exception!\n");
	}

	return false;
}
;
