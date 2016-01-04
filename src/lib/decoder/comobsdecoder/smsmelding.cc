/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: smsmelding.cc,v 1.1.2.1 2007/09/27 09:02:24 paule Exp $                                                       

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
#include <iostream>
#include <miutil/trimstr.h>
#include "smsmelding.h"

using namespace std;
using namespace miutil;

SmsMelding::SmsMelding()
    : synopno(-1),
      climano(-1),
      code(-1),
      resCode(OK) {

}

SmsMelding::SmsMelding(const std::string &autoObsPath_)
    : synopno(-1),
      climano(-1),
      code(-1),
      resCode(OK) {
}

std::string SmsMelding::createReturnXML(SmsMelding *msg) {
  if (msg->getResCode() != OK) {
    msg->getErrStream() << "\n\n" << *msg;
  }

  return SmsMelding::createReturnXML(msg->getResCode(),
                                     msg->getErrStream().str());
}

std::string SmsMelding::createReturnXML(EResultCodes resCode,
                                        const std::string &comment) {
  stringstream os;

  os << "<?xml version=\"1.0\"?><result><code>";

  switch (resCode) {
    case OK:
      os << "OK";
      break;
    case NOMASTER:
      os << "NOMASTER";
      break;
    case RETRANSMIT:
      os << "RETRANSMIT";
      break;
    case FORMAT:
      os << "FORMAT";
      break;
    case MISSINGARG:
      os << "MISSINGARG";
      break;
    case CODE_ERROR:
      os << "CODE_ERROR";
      break;
    case NOSTATION:
      os << "NOSTATION";
      break;
    case DATAFORMAT:
      os << "DATAFORMAT";
      break;
    case INTERNAL:
      os << "INTERNAL";
      break;
    case DATAWARNING:
      os << "DATAWARNING";
      break;
  }

  os << "</code><comment>" << comment << "</comment></result>";

  return os.str();

}

std::ostream&
operator<<(std::ostream &os, const SmsMelding &msg) {
  os << "SMS melding:\n" << "  valid: " << (msg.isValid() ? "TRUE" : "FALSE")
     << endl << "  climano: " << msg.getClimano() << endl << "  synopno: "
     << msg.getSynopno() << endl << "  code:    " << msg.getCode() << endl
     << "  DATA: " << endl;

  for (SmsMelding::CIMeldingList it = msg.getMeldingList().begin();
      it != msg.getMeldingList().end(); it++)
    os << "    " << *it << endl;

  os << endl << endl;

  return os;
}

void SmsMelding::formatLogMsg(std::string &str, const std::string &timestamp) {
  std::string dash(timestamp.length() - 1, '-');
  std::string::size_type i;

  dash += " ";

  trimstr(str, TRIMBACK);

  str.insert(0, timestamp);
  str += "\n";

  i = str.find_first_of('\n');

  while (i != std::string::npos) {
    if ((i + 1) < str.length()) {
      str.insert(i + 1, dash);
      i = str.find_first_of('\n', i + 1);
    } else
      break;
  }
}

