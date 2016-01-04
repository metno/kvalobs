/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: smsbase.cc,v 1.4.2.5 2007/09/27 09:02:24 paule Exp $                                                       

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

#include <stdio.h>
#include <milog/milog.h>
#include "comobsdecode.h"
#include "smsbase.h"

using namespace miutil;
using namespace std;
using namespace kvalobs;

kvalobs::decoder::comobsdecoder::SmsBase::SmsBase(
    const ParamList &paramList_,
    kvalobs::decoder::comobsdecoder::ComObsDecoder &decoder)
    : paramList(paramList_),
      inData(0),
      inDataparam(0),
      ComObsDec(decoder) {
}

kvalobs::decoder::comobsdecoder::SmsBase::~SmsBase() {
}

void kvalobs::decoder::comobsdecoder::SmsBase::initInData(
    const miutil::CommaString *indata, const char **inParams) {
  inData = indata;
  inDataparam = inParams;
}

bool kvalobs::decoder::comobsdecoder::SmsBase::getInData(const char *param,
                                                         std::string &val) {
  val.erase();

  for (int i = 0; inDataparam[i]; i++) {
    if (strcmp(inDataparam[i], param) == 0) {
      if (inData->get(i, val)) {
        return true;
      } else {
        return false;
      }
    }
  }

  return false;
}

kvalobs::decodeutil::DecodedData*
kvalobs::decoder::comobsdecoder::SmsBase::decode(
    long stationid, int smscode, const SmsMelding::MeldingList &obs,
    std::string &returnMessage, std::list<kvalobs::kvRejectdecode> &rejected,
    bool &hasRejected) {
  LOGFATAL("----- NOT IMPLEMENTED -----" << endl);
  return 0;
}

std::string kvalobs::decoder::comobsdecoder::SmsBase::createDHM(
    const std::string &dhm) {
  int year, mnt, d, h, m;
  char buf[100];

  miDate dateNow(miDate::today());

  year = dateNow.year();
  mnt = dateNow.month();

  LOGDEBUG6("SmsBase::createDHM: <" << dhm << ">!" << endl);

  if (sscanf(dhm.c_str(), "%2d%2d%2d", &d, &h, &m) != 3)
    return std::string();  //Null date

  if (d < 1 || d > 31 || h < 0 || h > 23 || m < 0 || m > 60)
    return std::string();  //Null date

  if (d > dateNow.day()) {
    //dag i m�neden er etter i dag. Er datoen for
    //forrige m�ned.
    mnt--;

    if (mnt <= 0) {
      mnt = 12;
      year--;
    }
  }

  sprintf(buf, "%02d%02d%02d%02d", mnt, d, h, m);

  return std::string(buf);

}

std::string kvalobs::decoder::comobsdecoder::SmsBase::createFromDHM(
    const std::string &dhm) {
  int year, mnt, d, h, m;
  char buf[100];

  miDate dateNow(miDate::today());

  year = dateNow.year();
  mnt = dateNow.month();

  LOGDEBUG6("SmsBase::createFromDHM: <" << dhm << ">!" << endl);

  if (sscanf(dhm.c_str(), "%2d%2d%2d", &d, &h, &m) != 3)
    return std::string();  //Null date

  if (d < 1 || d > 31 || h < 0 || h > 23 || m < 0 || m > 60)
    return std::string();  //Null date

  if (d > dateNow.day()) {
    //dag i m�neden er etter i dag. Er datoen for
    //forrige m�ned.
    mnt--;

    if (mnt <= 0) {
      mnt = 12;
      year--;
    }
  }

  sprintf(buf, "%04d%02d%02d%02d%02d00", year, mnt, d, h, m);

  return std::string(buf);
}

