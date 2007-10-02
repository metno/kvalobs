/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: WMORaport.cc,v 1.6.2.2 2007/09/27 09:02:37 paule Exp $                                                       

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
#include <cctype>
#include <string>
#include <iostream>
#include <sstream>
#include <boost/regex.hpp>
#include "WMORaport.h"

using namespace std;
using namespace boost;

/**
 * Format of the WMO reports from NORCOM:
 * All message reports shall start with the 
 * sequence ZCZC, and after that comes the message type. The message type
 * is on the form: TTAAii CCCC DTG [BBB]
 * The message type is coded in TT. Where the diffrent message types is as
 * follows:
 *    SI,SM,SN             -> synop
 *    SA,SP                -> metar
 *    UE,UF,UK,UL,UM,US,UZ -> temp
 *    UG,UH,Ui,UP,UQ,UY    -> pilo
 *    UA,UD                -> arep
 *    SS                   -> drau
 *    SO                   -> bath
 *    ISRZ                 -> tide
 */


namespace{
  char *splitType[]={"METAR", "BBXX", 0};

  regex zczc("^ *ZCZC *[0-9]* *");
  regex nil("[0-9A-Za-z]* *NIL[ =]*", regex::icase| regex::normal);
  //  regex nil("[0-9A-Za-z]* *NIL[ =]*");
  regex NNNN("^ *N+ *");
  regex type("^ *((AA|BB)XX *\\d* *)|(METAR *)");
  regex synop("^ *S(I|M|N)\\w+ +\\w+ +\\d+ *\\w*");
  regex metar("^ *S(A|P)\\w+ +\\w+ +\\d+ *\\w*");
  regex temp("^ *U(E|F|K|L|M|S|Z)\\w+ +\\w+ +\\d+ *\\w*");
  regex pilo("^ *U(G|H|I|P|Q|Y)\\w+ +\\w+ +\\d+ *\\w*");
  regex arep("^ *U(A|D)\\w+ +\\w+ +\\d+ *\\w*");
  regex drau("^ *SS\\w+ +\\w+ +\\d+ *\\w*");
  regex bath("^ *SO\\w+ +\\w+ +\\d+ *\\w*");
  regex tide("^ *ISRZ\\w+ +\\w+ +\\d+ *\\w*");
  regex endMsg("^ *([0-9A-Za-z/\\-\\+]| |=)* *= *");
  //regex endMsg("^.*= .*");
  //regex endMsg("^ *([0-9A-Za-z/]| )* *= *");
  regex bl("^( |\t)*");
}

std::string 
WMORaport::tokenString(WMORaport::Token token)
{
  switch(token){
  case START:      return "START"; 
  case ENDMSG:     return "ENDMSG"; 
  case TYPE:       return "TYPE";
  case BL:         return "BL";
  case SYNOP:      return "SYNOP";
  case METAR:      return "METAR";
  case TEMP:       return "TEMP";
  case PILO:       return "PILO";
  case AREP:       return "AREP";
  case DRAU:       return "DRAU";
  case BATH:       return "BATH";
  case TIDE:       return "TIDE";
  case CONTINUE:   return "CONTINUE";
  case ENDOFINPUT: return "ENDOFINPUT";
  default:
    return "UNKNOWN";
  }
}

void 
WMORaport::pushError(Token token, Token prevToken, const std::string &buf)
{
  errorStr << "ERROR line: " << lineno << endl
	   << "----- buf:  [" << buf << "]" << endl 
	   << "----- Unexpected token: " << tokenString(token) << endl 
	   << "----- Prev token:       " << tokenString(prevToken) << endl;
 }


