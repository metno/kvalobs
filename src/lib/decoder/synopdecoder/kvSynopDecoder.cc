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
#include "kvSynopDecoder.h"
#include "synopLexer.h"

#include <map>

/** Created by DNMI/PU: j.schulze@met.no
    at Thu Jan 23 13:48:46 2003 */

using namespace std; 
using namespace kvalobs;
using namespace miutil;



bool kvSynopDecoder::initialise(const list<kvStation>& kpos, int earlyobs, int lateobs)
{
  autoReferenceTime();
  early = earlyobs * (-1);
  late  = lateobs;

  synopRegister.clear();
  shipRegister.clear();
  tempoRegister.clear();

  map<int,miTime> fromtimes;

  miTime now = miTime::nowTime();
  miTime from;

  miString call;
  int      syno;
  int      stat;
  
  list<kvStation>::const_iterator itr=kpos.begin();

  for(;itr!=kpos.end();itr++) {
    stat = itr->stationID();
    call = itr->call_sign();
    syno = itr->wmonr();
    from = itr->fromtime();


    if(from > now ) // station is not active yet!
      continue;
    
    if(fromtimes.count(syno))
      if(fromtimes[syno] > from ) // there is allready a newer
	continue;                 // record for this station ...

    if(syno > 0 )
      fromtimes[syno] = from;
    
    if(!stat)
      continue;
       
    if(syno)
      synopRegister[syno] = stat;
    
    if(call.exists())
      shipRegister[call]  = stat;
  
  }

  lastTempoIndex  
    = now.month() * 10000000  
    + now.day()   * 100000
    + now.hour()  * 1000
    + now.min()   * 10;
  
  return !synopRegister.empty();
}


void kvSynopDecoder::setReferenceTime(const miTime& r)
{
  ref        = r;
  continuous = false;
}

void kvSynopDecoder::autoReferenceTime()
{
  ref        = miTime::nowTime();
  continuous = true;
}
 
miTime kvSynopDecoder::createObsTime()
{
 
  if(continuous) 
    ref = miTime::nowTime();
 
  int mon = ref.month();
  int yea = ref.year();

  int YY   = obs.Info()->YY;
  int GG   = obs.Info()->GG;
  int GGgg = obs.Info()->GGgg;
  int gg   = 0;

//   if( GGgg) {  
//     GG = GGgg / 100;
//     gg = GGgg % 100;
//   }

  miTime obsT(yea,mon,YY,GG,gg,0); 
  miTime refT=ref;
  refT.addDay(10);

  if (obsT > refT ) {
    mon--;
    if(mon < 1) {
      yea--;
      mon=12;
    }
    obsT.setTime(yea,mon,YY,GG,gg,0);
  }

  return  obsT;
}

int  kvSynopDecoder::findStationid()
{
  int      land = obs.Info()->IIiii;
  int      boat = obs.Info()->A1BwNb;
  miString call = obs.Info()->callSign;


  needTmp = false;

  if(land) {
    if(synopRegister.count(land))
      return synopRegister[land];
    return 0;
  }

  if(call.exists()) {
    if(call != "SHIP" && call != "SYNOP" ) {
      if(shipRegister.count(call))
	return shipRegister[call];
      return temporaryRegister(call);
    }
  }
 
  if(boat) {
    return temporaryRegister(boat);
  }

  return 0;
}

int kvSynopDecoder::temporaryRegister(int uknown)
{
  miString uk = miString(uknown);
  return temporaryRegister(uk);
}


int kvSynopDecoder::temporaryRegister(miString uknown)
{
  if ( tempoRegister.count(uknown))
    return tempoRegister[uknown];

  lastTempoIndex++;
  tempoRegister[uknown] = lastTempoIndex;
 
  // build a new station with an index and the callsign starting from now
  //  tstat.set(lastTempoIndex,0,0,0,0,"",0,0,"",uknown,"",6,false, miutil::miTime::nowTime());

  tstat.set(lastTempoIndex, kvDbBase::FLT_NULL, kvDbBase::FLT_NULL, kvDbBase::FLT_NULL, kvDbBase::FLT_NULL,
	    "", kvDbBase::INT_NULL, kvDbBase::INT_NULL, "", uknown, "", 6, false, miutil::miTime::nowTime());

  needTmp=true;

  return lastTempoIndex;
}


char kvSynopDecoder::checkObservationTime(miutil::miTime tbt, miutil::miTime obt)
{
  if(!continuous)
    return 0;
 
  int diff = miTime::minDiff(tbt,obt);
  
  if ( diff < early )
    return 3;
  
  if ( diff > late )
    return 4;

  return 0;
}


bool kvSynopDecoder::decode(const std::string &raw, list<kvData>&   data)
{     
  obs =  lexSynop(raw);
  data.clear();

  if(obs.hasUnrecognised()) {
    rejectComment = obs.Unrecognised();
    return false;
  }

  map<int,float>  deco         = obs.Data();
  map<int,float>::iterator itr = deco.begin();
 
  kvData d;

  miTime tbt = miTime::nowTime();
  miTime obt = createObsTime();
  int    pos = findStationid();
  int typeID = obs.typeID();

  if(!pos) {
    rejectComment = "unknown station/position";
    return false;
  }
  else {
    for(;itr!=deco.end();itr++) {
      if(itr->first) {
	if(itr->second > -5000 )  {// filter any kind of undef 

	  /// itr->first  = parameterindex ( int   )
 	  /// itr->second = value          ( float )


	  d.set(pos, obt, itr->second, itr->first, tbt, typeID,0);
	  /// set useifo (7) not for ship!!!
	  d.useinfo(7, ( typeID == 11 ? 0 : checkObservationTime(tbt,obt)) );
	  data.push_back(d);
	}
      }
    }
  }
  return true;
}


bool kvSynopDecoder::tmpStation(kvalobs::kvStation& kvs)
{
  if(needTmp)
    kvs = tstat;
  return needTmp;
}




kvRejectdecode kvSynopDecoder::rejected(const miutil::miString &decoder)
{
  miString message   = obs.Raw();
  miTime   timestamp = miTime::nowTime();
  return kvRejectdecode(message,timestamp,decoder,rejectComment);
}


