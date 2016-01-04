/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: smsbase.h,v 1.4.2.2 2007/09/27 09:02:24 paule Exp $                                                       

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

#ifndef __kvalobs_decoder_comobsdecoder_smsbase_h__
#define __kvalobs_decoder_comobsdecoder_smsbase_h__

#include <list>
#include <kvalobs/kvData.h>
#include <kvalobs/kvRejectdecode.h>
#include <kvalobs/paramlist.h>
#include <miutil/commastring.h>
#include "decodeddata.h"
#include "smsmelding.h"
#include "comobsdecode.h"

namespace kvalobs {
namespace decoder {
namespace comobsdecoder {

/**
 * \addtogroup comobsdecode
 *
 * @{
 */

class ComObsDecoder;

class SmsBase {
  SmsBase();
  SmsBase(const SmsBase &);
  SmsBase& operator=(const SmsBase &);

  miutil::miTime nowTime_;  //For testing only

 protected:
  const ParamList &paramList;
  const char **inDataparam;
  const miutil::CommaString *inData;
  kvalobs::decoder::comobsdecoder::ComObsDecoder &ComObsDec;

  void initInData(const miutil::CommaString *indata, const char **inParams);
  bool getInData(const char *param, std::string &val);

 public:
  std::string logid;
  SmsBase(const ParamList &paramList,
          kvalobs::decoder::comobsdecoder::ComObsDecoder &decoder);
  virtual ~SmsBase();

  /**
   * \brief setNowTime is used for testing only.
   */
  void setNowTime(const miutil::miTime &t) {
    nowTime_ = t;
  }

  /**
   * \brief Return the wall clock if nowTime_ is undef.
   *
   * If nowTime is not undef, return nowTime_.clock().
   *
   * This is for testing.
   */
  miutil::miClock nowClock() const {
    return nowTime_.undef() ? miutil::miClock::oclock() : nowTime_.clock();
  }

  /**
   * \brief Return the today if nowTime_ is undef.
   *
   * If nowTime is not undef, return nowTime_.date().
   *
   * This is for testing.
   */
  miutil::miDate nowDate() const {
    return nowTime_.undef() ? miutil::miDate::today() : nowTime_.date();
  }

  /**
   * \brief Return the nowTime if nowTime_ is undef.
   *
   * If nowTime is not undef, return nowTime_.
   *
   * This is for testing.
   */
  miutil::miTime nowTime() const {
    return nowTime_.undef() ? miutil::miTime::nowTime() : nowTime_;
  }

  /**
   * createDHM, lager en dato string på formatet MMDDhhmm, fra
   * formatet DDhhmm. Vi antar at datoen ikke gjelder for mer en
   * 1 måned tilbake i tid.
   */
  std::string createDHM(const std::string &dhm);

  /**
   * createFromDhm, lager en dato string på formatet YYYYMMDDhhmmss, fra
   * formatet DDhhmm. Vi antar at datoen ikke gjelder for mer en
   * 1 måned tilbake i tid.
   */

  std::string createFromDHM(const std::string &dhm);

  /**
   * createFromMDh, lager en dato string på formatet YYYYMMDDhhmmss, fra
   * formatet MMDDhh. Vi antar at datoen ikke gjelder for mer en
   * 12 måneder tilbake i tid.
   */
  std::string createFromMDh(const std::string &MDh);

  /**
   * \brief create a timestamp from a DDhhmm specification.
   * The timestamp is created relativ to the \a dt parameter.
   * If the \a dt parameter is left out our is undef, the current
   * time is used.
   *
   * \param dhm a string representing a time specification on the
   *        form DDhhmm, where DD is day in month, hh is hour in
   *        day and mm is minute in hour.
   * \param dt, create the timestamp relaive to this time.
   * \return a timestamp on success and undef on failure.
   */
  miutil::miTime
  createDTFromDHM(const std::string &dhm, const miutil::miTime &dt =
                      miutil::miTime());

  miutil::miTime createDTFromDH(const std::string &DH);
  miutil::miTime createDTFromYMDhm(const std::string &YDMhm);
  miutil::miTime createDTFromYMDh(const std::string &YYMMDDhh);
  std::string createFromYMDh(const std::string &YYMMDDhh);
  void stripNewLine(std::string &buf);
  void cleanString(std::string &str, const std::string &validChars =
                       "1234567890-+.");
  /**
   * separator, søker gjennom buf og returnerer første tegn som ikke
   * er med i mengden av av validChars. Antar at strengen buf bruke dette
   * tegnet som skille tegn mellom elementene.
   */
  char separator(const std::string &buf, const std::string &validChars =
                     "0123456789");

  miutil::miTime localTimeToUTC(const miutil::miTime &localTime);

  virtual kvalobs::decodeutil::DecodedData*
  decode(long stationid, int smscode, const SmsMelding::MeldingList &obs,
         std::string &returnMessage,
         std::list<kvalobs::kvRejectdecode> &rejected, bool &hasRejected);

};

}
}
}

#endif
