/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: smsmeldingparser.cc,v 1.1.2.1 2007/09/27 09:02:24 paule Exp $                                                       

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
#include <stdarg.h>
#include <ctype.h>
#include <miutil/trimstr.h>
#include "smsmeldingparser.h"

using namespace std;
using namespace miutil;

#ifdef __cplusplus
extern "C" {
#endif

  static void 
  SmsMeldingParser_startDoc(void *user_data);
  
  static void 
  SmsMeldingParser_endDoc(void *user_data);
  
  static void 
  SmsMeldingParser_startElement(void   *user_data,
				const xmlChar *fullname,
				const xmlChar **atts);
  static void    
  SmsMeldingParser_endElement(void  *user_data,
			      const xmlChar *name);

  static void        
  SmsMeldingParser_characters(void *user_data,
			      const xmlChar *ch,
			      int len);
  
  static void 
  SmsMeldingParser_errorFunc(void *ctx,
			     const char *msg,
			     ...);

  static void 
  SmsMeldingParser_warningFunc(void *ctx,
			       const char *msg,
			       ...);
  static void 
  SmsMeldingParser_fatalErrorSAXFunc(void *ctx,
				     const char *msg,
				     ...);

#ifdef __cplusplus
}
#endif

static xmlSAXHandler SmsMeldingParser_xmlSaxHandler={
  0, //    internalSubsetSAXFunc internalSubset;
  0, //    isStandaloneSAXFunc isStandalone;
  0, //    hasInternalSubsetSAXFunc hasInternalSubset;
  0, //    hasExternalSubsetSAXFunc hasExternalSubset;
  0, //    resolveEntitySAXFunc resolveEntity;
  0, //    getEntitySAXFunc getEntity;
  0, //    entityDeclSAXFunc entityDecl;
  0, //    notationDeclSAXFunc notationDecl;
  0, //    attributeDeclSAXFunc attributeDecl;
  0, //    elementDeclSAXFunc elementDecl;
  0, //    unparsedEntityDeclSAXFunc unparsedEntityDecl;
  0, //    setDocumentLocatorSAXFunc setDocumentLocator;
  SmsMeldingParser_startDoc,     //    startDocumentSAXFunc startDocument;
  SmsMeldingParser_endDoc,       //    endDocumentSAXFunc endDocument;
  SmsMeldingParser_startElement, //    startElementSAXFunc startElement;
  SmsMeldingParser_endElement,   //    endElementSAXFunc endElement;
  0, //    referenceSAXFunc reference;
  SmsMeldingParser_characters,   //    charactersSAXFunc characters;
  0, //    ignorableWhitespaceSAXFunc ignorableWhitespace;
  0, //    processingInstructionSAXFunc processingInstruction;
  0, //    commentSAXFunc comment;
  SmsMeldingParser_warningFunc,       //    warningSAXFunc warning;
  SmsMeldingParser_errorFunc,         //    errorSAXFunc error;
  SmsMeldingParser_fatalErrorSAXFunc, //    fatalErrorSAXFunc fatalError;
  0, //    getParameterEntitySAXFunc getParameterEntity;
  0, //    cdataBlockSAXFunc cdataBlock;
  0, //    externalSubsetSAXFunc externalSubset;
  1  //     int initialized;
  };


bool 
SmsMeldingParser::checkNumber(const std::string &sNum, int &num)
{
  for(string::size_type i=0; i<sNum.length(); i++){
    if(!isdigit(sNum[i])){
      return false;
    }
  }

  num=atoi(sNum.c_str());

  return true;
}

void 
SmsMeldingParser::startDocument()
{
  //  cerr << "startDocument!\n";
}

void 
SmsMeldingParser::endDocument()
{
  //cerr << "endDocument!\n";
}

