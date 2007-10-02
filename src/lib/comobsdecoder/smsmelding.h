/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: smsmelding.h,v 1.1.2.1 2007/09/27 09:02:24 paule Exp $                                                       

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
#ifndef __smsmelding_h_bm314__
#define __smsmelding_h_bm314__

#include <ostream>
#include <string>
#include <list>
#include <sstream>

class SMSApp;

class SmsMelding
{
 public:
  typedef std::list<std::string>                 MeldingList;
  typedef std::list<std::string>::iterator       IMeldingList;
  typedef std::list<std::string>::const_iterator CIMeldingList;
  
  typedef enum{OK, NOMASTER, RETRANSMIT, FORMAT, MISSINGARG, CODE_ERROR, 
	      NOSTATION, DATAFORMAT, INTERNAL, DATAWARNING} EResultCodes;

 private:
  int synopno;
  int climano;
  int code;
  EResultCodes resCode;
  std::stringstream errStream;
  MeldingList meldingList;
  std::string rawDoc;

  friend class SmsMeldingParser;

  void formatLogMsg(std::string &str, 
		    const std::string &timestamp);
  

 public:
  SmsMelding();
  SmsMelding(const std::string &autoObsPath_);
  ~SmsMelding()
    {
    }

  int getSynopno()const{ return synopno;}
  int getClimano()const{ return climano;}
  int getCode()const { return code;}
  bool hasClimanoOrSynopno()const{ return climano>0 || synopno>0;}
  const MeldingList& getMeldingList()const { return meldingList;}

  bool isValid()const{ return ((synopno>0 || climano>0) && code>-1 && 
			       meldingList.size()>0);}

  std::stringstream& getErrStream(){ return errStream;}
  EResultCodes  getResCode()const{ return resCode;}
  void setResCode(EResultCodes c){ resCode=c;}
  
  static std::string createReturnXML(SmsMelding *msg);
  static std::string createReturnXML(EResultCodes resCode, const std::string &comment);

  void        setRawDoc(const std::string &doc){ rawDoc=doc;}
  std::string getRawDoc()const{ return rawDoc;}

  friend std::ostream &operator<<(std::ostream &os, const SmsMelding &msg); 
};
  

std::ostream &operator<<(std::ostream &os, const SmsMelding &msg); 

#endif
