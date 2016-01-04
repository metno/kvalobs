/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: sms2.cc,v 1.7.2.13 2007/09/27 09:02:24 paule Exp $                                                       

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

#include <limits.h>
#include <stdlib.h>
#include <milog/milog.h>
#include <puTools/miTime.h>
#include <miutil/timeconvert.h>
#include "sms2.h"

/**
 * SMSDecode2.
 * Denne klassen brukes for � dekode SMS meldinger fra nedb�rstasjoner.
 * SMS code er 2.
 */

/* 2003.08.20 
 * Bxrge Moe
 * Endret konverterings koden for v�rtegn. SMS v�rtegn 21 
 * konverteres til 'klimav�rtegn' 28. SMS v�rtegn 22 brukes ikke lenger.
 *
 * 2003.08.21
 * Bxrge Moe
 * Lagt til kode for � st�tte RR_24 i synop. 
 *
 * 2004.09.13
 * Fikset et konsistens problem.
 * Ved t�rt kode vi -1 og ved ikke m�lbar nedb�r kode vi 0.0.
 * Ved t�rt sendes en tom streng for sms kode 2 og ved ikke m�lbar
 * nedb�r sendes 0. Det er gjort endring b�de for kvalobs og AutoObs.
 *
 * 2004.12.14
 * Bxrge Moe
 * Lagt til bedre sjekk av gyldig observasjons tid. N� sjekkes datoen 
 * mot 'n�tid'. Vi tilater kun at �ret aviker fra dette �ret med to �r i
 * begge retininger. Alt av mnd, dag og time sjekkes mot gyldige verdier.
 *
 * 2006.01.17
 * Bxrge Moe
 * Oppdatert dekodingen av nedbørslag til ny spek.
 * 
 * 2006.03.20
 * Bxrge Moe
 * Fjernet kravet om at 'tildato' skal være i intervallet 3 til 9 UTC.
 * 
 * 2007.05.09
 * Bxrge Moe
 * Fjernet flagging av controllinfo(12)=2. Flagging av oppsamling gores na i
 * en kontroll sjekk.
 */

using namespace std;
using namespace miutil;
using namespace kvalobs::decodeutil;
namespace pt = boost::posix_time;

namespace {

const char *PARAM_IN[] = { "stnr", "code", "KLCOMOBS",  //Blir lagt til av ComObs, UTC tid.
    "RR", "P1", "P2", "P3", "SA", "SD", "KLSTART",  //Kommer fra stasjonen, lokaltid.
    "KLOBS",   //Kommer fra stasjonen, lokaltid.
    0 };
}

kvalobs::decoder::comobsdecoder::Sms2::Sms2(
    const ParamList &paramList,
    kvalobs::decoder::comobsdecoder::ComObsDecoder &dec)
    : SmsBase(paramList, dec) {
}

kvalobs::decoder::comobsdecoder::Sms2::~Sms2() {
}

std::string kvalobs::decoder::comobsdecoder::Sms2::createObsTime(
    const std::string &YYMMDDhhmm, miutil::miTime &obsTime) {
  int year, mnt, d, h, m;
  char buf[100];
  miTime now(nowTime());

  IDLOGDEBUG5(logid, "SMSDecode2::createObsTime: <" << YYMMDDhhmm << ">" <<endl);

  if (sscanf(YYMMDDhhmm.c_str(), "%2d%2d%2d%2d%2d", &year, &mnt, &d, &h, &m)
      != 5)
    return std::string();  //Null date

  if (mnt > 12 || mnt < 1 || d < 1 || d > 31 || h < 0 || h > 23 || m < 0
      || m > 59)
    return std::string();  //Null date

  if (year < 100)
    year += 2000;

  if (year < (now.date().year() - 1) || year > (now.date().year() + 1)
      || mnt < 1 || mnt > 12 || d < 1 || d > 31 || h < 0 || h > 23)
    return std::string();

  sprintf(buf, "%04d%02d%02d%02d%02d00", year, mnt, d, h, m);
  obsTime = miTime(year, mnt, d, h, m);

  return std::string(buf);
}

