/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: kvSynopDecoder.cc,v 1.18.2.5 2007/09/27 09:02:18 paule Exp $                                                       

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
#include <boost/lexical_cast.hpp>
#include <puTools/miTime.h>
#include <miutil/timeconvert.h>
#include "kvSynopDecoder.h"
#include "synopLexer.h"

#include <map>

/** Created by DNMI/PU: j.schulze@met.no
 at Thu Jan 23 13:48:46 2003 */

using namespace std;
using namespace kvalobs;
using namespace miutil;

miutil::miTime kvSynopDecoder::firstDayNextMonth(const miTime &mi) {
  miTime ref(mi.year(), mi.month(), 28, 0, 0, 0);
  ref.addDay(5);  //This is in the next month.
  return miTime(ref.year(), ref.month(), 1, 0, 0, 0);
}

miutil::miTime kvSynopDecoder::lastDayThisMonth(const miutil::miTime &mi) {
  miTime firstDay = firstDayNextMonth(mi);
  firstDay.addDay(-1);  //Adjust it to last day in the previous month.
  return firstDay;
}

miutil::miTime kvSynopDecoder::createObsTime(int day, int hour,
                                             const miTime &refTime) {
  int mon = refTime.month();
  int yea = refTime.year();

  miTime obsT(yea, mon, day, hour, 0, 0);
  miTime refT = refTime;
  refT.addDay(10);

  miTime lastDay = lastDayThisMonth(refTime);

  if (refTime.day() == lastDay.day() && obsT.day() == 1
      && obsT.month() == refTime.month()) {
    miTime newRef(lastDay);
    newRef.addDay(1);  //First day next month
    obsT = miTime(newRef.year(), newRef.month(), day, hour, 0, 0);
  }

  if (obsT > refT) {
    mon--;
    if (mon < 1) {
      yea--;
      mon = 12;
    }
    obsT.setTime(yea, mon, day, hour, 0, 0);
  }

  return obsT;
}

bool kvSynopDecoder::initialise(const list<kvStation>& kpos, int earlyobs,
                                int lateobs) {
  autoReferenceTime();
  early = earlyobs * (-1);
  late = lateobs;

  synopRegister.clear();
  shipRegister.clear();
  tempoRegister.clear();

  map<int, miTime> fromtimes;

  miTime now = miTime::nowTime();
  miTime from;

  string call;
  int syno;
  int stat;

  list<kvStation>::const_iterator itr = kpos.begin();

  for (; itr != kpos.end(); itr++) {
    stat = itr->stationID();
    call = itr->call_sign();
    syno = itr->wmonr();
    from = to_miTime(itr->fromtime());

    if (from > now)  // station is not active yet!
      continue;

    if (fromtimes.count(syno))
      if (fromtimes[syno] > from)  // there is allready a newer
        continue;                 // record for this station ...

    if (syno > 0)
      fromtimes[syno] = from;

    if (!stat)
      continue;

    if (syno)
      synopRegister[syno] = stat;

    if (!call.empty())
      shipRegister[call] = stat;
  }

  lastTempoIndex = now.month() * 10000000 + now.day() * 100000
      + now.hour() * 1000 + now.min() * 10;

  return !(synopRegister.empty() && shipRegister.empty());
}

void kvSynopDecoder::setReferenceTime(const miTime& r) {
  ref = r;
  continuous = false;
}

void kvSynopDecoder::autoReferenceTime() {
  ref = miTime::nowTime();
  continuous = true;
}

miTime kvSynopDecoder::createObsTime() {

  if (continuous)
    ref = miTime::nowTime();

  return createObsTime(obs.Info()->YY, obs.Info()->GG, ref);
}

int kvSynopDecoder::findStationid() {
  int land = obs.Info()->IIiii;
  int boat = obs.Info()->A1BwNb;
  string call = obs.Info()->callSign;

  msgid.clear();
  needTmp = false;

  if (land) {
    ostringstream o;
    o << land;
    msgid = o.str();
    if (synopRegister.count(land))
      return synopRegister[land];
    return 0;
  }

  if (!call.empty()) {
    msgid = call;
    if (call != "SHIP" && call != "SYNOP") {
      if (shipRegister.count(call))
        return shipRegister[call];
      return temporaryRegister(call);
    }
  }

  if (boat) {
    ostringstream o;
    o << boat;
    msgid = o.str();
    return temporaryRegister(boat);
  }

  return 0;
}

