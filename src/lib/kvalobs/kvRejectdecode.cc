/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: kvRejectdecode.cc,v 1.6.6.2 2007/09/27 09:02:30 paule Exp $

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
#include <kvalobs/kvRejectdecode.h>

using namespace std;
using namespace miutil;

std::string kvalobs::kvRejectdecode::toSend() const
{
	ostringstream ost;
	ost << "(" << quoted(message_) << "," << quoted(tbtime_) << ","
			<< quoted(decoder_) << "," << quoted(comment_) << ")";
	return ost.str();
}

bool kvalobs::kvRejectdecode::set(const std::string &message__,
		const miTime &tbtime__, const std::string &decoder__,
		const std::string &comment__, bool fixed)
{
	message_ = message__;
	tbtime_ = tbtime__;
	decoder_ = decoder__;
	comment_ = comment__;
	fixed_ = fixed;
	sortBy_ = tbtime_.isoTime();
}

bool kvalobs::kvRejectdecode::set(const dnmi::db::DRow& r_)
{
	dnmi::db::DRow &r = const_cast<dnmi::db::DRow&>(r_);
	string buf;
	list<string> names = r.getFieldNames();
	list<string>::iterator it = names.begin();

	fixed_ = false;
	for (; it != names.end(); it++)
	{
		try
		{
			buf = r[*it];
			if (*it == "message")
				message_ = buf;
			else if (*it == "tbtime")
				tbtime_ = miTime(buf);
			else if (*it == "decoder")
				decoder_ = buf;
			else if (*it == "comment")
				comment_ = buf;
			else if (*it == "fixed")
			{
				std::cout << buf << std::endl;
				if ((not buf.empty()) and (buf[0] == 't' or buf[0] == 'T'))
					fixed_ = true;
			}
		} catch (...)
		{
			CERR("kvRejectdecode: exception ..... \n");
		}
	}
	sortBy_ = tbtime_.isoTime();
	return true;
}

std::string kvalobs::kvRejectdecode::uniqueKey() const
{
	ostringstream ost;

	ost << " WHERE  tbtime=" << quoted(tbtime_.isoTime()) << " AND "
			<< "        message=" << quoted(message_) << " AND "
			<< "        decoder=" << quoted(decoder_);

	return ost.str();
}