WMORaport::Token 
WMORaport::getToken(std::istringstream &inputStream,
		    std::string &buf)
{
  cmatch what;

  buf.clear();

  while(!putBackBuffer.empty() || getline(inputStream, buf)){
    
    //This code find a 'type' that may not be alone on a line.
    //We split out the type a generate two lines of thit one line.
    //The next line is in the putBackBuffer
    //
    //Type that this logic is implmented for at the momment:
    // 1) METAR
    // 2) BBXX

    if(!buf.empty()){
      //putBackBuffer is empty

      bool found=false;
      string::size_type pos;
      
      for(int i=0; splitType[i] && !found; i++){
	pos=buf.find(splitType[i]);

	if(pos!=string::npos){
	  putBackBuffer=buf.substr(pos+strlen(splitType[i]));
	  buf=buf.substr(0, pos+strlen(splitType[i]));
	  
	  while(!buf.empty() && isspace(buf[0]))
	    buf.erase(0, 1);
	  
	  while(!putBackBuffer.empty() && isspace(putBackBuffer[0]))
	    putBackBuffer.erase(0, 1);
	  
	  found=true;
	}
      }
    }else{
      buf=putBackBuffer;
      putBackBuffer.clear();
    }
      
      
    lineno++;
    
    if(regex_match(buf.c_str(), what, ::zczc)){
      continue;
    }else if(regex_match(buf.c_str(), what, ::nil)){
      continue;
    }else if(regex_match(buf.c_str(), what, ::NNNN)){
      continue;
    }else if(regex_match(buf.c_str(), what, ::bl)){
      return BL;
    }else if(regex_match(buf.c_str(), what, ::type)){
      return TYPE;
    }else if(regex_match(buf.c_str(), what, ::synop)){
      return SYNOP;
    }else if(regex_match(buf.c_str(), what, ::metar)){
      return METAR;
    }else if(regex_match(buf.c_str(), what, ::temp)){
      return TEMP;
    }else if(regex_match(buf.c_str(), what, ::pilo)){
      return PILO;
    }else if(regex_match(buf.c_str(), what, ::arep)){
      return AREP;
    }else if(regex_match(buf.c_str(), what, ::drau)){
      return DRAU;
    }else if(regex_match(buf.c_str(), what, ::bath)){
      return BATH;
    }else if(regex_match(buf.c_str(), what, ::tide)){
      return TIDE;
    }else if(regex_match(buf.c_str(), what, ::endMsg)){
      return ENDMSG;
    }else{
      return CONTINUE;
    }
  }
  
  return ENDOFINPUT;
}


WMORaport::WMORaport(bool warnAsError_):
  warnAsError(warnAsError_)
{
}

WMORaport::WMORaport(const WMORaport &r):
  synop_(r.synop_),temp_(r.temp_), metar_(r.metar_), pilo_(r.pilo_),
  arep_(r.arep_), drau_(r.drau_),bath_(r.bath_), tide_(r.tide_)
{
}
 
WMORaport::~WMORaport()
{
}


WMORaport& 
WMORaport::operator=(const WMORaport &rhs)
{
  if(this!=&rhs){
      synop_=rhs.synop_;
      temp_=rhs.temp_;
      metar_=rhs.metar_;
      pilo_=rhs.pilo_;
      arep_=rhs.arep_;
      drau_=rhs.drau_;
      bath_=rhs.bath_;
      tide_=rhs.tide_;
  }
  return *this;
}

bool 
WMORaport::decode(std::istringstream &ist)
{
  Token token=START;
  Token prevToken;
  string buf;
  ostringstream ost;
  MsgMap  *msgMap=&errorMap_;
  string  typeStr;
  string  sectionName;
  
  lineno=0;
  
  while(token!=ENDOFINPUT){
    prevToken=token;
    token=getToken(ist, buf);
    
    switch(token){
    case START:
      break;
    case ENDMSG:{
      bool error=false;

      switch(prevToken){
      case ENDMSG:
      case CONTINUE:
      case TYPE:
	//remove space at the beginning.
	while(!buf.empty() && isspace(buf[0]))
	  buf.erase(0, 1);
	
	if(!buf.empty())
	  ost << buf << endl;
	break;
      case BL:
	ost << buf << endl;
	break;
      default:
	//METAR har ofte feil. En feil som ofte opptrer
	//er at 'METAR' i begynnelsen av strengen mangler.
	//Vi kan reparere denne. Dvs legge den inn i rett
	//tabell
	if(prevToken==METAR || sectionName=="METAR"){
	  typeStr="METAR";
	  ost << buf << endl;
	}else{
	  pushError(token, prevToken, buf);
	  error=true;
	}
      }
      
      if(!error)
	(*msgMap)[typeStr].push_back(ost.str());
      
      ost.str("");
      break;
    }
    case TYPE:
      typeStr=buf;
      ost.str("");
      break;
    case BL:
      if(!ost.str().empty()){
	if(warnAsError){
	  errorStr << "ERROR line: " << lineno-1 << endl
		   << "----- Missing '=' " << endl;
	}
	
	buf=ost.str();

	string::size_type i=buf.find_last_of("\n");

	if(i!=string::npos && (i+1)==buf.length())
	  buf.insert(i, "=");
	else
	  buf.append("=");

	(*msgMap)[typeStr].push_back(buf);
      }
      ost.str("");
      break;
    case SYNOP:
      sectionName="SYNOP";
      msgMap=&synop_;
      ost.str("");
      break;
    case METAR:
      sectionName="METAR";
      msgMap=&metar_;
      ost.str("");
      break;
    case TEMP:
      sectionName="TEMP";
      msgMap=&temp_;
      ost.str("");
      break;
    case PILO:
      sectionName="PILO";
      msgMap=&pilo_;
      ost.str("");
      break;
    case AREP:
      sectionName="AREP";
      msgMap=&arep_;
      ost.str("");
      break;
    case DRAU:
      sectionName="DRAU";
      msgMap=&drau_;
      ost.str("");
      break;
    case BATH:
      sectionName="BATH";
      msgMap=&bath_;
      ost.str("");
      break;
    case TIDE:
      sectionName="TIDE";
      msgMap=&tide_;
      ost.str("");
      break;
    case CONTINUE:
      ost << buf << endl;
      break;
    case ENDOFINPUT:
      if(!ost.str().empty()){
	//ost << "=" << endl;
	(*msgMap)[typeStr].push_back(ost.str());
      }
      break;
    default:
      errorStr << "ERROR line: " << lineno << endl
	       << "----- UNKNOWN token from <getToken> " << endl;
      break;
    }
  }
  
  return true;
}