int kvSynopDecoder::temporaryRegister(int uknown) {
  string uk = boost::lexical_cast<string>(uknown);
  return temporaryRegister(uk);
}

int kvSynopDecoder::temporaryRegister(std::string uknown) {
  if (tempoRegister.count(uknown))
    return tempoRegister[uknown];

  lastTempoIndex++;
  tempoRegister[uknown] = lastTempoIndex;

  // build a new station with an index and the callsign starting from now
  //  tstat.set(lastTempoIndex,0,0,0,0,"",0,0,"",uknown,"",6,false, miutil::miTime::nowTime());

  tstat.set(lastTempoIndex, kvDbBase::FLT_NULL, kvDbBase::FLT_NULL,
            kvDbBase::FLT_NULL, kvDbBase::FLT_NULL, "", kvDbBase::INT_NULL,
            kvDbBase::INT_NULL, "", uknown, "", 6, false,
            boost::posix_time::microsec_clock::universal_time());

  needTmp = true;

  return lastTempoIndex;
}

char kvSynopDecoder::checkObservationTime(miutil::miTime tbt,
                                          miutil::miTime obt) {
  if (!continuous)
    return 0;

  int diff = miTime::minDiff(tbt, obt);

  if (diff < early)
    return 3;

  if (diff > late)
    return 4;

  return 0;
}

/*
 * Fra Lars Andresen: Har jeg f�tt f�lgende spesifikasjon for � skille mellom HL=-3 og
 * HL=-32767: Hvis skymengde, N, eller sikt, VV, mangler ("/"), enten den ene
 * eller andre eller begge (selv om gruppe 7 er med), s� anses HL="/" som
 * manglende.
 *
 */

void kvSynopDecoder::correct_h_VV_N(std::list<kvalobs::kvData> &data) const {
  int h = INT_MAX;  //paramid 55
  int VV = INT_MAX;  //paramid 273
  int N = INT_MAX;  //paramid 15
  list<kvalobs::kvData>::iterator ith = data.end();

  for (list<kvalobs::kvData>::iterator it = data.begin(); it != data.end();
      ++it) {
    if (it->paramID() == 55) {
      ith = it;
      h = static_cast<int>(it->original());
    } else if (it->paramID() == 273)
      VV = static_cast<int>(it->original());
    else if (it->paramID() == 15)
      N = static_cast<int>(it->original());
  }

  if (h == -3 && (VV == INT_MAX || N == INT_MAX)) {
    if (ith != data.end())
      data.erase(ith);
  }

}

bool kvSynopDecoder::decode(const std::string &raw, list<kvData>& data) {
  obs = lexSynop(raw, hshsInMeter);
  data.clear();

  if (obs.getType() == syn::MOBIL) {
    //We do NOT support SYNOP MOBIL.
    rejectComment = "MOBIL";
    return false;
  }

  if (obs.hasUnrecognised()) {
    rejectComment = obs.Unrecognised();
    return false;
  }

  map<int, float> deco = obs.Data();
  map<int, float>::iterator itr = deco.begin();

  kvData d;

  boost::posix_time::ptime tbt =
      boost::posix_time::microsec_clock::universal_time();
  miTime obt = createObsTime();
  int pos = findStationid();
  int typeID = obs.typeID();

  if (!pos) {
    rejectComment = "unknown station/position";
    return false;
  } else {
    for (; itr != deco.end(); itr++) {
      if (itr->first) {
        if (itr->second > -5000) {  // filter any kind of undef

          /// itr->first  = parameterindex ( int   )
          /// itr->second = value          ( float )

          d.set(pos, to_ptime(obt), itr->second, itr->first, tbt, typeID, 0);
          /// set useifo (7) not for ship!!!
          d.useinfo(
              7,
              (typeID == 11 ? 0 : checkObservationTime(to_miTime(tbt), obt)));
          data.push_back(d);
        }
      }
    }

    correct_h_VV_N(data);

  }
  return true;
}

bool kvSynopDecoder::tmpStation(kvalobs::kvStation& kvs) {
  if (needTmp)
    kvs = tstat;
  return needTmp;
}

kvRejectdecode kvSynopDecoder::rejected(const std::string &decoder) {
  string message = obs.Raw();
  return kvRejectdecode(message,
                        boost::posix_time::microsec_clock::universal_time(),
                        decoder, rejectComment);
}