miutil::miTime kvalobs::decoder::comobsdecoder::SmsBase::createDTFromDHM(
    const std::string &dhm, const miutil::miTime &dt) {
  int year, mnt, d, h, m;
  char buf[100];

  miDate dateNow;

  if (dt.undef()) {
    dateNow = miDate(miDate::today());
  } else {
    dateNow = dt.date();
  }

  year = dateNow.year();
  mnt = dateNow.month();

  LOGDEBUG6("SmsBase::createDTFromDHM: <" << dhm << ">!" << endl);

  if (sscanf(dhm.c_str(), "%2d%2d%2d", &d, &h, &m) != 3)
    return miTime();

  if (d < 1 || d > 31 || h < 0 || h > 23 || m < 0 || m > 60)
    return miTime();

  if (d > dateNow.day()) {
    //Hvis dag i m�neden er etter i dag. Er datoen for
    //forrige m�ned.
    mnt--;

    if (mnt <= 0) {
      mnt = 12;
      year--;
    }
  }

  return miTime(year, mnt, d, h);
}

std::string kvalobs::decoder::comobsdecoder::SmsBase::createFromMDh(
    const std::string &MDh) {

  int year, mnt, d, h, m;
  char buf[100];

  miDate dateNow(miDate::today());

  year = dateNow.year();

  LOGDEBUG6("SmsBase::createFromMDh: <" << MDh << ">!");

  if (sscanf(MDh.c_str(), "%2d%2d%2d", &mnt, &d, &h) != 3)
    return std::string();  //Null date

  if (mnt > 12 || mnt < 1 || d < 1 || d > 31 || h < 0 || h > 23)
    return std::string();  //Null date

  if (mnt > dateNow.month()) {
    year--;
  }

  sprintf(buf, "%04d%02d%02d%02d0000", year, mnt, d, h);

  return std::string(buf);
}

miutil::miTime kvalobs::decoder::comobsdecoder::SmsBase::createDTFromDH(
    const std::string &DH) {
  int year, mnt, d, h, m;
  char buf[100];
  miTime dateNow(miTime::nowTime());

  if (DH.length() != 4)
    return miTime();  //Null DateTime

  //DEBUG
  //  dateNow=dnmi::DateTime(2004, 02, 29, 0, 01);
  //std::cerr << "DateTime now: " << dateNow << std::endl;

  year = dateNow.year();
  mnt = dateNow.month();

  if (DH.length() != 4)
    return miTime();  //Null DateTime

  if (sscanf(DH.c_str(), "%2d%2d", &d, &h) != 2)
    return miTime();  //Null DateTime

  if (d < 1 || d > 31 || h < 0 || h > 23)
    return miTime();  //Null date

  //Hvis vi er i slutten av m�neden, og d=1, f�rste dag i neste m�ned
  //setter vi obstiden til neste dag.
  if (dateNow.day() == dateNow.date().daysInMonth() && d == 1) {
    dateNow.addDay(1);
    return miTime(dateNow.date(), miClock(h, 0, 0));
  }

  miTime retDT(year, mnt, d, h);

  //DEBUG

  if (retDT.undef()) {
    LOGDEBUG("retDT: undefined!" << endl);
  } else {
    LOGDEBUG(
        "now:     " << dateNow << endl << "retDt:   " << retDT <<endl << "hoursTo: " << miTime::hourDiff(retDT, dateNow) << endl);
  }

  if (retDT.undef() || miTime::hourDiff(retDT, dateNow) > 24) {
    dateNow.addDay(-1 * dateNow.day());
    retDT = miTime(dateNow.year(), dateNow.month(), d, h);
  }

  LOGDEBUG("retDT: " << retDT << endl);

  return retDT;
}

miutil::miTime kvalobs::decoder::comobsdecoder::SmsBase::createDTFromYMDhm(
    const std::string &YDMhm) {
  int year, mnt, d, h, m;
  char buf[100];

  std::cerr << "SmsBase::createDTFromYDMhm: IN <" << YDMhm << ">\n";

  if (YDMhm.length() == 10) {
    if (sscanf(YDMhm.c_str(), "%2d%2d%2d%2d%2d", &year, &mnt, &d, &h, &m) != 5)
      return miTime();  //Null date

    year += 2000;
  } else if (YDMhm.length() == 12) {
    if (sscanf(YDMhm.c_str(), "%4d%2d%2d%2d%2d", &year, &mnt, &d, &h, &m) != 5)
      return miTime();  //Null date
  } else {
    return miTime();
  }

  if (d < 1 || d > 31 || h < 0 || h > 23 || mnt < 1 || mnt > 12 || m < 0
      || m > 61)
    return miTime();  //Null date

  miTime retDT(year, mnt, d, h, m);

  LOGDEBUG6("SmsBase::createDTFromYMDhm: OUT: " << retDT << endl);

  return retDT;
}

