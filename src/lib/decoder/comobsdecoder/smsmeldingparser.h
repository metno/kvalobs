/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: smsmeldingparser.h,v 1.1.2.1 2007/09/27 09:02:24 paule Exp $                                                       

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
#ifndef __smsmeldingparser_h_bm314__
#define __smsmeldingparser_h_bm314__

#include <string>
#include <sstream>
#include <libxml/parser.h>
#include "smsmelding.h"

class SmsMeldingParser
{

  typedef enum{ START, IN_MELDING, IN_SYNOPNO, IN_CLIMANO, 
		IN_DATA, ERROR, END} States;
  
  States state;
  bool   errFlag;
  std::ostringstream errMsg;
  std::ostringstream inputBuf;
  SmsMelding         *msg;

  bool checkNumber(const std::string &sNum, int &num);

 public:
  SmsMeldingParser();
  virtual ~SmsMeldingParser();

  virtual void startDocument();
  virtual void endDocument();
  virtual void startElement(const xmlChar *fullname,
		            const xmlChar **atts);
  virtual void endElement(const xmlChar *name);
  virtual void characters(const xmlChar *ch,
			  int           len);
  virtual void error(const std::string &msg);
  virtual void warning(const std::string &msg);
  virtual void fatalError(const std::string &msg);

  std::string getErrMsg()const{ return errMsg.str();}

  SmsMelding* parse(const std::string &melding);
};


#endif
