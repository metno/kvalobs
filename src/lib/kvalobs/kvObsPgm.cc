/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvObsPgm.cc,v 1.7.6.5 2007/09/27 09:02:30 paule Exp $                                                       

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
#include <kvalobs/kvObsPgm.h> 

using namespace std;
using namespace miutil;


miString kvalobs::kvObsPgm::toSend() const
{
   ostringstream ost;
   ost << "("
       << stationid_              << ","
       << paramid_                << ","
       << level_                  << ","
       << nr_sensor_              << ","
       << typeid_                 << ","
       << quoted(collector_)      << ","
       << quoted(kl00_)           << ","
       << quoted(kl01_)           << ","
       << quoted(kl02_)           << ","
       << quoted(kl03_)           << ","
       << quoted(kl04_)           << ","
       << quoted(kl05_)           << ","
       << quoted(kl06_)           << ","
       << quoted(kl07_)           << ","
       << quoted(kl08_)           << ","
       << quoted(kl09_)           << ","
       << quoted(kl10_)           << ","
       << quoted(kl11_)           << ","
       << quoted(kl12_)           << ","
       << quoted(kl13_)           << ","
       << quoted(kl14_)           << ","
       << quoted(kl15_)           << ","
       << quoted(kl16_)           << ","
       << quoted(kl17_)           << ","
       << quoted(kl18_)           << ","
       << quoted(kl19_)           << ","
       << quoted(kl20_)           << ","
       << quoted(kl21_)           << ","
       << quoted(kl22_)           << ","
       << quoted(kl23_)           << ","
       << quoted(mon_)            << ","
       << quoted(tue_)            << ","
       << quoted(wed_)            << ","
       << quoted(thu_)            << ","
       << quoted(fri_)            << ","
       << quoted(sat_)            << ","
       << quoted(sun_)            << ","
       << quoted(fromtime_)       << ","
       << quoted(totime_)         << ")";
   return ost.str();
}


bool kvalobs::kvObsPgm::set( int stationid,
                             int paramid,
                             int level,
                             int nr_sensor,
                             int typ,
                             bool collector,
                             bool kl00,
                             bool kl01,
                             bool kl02,
                             bool kl03,
                             bool kl04,
                             bool kl05,
                             bool kl06,
                             bool kl07,
                             bool kl08,
                             bool kl09,
                             bool kl10,
                             bool kl11,
                             bool kl12,
                             bool kl13,
                             bool kl14,
                             bool kl15,
                             bool kl16,
                             bool kl17,
                             bool kl18,
                             bool kl19,
                             bool kl20,
                             bool kl21,
                             bool kl22,
                             bool kl23,
                             bool mon,
                             bool tue,
                             bool wed,
                             bool thu,
                             bool fri,
                             bool sat,
                             bool sun,
                             const miutil::miTime& fromtime,
                             const miutil::miTime& totime){
  stationid_ = stationid;
  paramid_   = paramid;
  level_     = level;
  nr_sensor_ = nr_sensor;
  typeid_    = typ;
  collector_ = collector;
  kl00_ = kl00;
  kl01_ = kl01;
  kl02_ = kl02;
  kl03_ = kl03;
  kl04_ = kl04;
  kl05_ = kl05;
  kl06_ = kl06;
  kl07_ = kl07;
  kl08_ = kl08;
  kl09_ = kl09;
  kl10_ = kl10;
  kl11_ = kl11;
  kl12_ = kl12;
  kl13_ = kl13;
  kl14_ = kl14;
  kl15_ = kl15;
  kl16_ = kl16;
  kl17_ = kl17;
  kl18_ = kl18;
  kl19_ = kl19;
  kl20_ = kl20;
  kl21_ = kl21;
  kl22_ = kl22;
  kl23_ = kl23;
  mon_ = mon;
  tue_ = tue;
  wed_ = wed;
  thu_ = thu;
  fri_ = fri;
  sat_ = sat;
  sun_ = sun;
  fromtime_ = fromtime;
  totime_ = totime;
  sortBy_= miString(stationid_);
  return true;
}