miutil::miTime kvalobs::decoder::comobsdecoder::SmsBase::createDTFromYMDh(
    const std::string &YYMMDDhh) {
  int year, mnt, d, h;
  char buf[100];

  LOGDEBUG6("SmsBase::createDTFromYDMh: IN <" << YYMMDDhh << ">" << endl);

  if (YYMMDDhh.length() != 8)
    return miTime();  //Null DateTime

  miDate dateNow(miDate::today());

  if (sscanf(YYMMDDhh.c_str(), "%2d%2d%2d%2d", &year, &mnt, &d, &h) != 4)
    return miTime();  //Null DateTime

  if (d < 1 || d > 31 || h < 0 || h > 23 || mnt < 1 || mnt > 12)
    return miTime();  //Null date

  year += 2000;

  miTime retDT(year, mnt, d, h);

  LOGDEBUG6("SmsBase::createDTFromYMDh: OUT: " << retDT << endl);

  return retDT;
}

std::string kvalobs::decoder::comobsdecoder::SmsBase::createFromYMDh(
    const std::string &YYMMDDhh) {
  miTime dt;
  char buf[30];

  dt = createDTFromYMDh(YYMMDDhh);

  if (dt.undef())
    return "";

  sprintf(buf, "%04d%02d%02d%02d", dt.year(), dt.month(), dt.day(), dt.hour());

  return buf;
}

void kvalobs::decoder::comobsdecoder::SmsBase::stripNewLine(std::string &buf) {
  std::string::size_type pos;

  pos = buf.find('\r');

  while (pos != std::string::npos) {
    buf.erase(pos, 1);
    pos = buf.find('\r');
  }

  pos = buf.find('\n');

  while (pos != std::string::npos) {
    buf.erase(pos, 1);
    pos = buf.find('\n');
  }

}

void kvalobs::decoder::comobsdecoder::SmsBase::cleanString(
    std::string &str, const std::string &validChars) {
  std::string::size_type pos;

  pos = str.find_first_not_of(validChars);

  while (pos != std::string::npos) {
    str.erase(pos, 1);
    pos = str.find_first_not_of(validChars);
  }
}

char kvalobs::decoder::comobsdecoder::SmsBase::separator(
    const std::string &buf, const std::string &validChars) {
  std::string::size_type pos;

  pos = buf.find_first_not_of(validChars);

  if (pos != std::string::npos) {
    return buf[pos];
  }

  return -1;
}

/**
 *
 * Det er sommertid fra siste s�ndag i mars klokka to (klokka 1 UTC/GMT) til 
 * siste s�ndag i oktober klokka to (klokka 1 UTC/GMT). I Norge stiller vi 
 * derfor fram fra klokken 2 til 3 om v�ren, og tilbake fra 3 til 2 om h�sten.
 *
 */

miutil::miTime kvalobs::decoder::comobsdecoder::SmsBase::localTimeToUTC(
    const miutil::miTime &localTime_) {
  using namespace dnmi;
  miTime localTime(localTime_);
  miTime summerTime(localTime.year(), 3, 31, 2);
  miTime winterTime(localTime.year(), 10, 31, 3);

  if (summerTime.dayOfWeek() != 0)
    summerTime.addDay(-1 * summerTime.date().dayOfWeek());

  if (winterTime.dayOfWeek() != 0)
    winterTime.addDay(-1 * winterTime.date().dayOfWeek());

  //cerr << "Summertime: " << summerTime << endl;
  //cerr << "Wintertime: " << winterTime << endl;

  if (localTime >= summerTime && localTime < winterTime) {
    //"Day light saving time (dst)" er i aksjon.
    //cerr << "Sommertid korreksjon: -2 timer!\n";
    localTime.addSec(-2 * 3600);
    return localTime;
  } else {
    //cerr << "Vintertid korreksjon: -1 time!\n";
    localTime.addSec(-1 * 3600);
    return localTime;
  }
}