miutil::miTime kvalobs::decoder::comobsdecoder::Sms2::createObsTime(
    const std::string &YYYYMMDDhh, std::string &sYYYYMMDDhh) {
  int year, mnt, d, h, m;
  miTime dt;
  char buf[30];
  miTime now(nowTime());

  if (YYYYMMDDhh.length() == 8) {
    //asume YYMMDDhh
    if (sscanf(YYYYMMDDhh.c_str(), "%2d%2d%2d%2d", &year, &mnt, &d, &h) != 4)
      return miTime();  //Null date,

    year += 2000;

    if (year < (now.year() - 1) || year > (now.year() + 1) || mnt < 1
        || mnt > 12 || d < 1 || d > 31 || h < 0 || h > 23)
      return miTime();

    dt = miTime(year, mnt, d, h);
  } else if (YYYYMMDDhh.length() == 10) {
    if (sscanf(YYYYMMDDhh.c_str(), "%4d%2d%2d%2d", &year, &mnt, &d, &h) != 4)
      return miTime();  //Null date,

    if (year < (now.year() - 1) || year > (now.year() + 1) || mnt < 1
        || mnt > 12 || d < 1 || d > 31 || h < 0 || h > 23)
      return miTime();

    dt = miTime(year, mnt, d, h);
  } else {
    return miTime();
  }

  sprintf(buf, "%04d%02d%02d%02d", dt.year(), dt.month(), dt.day(), dt.hour());
  sYYYYMMDDhh = buf;

  return dt;
}

bool kvalobs::decoder::comobsdecoder::Sms2::onlyZeros(const std::string &val) {
  for (std::string::size_type i = 0; i < val.length(); i++) {
    if (val[i] != '0')
      return false;
  }

  return true;
}

int kvalobs::decoder::comobsdecoder::Sms2::decodeV1V2V3(const std::string &w_,
                                                        std::string &v1,
                                                        std::string &v2,
                                                        std::string &v3,
                                                        std::string &intensitet,
                                                        std::ostream &err) {
  int v;
  char ic;
  std::string vs;
  std::string w(w_);
  int k;

  //	LOGDEBUG("decodeV1V2V3: before cleanStrinh w [" << w << "]");
  cleanString(w, "0123456789");
  //	LOGDEBUG("decodeV1V2V3: after cleanStrinh w [" << w << "]");

  v1.erase();
  v2.erase();
  v3.erase();
  intensitet = "333";

  if (w.empty())
    return 0;

  if ((w.size() % 2) != 0) {
    err << "Ugyldig værtegn: antall siffer må være et partall!";
    IDLOGERROR(logid, "Ugyldig værtegn: antall siffer må være et partall!");
    return -1;
  }

  k = 0;
  while (!w.empty()) {
    vs = w.substr(0, 2);
    w.erase(0, 2);

    switch (vs[1]) {
      case '0':
        ic = '0';
        v = vs[0] - '0';
        break;
      case '1':
        ic = '1';
        v = vs[0] - '0';
        break;
      case '2':
        ic = '2';
        v = vs[0] - '0';
        break;
      default:
        ic = '1';
        v = atoi(vs.c_str());
    }

    if (v == 1)
      vs = "03";
    else if (v == 2)
      vs = "02";
    else if (v == 3)
      vs = "01";
    else if (v == 4)
      vs = "07";
    else if (v == 5)
      vs = "05";
    else if (v == 6)
      vs = "04";
    else if (v == 73)
      vs = "08";
    else if (v == 74)
      vs = "10";
    else if (v == 85)
      vs = "12";
    else if (v == 86)
      vs = "17";
    else if (v == 97)
      vs = "20";
    else if (v == 98)
      vs = "28";
    else {
      err << "ERROR: Værtegn: " << k + 1 << " ugyldig kode (" << vs << ")!\n";
      IDLOGERROR(logid,
                 "ERROR: Værtegn: " << k+1 << " ugyldig kode (" << vs << ")!");
      return -1;
    }

    intensitet[k] = ic;

    if (k == 0)
      v1 = vs;
    else if (k == 1)
      v2 = vs;
    else if (k == 2)
      v3 = vs;
    else {
      IDLOGERROR(logid,
                 "Sms2::decodeV1V2V3: Ugyldig værtegn. Mer enn 6 siffer.\n");
      err << "Sms2::decodeV1V2V3: Ugyldig værtegn. Mer enn 6 siffer.\n";
      return -1;
    }

    k++;
  }

  return k;
}
void kvalobs::decoder::comobsdecoder::Sms2::decodeAndAddV1V2V3(
    DecodedDataElem &data, const miutil::miTime &obstime,
    const std::string &what) {
  string v1, v2, v3, vi;
  string buf;
  stringstream sstrm;

  if (getInData(what.c_str(), buf)) {
    cleanString(buf);

    if (!buf.empty()) {
      sstrm.str("");

      if (decodeV1V2V3(buf, v1, v2, v3, vi, sstrm) > 0) {
        addV1V2V3(data, obstime, v1, v2, v3, vi);
      }

      buf = sstrm.str();

      if (!buf.empty()) {
        IDLOGDEBUG1(logid, "decode2kv: Værtegn: " + what + " : " + buf);
      }
    }
  }
}