void 
SmsMeldingParser::startElement(const xmlChar *fullname,
			       const xmlChar **atts)
{
  //  cerr << "startElement: " << fullname << endl;
  //cerr << "  attributer: \n";
  
  //for(char i=0; atts && atts[i]; i++)
  //  cerr <<"      " << atts[i] << endl;

  if(errFlag){
    //cerr << "startElement: <errFlag=true>\n";
    return;
  }

  inputBuf.str("");

  if(strcmp((const char*)fullname, "melding")==0){
    if(state!=START){
      errMsg<<"TAG: melding, but not state <START>\n"; 
      errFlag=true;
      return;
    }
    state=IN_MELDING;

    if(atts){
      if(strcmp((const char*)atts[0], "code")==0){
	if(atts[1]){
	  std::string val((const char*)atts[1]);

	  trimstr(val, TRIMBOTH);

	  if(!checkNumber(val, msg->code)){
	      errMsg << "TAG: melding: ATTRIBUT: code, val not a number!\n";
	      errFlag=true;
	      return;
	  }
	}
      }
    }  
  }else{
    if(state!=IN_MELDING){
      errMsg << "TAG: " << fullname << ", but not in state <IN_MELDING>\n";
      errFlag=true;
      return;
    }

    if(strcmp((const char*)fullname, "synopno")==0){
      state=IN_SYNOPNO;
      return;
    }else if(strcmp((const char*)fullname, "climano")==0){
      state=IN_CLIMANO;
      return;
    }else if(strcmp((const char*)fullname, "data")==0){
      state=IN_DATA;
      return;
    }else{
      //cerr<< "endElement: TAG: " << fullname << " unknown tag!\n";
      errMsg << "TAG: " << fullname << " unknown tag!\n";
      errFlag=true;
      state=ERROR;
      return;
    }
  }

}

void 
SmsMeldingParser::endElement(const xmlChar *name)
{
  //cerr << "endElement: " << name << endl;
  
  if(errFlag){
    //cerr << "endElement (errFlag=true): " << name << endl;
    return;
  }
  
  string buf(inputBuf.str());

  if(strcmp((const char*)name, "climano")==0){
    if(state!=IN_CLIMANO){
      errMsg << "ENDTAG; climano, but not in state <IN_CLIMANO>\n";
      errFlag=true;
      return;
    }

    trimstr(buf, TRIMBOTH);

    if(buf.empty() || !checkNumber(buf, msg->climano)){
      errMsg << "ENDTAG; climano, not a number!\n";
      errFlag=true;
      return;
    }
  }else if(strcmp((const char*)name, "synopno")==0){
    if(state!=IN_SYNOPNO){
      errMsg << "ENDTAG; synopno, but not in state <IN_SYNOPNO>\n";
      errFlag=true;
      return;
    }

    trimstr(buf, TRIMBOTH);
    
    if(buf.empty() || !checkNumber(buf, msg->climano)){
      errMsg << "ENDTAG; synopno, not a number!\n";
      errFlag=true;
      return;
    }
  }else if(strcmp((const char*)name, "data")==0){
    if(state!=IN_DATA){
      errMsg << "ENDTAG; data, but not in state <IN_DATA>\n";
      errFlag=true;
      return;
    }

    trimstr(buf, TRIMBOTH);
    msg->meldingList.push_back(buf);
  }else if(strcmp((const char*)name, "melding")==0){
    if(state!=IN_MELDING){
      errMsg << "ENDTAG; melding, but not in state <IN_MELDING>\n";
      errFlag=true;
      return;
    }
    state=END;
  }else{
    errMsg << "ENDTAG: " << name << " : in invalid state!";
    errFlag=true;
    return;
  }

  state=IN_MELDING;
   
}

void
SmsMeldingParser::characters(const xmlChar *ch,
			     int len)
{
  //cerr << "characters: [";
  
  //for(int i=0; i<len; i++)
  //  cerr << ch[i];

  //cerr << "]\n";

  inputBuf.write((const char*)ch, len);
}


void 
SmsMeldingParser::error(const std::string &msg)
{

  errMsg << "xml error: "<< msg << endl;
}

void 
SmsMeldingParser::warning(const std::string &msg)
{
  errMsg << "xml warning: "<< msg << endl;
}

void 
SmsMeldingParser::fatalError(const std::string &msg)
{
  errMsg << "xml fatal error: "<< msg << endl;
}

