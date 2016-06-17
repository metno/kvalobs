/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: sms12.cc,v 1.2.2.3 2007/09/27 09:02:24 paule Exp $                                                       

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
#include <stdlib.h>
#include <sstream>
#include <milog/milog.h>
#include "decodeutil.h"
#include <miutil/trimstr.h>
#include <kvalobs/kvTextData.h>
#include "sms13.h"

/**
 * SMSDecode13.
 * Denne klassen brukes for å dekode SMS service meldinger fra observatør.
 */

using namespace std;
using namespace miutil;
using namespace kvalobs::decodeutil;

namespace kvalobs {
namespace decoder {
namespace comobsdecoder {


Sms13::
Sms13(const ParamList &paramList,
      kvalobs::decoder::comobsdecoder::ComObsDecoder &dec)
: SmsBase(paramList, dec)
{
}

Sms13::
~Sms13()
{
}

kvalobs::decodeutil::DecodedData*
Sms13::decode(long stationid,
              int  smscode,
              const SmsMelding::MeldingList &obs,
              std::string &returnMessage,
              std::list<kvalobs::kvRejectdecode> &rejected,
              bool &hasRejected)
{
    ostringstream ost;
    string msg;
    miTime obstime;
    DecodedData   *smsData;

    for(SmsMelding::CIMeldingList it=obs.begin(); it!=obs.end(); it++)
        ost << *it;

    msg=ost.str();
    trimstr(msg);

    if( ComObsDec.header.receivedTime.undef() )
        obstime = miTime::nowTime();
    else
        obstime = ComObsDec.header.receivedTime;

    try{
       smsData=new DecodedData(paramList, stationid, smscode+300);
    }
    catch(...){
       IDLOGFATAL( logid, "NOMEM: failed to allocate memory for SmsData!" << endl);
       return 0;
    }

    DecodedDataElem data=smsData->createDataElem();
    data.setDate(obstime);
    data.addData("TEXT", msg);
    smsData->add(data);

    return smsData;
}


}
}
}