void kvalobs::decoder::comobsdecoder::Sms2::addV1V2V3(
    DecodedDataElem &data, const miutil::miTime &obstime, const std::string &v1,
    const std::string &v2, const std::string &v3, const std::string &vi) {
  data.addDataObstime("V4", v1, obstime);
  data.addDataObstime("V5", v2, obstime);
  data.addDataObstime("V6", v3, obstime);

  if (!v1.empty())
    data.addDataObstime("V4S", vi.substr(0, 1), obstime);

  if (!v2.empty())
    data.addDataObstime("V5S", vi.substr(1, 1), obstime);

  if (!v3.empty())
    data.addDataObstime("V6S", vi.substr(2, 1), obstime);
}

kvalobs::decodeutil::DecodedData*
kvalobs::decoder::comobsdecoder::Sms2::decode(
    long stationid, int smscode, const SmsMelding::MeldingList &obs,
    std::string &returnMessage, std::list<kvalobs::kvRejectdecode> &rejected,
    bool &hasRejected) {
  miTime dtObs;
  string buf;
  ostringstream errs;
  char sepBuf[2];
  char sep;
  bool error;
  DecodedData *smsData;

  sepBuf[1] = '\0';

  hasRejected = false;

  try {
    smsData = new DecodedData(paramList, stationid, smscode + 300);
  } catch (...) {
    IDLOGFATAL(logid, "NOMEM: failed to allocate memory for SmsData!" << endl);
    return 0;
  }

  DecodedDataElem data = smsData->createDataElem();

  for (SmsMelding::CIMeldingList it = obs.begin(); it != obs.end(); it++) {
    data.clean();
    buf = *it;
    stripNewLine(buf);

    if (buf.length() < 3) {
      IDLOGERROR(logid, "Invalid format!");
      continue;
    }

    sep = separator(buf);

    if (sep < 0) {
      IDLOGERROR(logid, "Invalid dataformat. Unknown element separator.");
      continue;
    }

    sepBuf[0] = sep;
    IDLOGINFO(logid, "Using seperator <" + string(sepBuf) + ">.");
    CommaString cStr(buf, sep);

    initInData(&cStr, PARAM_IN);

    if (cStr.size() < 3) {
      IDLOGERROR(logid, "Invalid dataformat. Too few elements.");
      continue;
    }

    errs.str("");
    if (!doKvPrecipitation(data, cStr, errs)) {
      string mybuf;
      cStr.copy(mybuf);
      IDLOGERROR(logid, "Invalid dataformat cant do precipitation.");
      rejected.push_back(
          kvRejectdecode(mybuf,
                         boost::posix_time::microsec_clock::universal_time(),
                         "comobs/typeid=302", errs.str()));
      hasRejected = true;
      continue;
    }

    dtObs = data.getDate();

    if (dtObs.hour() != 6) {
      dtObs = miTime(dtObs.date(), miClock(6, 0, 0));
    }

    //P3 18-06. This is for the observation time.
    decodeAndAddV1V2V3(data, dtObs, "P3");

    dtObs.addDay(-1);
    dtObs = miTime(dtObs.date(), miClock(18, 0, 0));
    decodeAndAddV1V2V3(data, dtObs, "P2");

    //P2 06-12
    dtObs = miTime(dtObs.date(), miClock(12, 0, 0));
    decodeAndAddV1V2V3(data, dtObs, "P1");

    if (data.dataSize() > 0 || data.textDataSize() > 0) {
      smsData->add(data);
      IDLOGINFO(logid, "SmsMelding decoded." << endl);
    } else {
      IDLOGINFO(logid, "No data." << endl);
    }
  }

  return smsData;
}