bool
WMORaport::split(const std::string &raport)
{
  std::istringstream inputStream;
  string msg(raport);

  errorStr.str("");
  cleanCR(msg);
  inputStream.str(msg);

  return decode(inputStream);
}





void 
WMORaport::cleanCR(std::string &buf)const
{
  string::size_type i;
  
  i=buf.find('\r', 0);
  
  while(i!=string::npos){
    buf.erase(i, 1);
    i=buf.find('\r', i);
  }
}


std::ostream& 
operator<<(std::ostream& output,
	   const WMORaport& r)
{
  WMORaport::CIMsgMap  itm;
  WMORaport::CIMsgList itl;
  
  for(itm=r.synop_.begin();
      itm!=r.synop_.end();
      itm++){
    output << itm->first << endl;
    for(itl=itm->second.begin();
	itl!=itm->second.end();
	itl++){
      output << *itl;
    }
    
    output << endl;
  }

  for(itm=r.temp_.begin();
      itm!=r.temp_.end();
      itm++){
    output << itm->first << endl;
    for(itl=itm->second.begin();
	itl!=itm->second.end();
	itl++){
      output << *itl;
    }
    output << endl;
  }

  for(itm=r.metar_.begin();
      itm!=r.metar_.end();
      itm++){
    output << itm->first << endl;
    for(itl=itm->second.begin();
	itl!=itm->second.end();
	itl++){
      output << *itl;
    }

    output << endl;
  }

  for(itm=r.pilo_.begin();
      itm!=r.pilo_.end();
      itm++){
    output << itm->first << endl;
    for(itl=itm->second.begin();
	itl!=itm->second.end();
	itl++){
      output << *itl;
    }

    output << endl;
  }

  for(itm=r.arep_.begin();
      itm!=r.arep_.end();
      itm++){
    output << itm->first << endl;
    for(itl=itm->second.begin();
	itl!=itm->second.end();
	itl++){
      output << *itl;
    }

    output << endl;
  }
  
  for(itm=r.drau_.begin();
      itm!=r.drau_.end();
      itm++){
    output << itm->first << endl;
    for(itl=itm->second.begin();
	itl!=itm->second.end();
	itl++){
      output << *itl;
    }
    
    output << endl;
  }
  
  for(itm=r.bath_.begin();
	itm!=r.bath_.end();
	itm++){
    output << itm->first << endl;
    for(itl=itm->second.begin();
	itl!=itm->second.end();
	itl++){
      output << *itl;
    }
    
    output << endl;
  }

  for(itm=r.tide_.begin();
      itm!=r.tide_.end();
      itm++){
    output << itm->first << endl;
    for(itl=itm->second.begin();
	itl!=itm->second.end();
	itl++){
      output << *itl;
    }
    output << endl;
  }

  output << endl;

  if(r.errorMap_.begin() != r.errorMap_.end())
  {
    output << "-------ERROR MAP (BEGIN) ---------" << endl;

    for(itm=r.errorMap_.begin();
	itm!=r.errorMap_.end();
	itm++){
      output << itm->first << endl;
      for(itl=itm->second.begin();
	  itl!=itm->second.end();
	  itl++){
	output << *itl;
      }
      output << "-------ERROR MAP (END) ---------" << endl;
    }
  }

  return output;
}
	
  
