/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvMetarDecoder.cc,v 1.1.6.2 2007/09/27 09:02:37 paule Exp $                                                       

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
#include "kvMetarDecoder.h"


/* Created by DNMI/PU: j.schulze@met.no
   at Wed Apr  2 08:34:13 2003 */

using namespace std; 
using namespace kvalobs;
using namespace miutil;



bool kvMetarDecoder::initialise(const std::list<kvalobs::kvStation>& kpos, const miutil::miString& wwfile) 
{
  autoReferenceTime();
  
  obs.wwTabInit(wwfile);

  miString icao;
  int      stat;
  
  list<kvStation>::const_iterator itr=kpos.begin();

  for(;itr!=kpos.end();itr++) {
    stat = itr->stationID();
    icao = itr->ICAOID();
    if(icao.exists())
      icaoRegister[icao]  = stat;
  }
  return !icaoRegister.empty();
}


void kvMetarDecoder::setReferenceTime(const miTime& r)
{
  ref        = r;
  continuous = false;
}

void kvMetarDecoder::autoReferenceTime()
{
  ref        = miTime::nowTime();
  continuous = true;
}
 
miTime kvMetarDecoder::createObsTime()
{
  if(continuous)
    ref = miTime::nowTime();

  return  obs.createObsTime(ref);
}

int  kvMetarDecoder::findStationid()
{
  miString icao = obs.ICAOID();

  if(icao.exists())
    if(icaoRegister.count(icao))
      return icaoRegister[icao];
  return 0;
}



bool kvMetarDecoder::decode(const miutil::miString raw)
{     
  tbt = miTime::nowTime();

  if(!obs.decode(raw) ) {
    rejectComment = obs.Unrecognised();
    return false;
  }
  
  if(!findStationid()) {
    rejectComment = "unknown position";
    return false;
  }

  return true;

}

list<kvData> kvMetarDecoder::Data()
{  
  list<kvData> data;
  list<kmet::obsbuf>  deco         = obs.Data();
  list<kmet::obsbuf>::iterator itr = deco.begin();
 
  kvData d;

  miTime obt = createObsTime();
  int    pos = findStationid();

  for(;itr!=deco.end();itr++) {
    if(itr->par) {
      d.set(pos, obt, itr->val, itr->par, tbt,kmet::kvalobsType, itr->lvl);
      data.push_back(d);
    }
  }
  return data;
}

list<kvTextData> kvMetarDecoder::Text()
{  
  list<kvTextData>             data;        
  list<kmet::txtbuf>           txt = obs.Text();
  list<kmet::txtbuf>::iterator itr = txt.begin();
 
  kvTextData t;

  miTime obt = createObsTime();
  int    pos = findStationid();

  for(;itr!=txt.end();itr++) {
    if(itr->par) {
      t.set(pos, obt, itr->val, itr->par, tbt,kmet::kvalobsType);
      data.push_back(t);
    }
  }
  return data;
}


kvRejectdecode kvMetarDecoder::rejected()
{
  miString message   = obs.Raw();
  miTime   timestamp = miTime::nowTime();
  return kvRejectdecode(message,timestamp,"metar",rejectComment);
}