bool kvalobs::decoder::comobsdecoder::Sms2::doKvPrecipitation(
    kvalobs::decodeutil::DecodedDataElem &data,
    const miutil::CommaString &cData, std::ostream &ost) {
  string klStart;
  string klObs;
  string klComObs;
  float RR;
  string buf;
  string SA;
  string SD;
  int iSD = INT_MAX;
  char sRR[128];
  miTime dtComObs;
  int dif;
  miTime t6;        //6 terminen
  kvUseInfo useInfo;

  getInData("SA", SA);
  getInData("SD", SD);
  getInData("KLCOMOBS", klComObs);
  getInData("RR", buf);
  getInData("KLSTART", klStart);
  getInData("KLOBS", klObs);

  cleanString(buf);
  cleanString(klStart, "0123456789");
  cleanString(klObs, "0123456789");
  cleanString(klComObs, "0123456789");

  //Check if buf only contains + or -. If so erase buf.
  if (!buf.empty() && (buf[0] == '+' || buf[0] == '-')) {
    string __s = buf.substr(0, 1);

    string::size_type i = buf.find_first_not_of(__s);

    if (i == string::npos) {
      buf.erase();
    }
  }

  if (buf.empty()) {
    strcpy(sRR, "-1");
  } else {
    RR = static_cast<float>(atoi(buf.c_str())) / 10;
    sprintf(sRR, "%0.1f", RR);
  }

  IDLOGDEBUG2(
      logid,
      "KLSTART: <" << klStart << ">" << endl << "KLOBS:   <" << klObs << ">" << endl);

  //if one of klStart or klObs is empty and the other
  //is not empty we have an invalid observation!
  if ((!klObs.empty() && klStart.empty())
      || (klObs.empty() && !klStart.empty())) {
    ost << "Missing a timestamp, <tildato> eller <fradato>!";
    return false;
  }

  if (!klComObs.empty()) {
    string ret;
    ret = createObsTime(klComObs, dtComObs);

    if (ret.empty()) {
      IDLOGWARN(logid, "Invalid KLCOMOBS" << endl);
      dtComObs = miTime();  //Set obsTime til undef
    } else {
      IDLOGDEBUG2(logid, "KLCOMOBS time: " << dtComObs << endl);
    }
  }

  if (dtComObs.undef()) {
    dtComObs = nowTime();
    IDLOGDEBUG2(logid, "CURRENT time: " << dtComObs << endl);
  }

  if (klObs.empty() && klStart.empty()) {
    t6 = dtComObs;
    t6.setTime(t6.date(), miClock(6, 0, 0));

    dif = miTime::minDiff(dtComObs, t6);
    //LOGDEBUG6("t6:       " << t6 << endl <<
    //      "dtComObs: " << dtComObs << endl <<
    //      "dif:      " << dif << " abs: " << abs(dif) << endl);
    //
    //if(abs(dif)>180){
    //  LOGDEBUG1("Out of timespec COMOBSTIME (� 3 hour): " << dtComObs);
    //  ost << "Out of timespec COMOBSTIME (� 3 hour): " << dtComObs;
    //  return false;
    //}

    if (abs(dif) > 180) {
      IDLOGDEBUG6(logid, "Set useflag(1)=1!" << endl);
      useInfo.set(1, 1);
    }

    useInfo.set(
        7,
        ComObsDec.getUseinfo7Code(302, to_ptime(dtComObs),
                                  pt::second_clock::universal_time(), logid));
    IDLOGDEBUG2(logid, "CORRECTED obstime: " << t6 << endl);

    data.setDate(t6);
    data.addData("RR_24", sRR, useInfo);
  } else {
    // klObs og klStart har data.
    miTime dtStart;
    miTime dtObs;
    char myBuf[128];

    //klStart og klObs er angitt i lokaltid. createObsTime konverterer
    //ikke til UTC, slik at strengrepresentasjonen som returneres i 'buf'
    //kan ikke brukes. Vi konverterer til UTC med localTime2UTC og lager en ny
    //strengrepresentasjon.

    dtStart = createObsTime(klStart, buf);
    dtStart = localTimeToUTC(dtStart);
    sprintf(myBuf, "%04d%02d%02d%02d", dtStart.year(), dtStart.month(),
            dtStart.day(), dtStart.hour());
    klStart = myBuf;

    dtObs = createObsTime(klObs, buf);
    dtObs = localTimeToUTC(dtObs);
    sprintf(myBuf, "%04d%02d%02d%02d", dtObs.year(), dtObs.month(), dtObs.day(),
            dtObs.hour());
    klObs = myBuf;

    if (dtStart.undef() || dtObs.undef()) {
      ost << "Either <fradato> or <tildato> is invalid!!";
      IDLOGWARN(logid, "Invalid KLSTART or KLOBS!" << endl);
      return false;
    }

    IDLOGDEBUG6(
        logid,
        "dtComObs: " << dtComObs << endl << "dtStart:  " << dtStart << endl << "dtObs:    " << dtObs << endl << "dif:      " << miTime::minDiff(dtComObs, dtObs));

//      if(miTime::minDiff(dtComObs, dtObs)<-360){
//         // cerr << "dtComObs: " << dtComObs << " dtObs: " << dtObs << " diff: " << miTime::minDiff(dtComObs, dtObs) << endl;
//         ost << "The observation is to far in the future (>+1 hour)!";
//         IDLOGERROR(logid,"The observation is to far in the future (>+1 hour)!");
//         return false;
//      }

    if (dtObs < dtStart) {
      ost << "Invalid observation interval <tildato> < 'fradato'!";
      IDLOGERROR(logid, "Invalid observation interval <tildato> < 'fradato'!");
      return false;
    }

    dif = miTime::minDiff(dtObs, dtStart);
    IDLOGDEBUG(
        logid,
        "dif: " << dif << "(" << dif/1440 << " days " << dif%1440 << " hour " << dif-(dif/1440)*1440 - dif%1440 << " min)");

    if (dif < 1260) {  //less than 21 hour
      ost << "Ivalid observation, to few hour (<21 hour)!";
      IDLOGERROR(logid, "Ivalid observation, to few hour (<21 hour)!");
      return false;
    }

    t6 = dtObs;
    t6.setTime(t6.date(), miClock(6, 0, 0));
    int tDif = abs(miTime::minDiff(dtObs, t6));

    //if(tDif>180){ //'tildato' not between ±3 6 UTC
    //	ost << "<tildato> not between ±3 (6 UTC)!";
    //	LOGWARN("<tildato> not between ±3 (6 UTC)!");
    //	return false;
    //}

    if (dif < 1620) {  //Less than 27 hour
      if (tDif > 60) {  //'tildato' not between �1 6 UTC
        if (dif >= 1260 && dif < 1380)  //[21-23> number of hour
          useInfo.set(1, 1);
        else if (dif >= 1380 && dif <= 1500)  //[23,25] number of hour
          useInfo.set(1, 4);
        else
          //<25,27] number of hour
          useInfo.set(1, 5);
      }
      data.setDate(t6);
      data.addData("RR_24", sRR, useInfo);
    } else {  // Number of hour is greater than 27 hour.
      if (abs(miTime::minDiff(dtObs, t6)) > 60) {  //'tildato' not between � 6 UTC
        useInfo.set(1, 5);
      }

      kvControlInfo controlInfo;

      //controlInfo.set(12, 2);

      data.setDate(t6);
      data.addData("KLSTART", klStart);
      data.addData("KLOBS", klObs);
      data.addData("RR_24", sRR, useInfo, controlInfo);
    }
  }

  bool hasSa = false;
  bool hasSd = false;
  string sSaSdEm = ComObsDec.getMetaSaSdEm(data.stationID(), data.typeID(),
                                           data.getDate());

  if (sSaSdEm[0] != '0')
    hasSa = true;

  if (sSaSdEm[1] != '0')
    hasSd = true;

  IDLOGDEBUG(
      logid,
      "sSaSdEm. " << sSaSdEm << " hasSa: " << (hasSa?"t":"f") << " hasSd: " << (hasSd?"t":"f"));
  cleanString(SA);
  cleanString(SD);

  if ((SD.empty() || SD == "0") && hasSd) {
    SD = "-1";
    iSD = -1;
  }

  if (!SD.empty() && SD != "-1") {
    int i;
    for (i = 0; i < SD.length() && isdigit(SD[i]); i++)
      ;

    if (i < SD.length()) {  //Invalid code
      SD = "-1";
      iSD = -1;
    } else {
      iSD = atoi(SD.c_str());
    }
  }

  if ((SA.empty() || SA == "0") && hasSa)
    SA = "-1";

//   if(iSD==1 || iSD==2){
//      SA="-1"; //Flekkvis sne
//   }

  if (!SA.empty())
    data.addData("SA", SA, useInfo);

  if (!SD.empty())
    data.addData("SD", SD, useInfo);

  return true;
}