bool kvalobs::kvObsPgm::set(const dnmi::db::DRow& r_)
{
  dnmi::db::DRow &r=const_cast<dnmi::db::DRow&>(r_);
  string       buf;
  list<string> names= r.getFieldNames();
  list<string>::iterator it=names.begin();


  for(;it!=names.end(); it++){
    try{
      buf=r[*it];
      if(*it=="stationid"){
        stationid_=atoi(buf.c_str());
      }else if(*it=="paramid"){
        paramid_=atoi(buf.c_str());
      }else if(*it=="level"){
        level_=atoi(buf.c_str());
      }else if(*it=="nr_sensor"){
        nr_sensor_=atoi(buf.c_str());
      }else if(*it=="typeid"){
        typeid_=atoi(buf.c_str());
      }else if(*it=="collector"){
        collector_= (buf=="t") ? true : false;
      }else if(*it=="kl00"){
        kl00_= (buf=="t") ? true : false;
      }else if(*it=="kl01"){
        kl01_= (buf=="t") ? true : false;
      }else if(*it=="kl02"){
        kl02_= (buf=="t") ? true : false;
      }else if(*it=="kl03"){
        kl03_= (buf=="t") ? true : false;
      }else if(*it=="kl04"){
        kl04_= (buf=="t") ? true : false;
      }else if(*it=="kl05"){
        kl05_= (buf=="t") ? true : false;
      }else if(*it=="kl06"){
        kl06_= (buf=="t") ? true : false;
      }else if(*it=="kl07"){
        kl07_= (buf=="t") ? true : false;
      }else if(*it=="kl08"){
        kl08_= (buf=="t") ? true : false;
      }else if(*it=="kl09"){
        kl09_= (buf=="t") ? true : false;
      }else if(*it=="kl10"){
        kl10_= (buf=="t") ? true : false;
      }else if(*it=="kl11"){
        kl11_= (buf=="t") ? true : false;
      }else if(*it=="kl12"){
        kl12_= (buf=="t") ? true : false;
      }else if(*it=="kl13"){
        kl13_= (buf=="t") ? true : false;
      }else if(*it=="kl14"){
        kl14_= (buf=="t") ? true : false;
      }else if(*it=="kl15"){
        kl15_= (buf=="t") ? true : false;
      }else if(*it=="kl16"){
        kl16_= (buf=="t") ? true : false;
      }else if(*it=="kl17"){
        kl17_= (buf=="t") ? true : false;
      }else if(*it=="kl18"){
        kl18_= (buf=="t") ? true : false;
      }else if(*it=="kl19"){
        kl19_= (buf=="t") ? true : false;
      }else if(*it=="kl20"){
        kl20_= (buf=="t") ? true : false;
      }else if(*it=="kl21"){
        kl21_= (buf=="t") ? true : false;
      }else if(*it=="kl22"){
        kl22_= (buf=="t") ? true : false;
      }else if(*it=="kl23"){
        kl23_= (buf=="t") ? true : false;
      }else if(*it=="mon"){
        mon_= (buf=="t") ? true : false;
      }else if(*it=="tue"){
        tue_= (buf=="t") ? true : false;
      }else if(*it=="wed"){
        wed_= (buf=="t") ? true : false;
      }else if(*it=="thu"){
        thu_= (buf=="t") ? true : false;
      }else if(*it=="fri"){
        fri_= (buf=="t") ? true : false;
      }else if(*it=="sat"){
        sat_= (buf=="t") ? true : false;
      }else if(*it=="sun"){
        sun_= (buf=="t") ? true : false;
      }else if(*it=="fromtime"){
        fromtime_=miTime(buf);
      }else if(*it=="totime"){
         totime_=miTime(buf);
      }
    }
    catch(...){
      CERR("kvObsPgm: exception ..... \n");
    }
  }
  sortBy_= miString(stationid_);
  return true;
}


bool kvalobs::kvObsPgm::isOn(const miutil::miTime& t) const
{
  	int dayW= t.dayOfWeek();
  	int hour= t.hour();

	//Borge Moe
	//2005.11.22
	//Check minute.
	//The resolution of time is one hour and
	//obspgm assumes observations with obstimes  
	//hh:00:00.
  	if(t.min()!=0 || t.sec()!=0)
  		return false;

  	// first check weekday..
  	if      (dayW==0 && !sun_)
    	return false;
  	else if (dayW==1 && !mon_)
    	return false;
  	else if (dayW==2 && !tue_)
    	return false;
  	else if (dayW==3 && !wed_)
    	return false;
  	else if (dayW==4 && !thu_)
    	return false;
  	else if (dayW==5 && !fri_)
    	return false;
  	else if (dayW==6 && !sat_)
    	return false;

  	// then check hour..
  	if      (hour== 0 && !kl00_)
    	return false;
  	else if (hour== 1 && !kl01_)
    	return false;
  	else if (hour== 2 && !kl02_)
    	return false;
  	else if (hour== 3 && !kl03_)
    	return false;
  	else if (hour== 4 && !kl04_)
    	return false;
  	else if (hour== 5 && !kl05_)
    	return false;
  	else if (hour== 6 && !kl06_)
    	return false;
  	else if (hour== 7 && !kl07_)
    	return false;
  	else if (hour== 8 && !kl08_)
    	return false;
  	else if (hour== 9 && !kl09_)
    	return false;
  	else if (hour==10 && !kl10_)
    	return false;
  	else if (hour==11 && !kl11_)
    	return false;
  	else if (hour==12 && !kl12_)
    	return false;
  	else if (hour==13 && !kl13_)
    	return false;
  	else if (hour==14 && !kl14_)
    	return false;
  	else if (hour==15 && !kl15_)
    	return false;
  	else if (hour==16 && !kl16_)
    	return false;
  	else if (hour==17 && !kl17_)
    	return false;
  	else if (hour==18 && !kl18_)
    	return false;
  	else if (hour==19 && !kl19_)
    	return false;
  	else if (hour==20 && !kl20_)
    	return false;
  	else if (hour==21 && !kl21_)
    	return false;
  	else if (hour==22 && !kl22_)
    	return false;
  	else if (hour==23 && !kl23_)
    	return false;

  	return true;
}


miutil::miString 
kvalobs::kvObsPgm::uniqueKey()const
{
  ostringstream ost;
  
  ost << " WHERE stationid=" << stationid_                  << " AND "
      << "       typeid="    << typeid_                     << " AND " 
      << "       paramid="   << paramid_                    << " AND "
      << "       level="     << level_                      << " AND "
      << "       fromtime="  << quoted(fromtime_.isoTime()) << " AND " 
      << "       totime="    << quoted(totime_.isoTime());
 
  return ost.str();
}