SmsMeldingParser::SmsMeldingParser()
  :state(START), errFlag(false) 
{
}

SmsMeldingParser::~SmsMeldingParser()
{
}

SmsMelding* 
SmsMeldingParser::parse(const std::string &melding)
{
  int ret;

  try{
    msg=new SmsMelding();
    msg->setRawDoc(melding);
  }
  catch(...){
    cerr << "SmsMeldingParser::parse: OUT OF MEMMORY\n";
    return 0;
  }
  
  errMsg.str("");
  state=START;
  ret=xmlSAXUserParseMemory(&SmsMeldingParser_xmlSaxHandler,
			    this, melding.c_str(), melding.length());

  if(ret==0){
    if(!errFlag){
      //cerr << "Parse: success!\n";
      return msg;
    }
  }

  delete msg;

  cerr << "Parse: failed\n";

  return 0;
}

#ifdef __cplusplus
extern "C" {
#endif


static void 
SmsMeldingParser_startDoc(void *user_data)
{
  ((SmsMeldingParser*)user_data)->startDocument();
}

static void 
SmsMeldingParser_endDoc(void *user_data)
{
  ((SmsMeldingParser*)user_data)->endDocument();
}

static void 
SmsMeldingParser_startElement(void   *user_data,
			       const xmlChar *fullname,
			       const xmlChar **atts)
{
  ((SmsMeldingParser*)user_data)->startElement(fullname, atts);
}

static void    
SmsMeldingParser_endElement(void  *user_data,
                            const xmlChar *name)
{
  ((SmsMeldingParser*)user_data)->endElement(name);
}

static void        
SmsMeldingParser_characters(void *user_data,
                            const xmlChar *ch,
                            int len)
{
  ((SmsMeldingParser*)user_data)->characters(ch, len);
}


static void 
SmsMeldingParser_errorFunc(void *ctx,
			   const char *msg,
			   ...)
{
  va_list args;
  char errBuf[256];
  char *buf;
  int  n;
  string m;

  va_start(args, msg);
  
  n=vsnprintf(errBuf, 256, msg, args);

  if((n+1)>256){
    try{
      buf=new char[n+1];
      va_start(args, msg);
      n=vsnprintf(buf, n+1, msg, args);
      m=buf;
      delete[] buf;
    }
    catch(...){
       cerr << "SmsMeldingParser_errorFunc: OUT OF MEMMORY!\n";
    }
  }else
    m=errBuf;

  va_end(args);
  
  ((SmsMeldingParser*)ctx)->error(m);
}

static void 
SmsMeldingParser_warningFunc(void *ctx,
			       const char *msg,
			     ...)
{
  va_list args;
  char errBuf[256];
  char *buf;
  int  n;
  string m;

  va_start(args, msg);
  
  n=vsnprintf(errBuf, 256, msg, args);

  if((n+1)>256){
    try{
      buf=new char[n+1];
      va_start(args, msg);
      n=vsnprintf(buf, n+1, msg, args);
      m=buf;
      delete[] buf;
    }
    catch(...){
       cerr << "SmsMeldingParser_errorFunc: OUT OF MEMMORY!\n";
    }
  }else
    m=errBuf;

  va_end(args);

  ((SmsMeldingParser*)ctx)->warning(m);
}

static void 
SmsMeldingParser_fatalErrorSAXFunc(void *ctx,
				   const char *msg,
				   ...)
{
  va_list args;
  char errBuf[256];
  char *buf;
  int  n;
  string m;

  va_start(args, msg);
  
  n=vsnprintf(errBuf, 256, msg, args);

  if((n+1)>256){
    try{
      buf=new char[n+1];
      va_start(args, msg);
      n=vsnprintf(buf, n+1, msg, args);
      m=buf;
      delete[] buf;
    }
    catch(...){
       cerr << "SmsMeldingParser_errorFunc: OUT OF MEMMORY!\n";
    }
  }else
    m=errBuf;

  va_end(args);


  ((SmsMeldingParser*)ctx)->fatalError(m);
}



#ifdef __cplusplus
}
#endif
