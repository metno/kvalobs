/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: kvalobsdataserializer.cc,v 1.1.2.2 2007/09/27 09:02:27 paule Exp $

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
#include "kvalobsdataserializer.h"
#include "kvalobsdataparser.h"
#include <miutil/timeconvert.h>
#include <libxml++/parsers/domparser.h>
#include <boost/lexical_cast.hpp>
#include <cmath>

using namespace std;
using namespace xmlpp;
using namespace boost;

namespace kvalobs
{

namespace serialize
{

KvalobsDataSerializer::KvalobsDataSerializer()
{
}

KvalobsDataSerializer::KvalobsDataSerializer(const KvalobsData & d) :
	data_(d)
{
}

KvalobsDataSerializer::KvalobsDataSerializer(const std::string & s)
{
	internal::KvalobsDataParser::parse(s, data_);
}

KvalobsDataSerializer::~KvalobsDataSerializer()
{
}

string KvalobsDataSerializer::serialize(const KvalobsData & d)
{
	KvalobsDataSerializer s(d);
	return s.toString();
}

const KvalobsData & KvalobsDataSerializer::toData() const
{
	return data_;
}

KvalobsData & KvalobsDataSerializer::toData()
{
	return data_;
}

namespace
{
template<typename Val>
Element * set_(Element * parent, const std::string & name, Val val)
{
	Element * ret = parent->add_child(name);
	ret->set_attribute("val", lexical_cast<string> (val));
	return ret;
}
}

std::string KvalobsDataSerializer::toString() const
{
	DomParser parser;
	Document * document = parser.get_document();
	Element * root = document->create_root_node("KvalobsData");
	if (data_.overwrite())
		root->set_attribute("overwrite", "1");

	using namespace internal;

	for (Observations::const_iterator station = data_.obs().begin(); station
			!= data_.obs().end(); ++station)
	{
		Element * st = set_(root, "station", station->get());
		for (StationID::const_iterator type = station->begin(); type
				!= station->end(); ++type)
		{
			Element * tp = set_(st, "typeid", type->get());
			for (TypeID::const_iterator obstime = type->begin(); obstime
					!= type->end(); ++obstime)
			{
				Element * ot = set_(tp, "obstime", obstime->get());
				if (obstime->invalidate())
					ot->set_attribute("invalidate", "1");

				// kvData:
				for (ObsTime::const_iterator sensor = obstime->begin(); sensor
						!= obstime->end(); ++sensor)
				{
					Element * snsr = sensor->get() ? set_(ot, "sensor",
							sensor->get()) : ot;
					for (Sensor::const_iterator level = sensor->begin(); level
							!= sensor->end(); ++level)
					{
						Element * lvl = level->get() ? set_(snsr, "level",
								level->get()) : snsr;
						for (Level::const_iterator rest = level->begin(); rest
								!= level->end(); ++rest)
						{
							Element * kvdata = lvl->add_child("kvdata");
							const DataContent & content = rest->content();
							kvdata->set_attribute("paramid", lexical_cast<
									string> (rest->paramID()));
							Element * original = kvdata->add_child("original");
							original->add_child_text(lexical_cast<string> (
									content.original));
							if (std::abs(content.original - content.corrected)
									> 0.01)
							{
								Element * corrected = kvdata->add_child(
										"corrected");
								corrected->add_child_text(
										lexical_cast<string> (content.corrected));
							}
							Element * ci = kvdata->add_child("controlinfo");
							ci->add_child_text(content.controlinfo.flagstring());
							Element * ui = kvdata->add_child("useinfo");
							ui->add_child_text(content.useinfo.flagstring());
							if (not content.cfailed.empty())
							{
								Element * cfailed =
										kvdata->add_child("cfailed");
								cfailed->add_child_text(content.cfailed);
							}
						}
					}
				}

				// kvTextData:
				for (Container<TextDataItem>::const_iterator rest =
						obstime->textData.begin(); rest
						!= obstime->textData.end(); ++rest)
				{
					Element * kvtextdata = ot->add_child("kvtextdata");
					kvtextdata->set_attribute("paramid", lexical_cast<string> (
							rest->paramID()));
					Element * original = kvtextdata->add_child("original");
					original->add_child_text(rest->content().original);
				}
			}
		}
	}

	KvalobsData::RejectList fixedRejected;
	data_.getRejectedCorrections(fixedRejected);
	if ( not fixedRejected.empty() )
	{
		std::map<std::string, KvalobsData::RejectList> decoderSortedRejectList;
		for ( KvalobsData::RejectList::const_iterator it = fixedRejected.begin(); it != fixedRejected.end(); ++ it )
			decoderSortedRejectList[it->decoder()].push_back(* it);

		Element * reject = root->add_child("fixed_rejected_messages");

		for ( std::map<std::string, KvalobsData::RejectList>::const_iterator decoder = decoderSortedRejectList.begin();
				decoder != decoderSortedRejectList.end(); ++ decoder )
		{
			Element * decoderElement = reject->add_child("decoder");
			decoderElement->set_attribute("val", decoder->first);
			for ( KvalobsData::RejectList::const_iterator it = decoder->second.begin(); it != decoder->second.end(); ++ it )
			{

				Element * message = decoderElement->add_child("message");
				message->set_attribute("tbtime", to_kvalobs_string(it->tbtime()));
				message->add_child_text(it->message());
			}
		}
	}

	std::string ret = document->write_to_string_formatted();
	return ret;
}

}
}

