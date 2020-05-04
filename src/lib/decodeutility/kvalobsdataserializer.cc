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
#include <mutex>

using namespace std;
using namespace xmlpp;
using namespace boost;
using boost::posix_time::ptime;


namespace kvalobs {

namespace serialize {


namespace {
struct BCI {
  string producer_;
};
  
static  std::mutex bciMu;
static map<KvalobsDataSerializer*, BCI *> bciMap;

BCI *getBci(KvalobsDataSerializer *t) {
  lock_guard<std::mutex> lck(bciMu);
  auto it=bciMap.find(t);
  if( it != bciMap.end()){
    return it->second;
  }
  return nullptr;
}


//create the BCI if it do not exist.
//return a pointer to the BCI.
BCI *setBci(KvalobsDataSerializer *t) {
  lock_guard<std::mutex> lck(bciMu);
  auto it=bciMap.find(t);
  if( it != bciMap.end()){
    return it->second;
  } 
  auto p=new BCI;
  bciMap[t]=p;
  return p;
}

void delBci(KvalobsDataSerializer *t) {
  lock_guard<std::mutex> lck(bciMu);
  auto it=bciMap.find(t);
  if( it != bciMap.end()){
     delete it->second;
     bciMap.erase(it);
  } 
}
}


KvalobsDataSerializer::KvalobsDataSerializer() {
}

KvalobsDataSerializer::KvalobsDataSerializer(const KvalobsData & d, const std::string &producer)
    : data_(d) {
      setBci(this)->producer_=producer;
}

KvalobsDataSerializer::KvalobsDataSerializer(const KvalobsData & d)
    : data_(d) {
}

KvalobsDataSerializer::KvalobsDataSerializer(const std::string & s) {
  internal::KvalobsDataParser::parse(s, data_);
}

KvalobsDataSerializer::~KvalobsDataSerializer() {
  delBci(this);
}

std::string KvalobsDataSerializer::defaultProducer="";

std::string KvalobsDataSerializer::producer()const{
  auto p=getBci(const_cast<KvalobsDataSerializer*>(this));
  if( !p) {
    return defaultProducer;
  }

  return p->producer_;
}

string KvalobsDataSerializer::serialize(const KvalobsData & d, const std::string &producer) {
  KvalobsDataSerializer s(d, producer);
  if( d.created().is_special())
    return s.toString();
  else
    return s.toString(d.created());
}

string KvalobsDataSerializer::serialize(const KvalobsData & d) {
  KvalobsDataSerializer s(d);
  if( d.created().is_special())
    return s.toString();
  else
    return s.toString(d.created());
}


const KvalobsData & KvalobsDataSerializer::toData() const {
  return data_;
}

KvalobsData & KvalobsDataSerializer::toData() {
  return data_;
}

namespace {
template<typename Val>
Element * set_(Element * parent, const std::string & name, Val val) {
  Element * ret = parent->add_child(name);
  ret->set_attribute("val", lexical_cast<string>(val));
  return ret;
}
}

std::string KvalobsDataSerializer::toString() const {
  ptime created=boost::posix_time::second_clock::universal_time();
  return toString(created);
}

std::string KvalobsDataSerializer::toString(const boost::posix_time::ptime &created) const {
  string myProducer;
  DomParser parser;
  Document * document = parser.get_document();
  Element * root = document->create_root_node("KvalobsData");
  if (data_.overwrite())
    root->set_attribute("overwrite", "1");

  
  if(!producer().empty()) {
    myProducer=producer();
  } else if( !data_.producer().empty() ){
    myProducer=data_.producer();
  }

  if( ! myProducer.empty() ) {
    root->set_attribute("producer", myProducer);
  }

  if( ! created.is_special() )
    root->set_attribute("created", to_kvalobs_string(created));

  using namespace internal;

  for (Observations::const_iterator station = data_.obs().begin();
      station != data_.obs().end(); ++station) {
    Element * st = set_(root, "station", station->get());
    for (StationID::const_iterator type = station->begin();
        type != station->end(); ++type) {
      Element * tp = set_(st, "typeid", type->get());
      for (TypeID::const_iterator obstime = type->begin();
          obstime != type->end(); ++obstime) {
        Element * ot = set_(tp, "obstime", to_kvalobs_string(obstime->get()));
        if (obstime->invalidate())
          ot->set_attribute("invalidate", "1");

        for (ObsTime::const_iterator tbt = obstime->begin();
            tbt != obstime->end(); ++tbt) {
          string tbtime =
              tbt->get().is_special() ? "" : to_kvalobs_string(tbt->get());
          Element * tt = set_(ot, "tbtime", tbtime);

          // kvData:
          for (TbTime::const_iterator sensor = tbt->begin();
              sensor != tbt->end(); ++sensor) {
            Element * snsr = set_(tt, "sensor", sensor->get());
            for (Sensor::const_iterator level = sensor->begin();
                level != sensor->end(); ++level) {
              Element * lvl =set_(snsr, "level", level->get());
              for (Level::const_iterator rest = level->begin();
                  rest != level->end(); ++rest) {
                Element * kvdata = lvl->add_child("kvdata");
                const DataContent & content = rest->content();
                kvdata->set_attribute("paramid",
                                      lexical_cast<string>(rest->paramID()));
                Element * original = kvdata->add_child("original");
                original->add_child_text(
                    lexical_cast<string>(content.original));
                Element * corrected = kvdata->add_child("corrected");
                corrected->add_child_text(
                      lexical_cast<string>(content.corrected));
                Element * ci = kvdata->add_child("controlinfo");
                ci->add_child_text(content.controlinfo.flagstring());
                Element * ui = kvdata->add_child("useinfo");
                ui->add_child_text(content.useinfo.flagstring());
                Element * cfailed = kvdata->add_child("cfailed");
                cfailed->add_child_text(content.cfailed);
              }
            }

            // kvTextData:
            for (Container<TextDataItem>::const_iterator rest = tbt->textData
                .begin(); rest != tbt->textData.end(); ++rest) {
              Element * kvtextdata = tt->add_child("kvtextdata");
              kvtextdata->set_attribute("paramid",
                                        lexical_cast<string>(rest->paramID()));
              Element * original = kvtextdata->add_child("original");
              original->add_child_text(rest->content().original);
            }
          }
        }
      }
    }
  }

  KvalobsData::RejectList fixedRejected;
  data_.getRejectedCorrections(fixedRejected);
  if (not fixedRejected.empty()) {
    std::map<std::string, KvalobsData::RejectList> decoderSortedRejectList;
    for (KvalobsData::RejectList::const_iterator it = fixedRejected.begin();
        it != fixedRejected.end(); ++it)
      decoderSortedRejectList[it->decoder()].push_back(*it);

    Element * reject = root->add_child("fixed_rejected_messages");

    for (std::map<std::string, KvalobsData::RejectList>::const_iterator decoder =
        decoderSortedRejectList.begin();
        decoder != decoderSortedRejectList.end(); ++decoder) {
      Element * decoderElement = reject->add_child("decoder");
      decoderElement->set_attribute("val", decoder->first);
      for (KvalobsData::RejectList::const_iterator it = decoder->second.begin();
          it != decoder->second.end(); ++it) {

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

