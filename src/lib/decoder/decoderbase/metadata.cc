/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: metadata.cc,v 1.1.2.3 2007/09/27 09:02:27 paule Exp $                                                       

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
#include <iostream>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <miutil/trimstr.h>
#include <libxml/parser.h>
#include "metadata.h"

using namespace std;
using namespace miutil;

#ifdef __cplusplus
extern "C" {
#endif

  static void 
  MetaParser_startDoc(void *user_data);
  
  static void 
  MetaParser_endDoc(void *user_data);
  
  static void 
  MetaParser_startElement(void   *user_data,
			  const xmlChar *fullname,
			  const xmlChar **atts);
  static void    
  MetaParser_endElement(void  *user_data,
			const xmlChar *name);
  

  static void        
  MetaParser_characters(void *user_data,
			const xmlChar *ch,
			int len);
  
  static void 
  MetaParser_errorFunc(void *ctx,
		       const char *msg,
		       ...);

  static void 
  MetaParser_warningFunc(void *ctx,
			 const char *msg,
			 ...);
  static void 
  MetaParser_fatalErrorSAXFunc(void *ctx,
			       const char *msg,
			       ...);


  static xmlSAXHandler MetaParser_xmlSaxHandler={
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
    MetaParser_startDoc,     //    startDocumentSAXFunc startDocument;
    MetaParser_endDoc,       //    endDocumentSAXFunc endDocument;
    MetaParser_startElement, //    startElementSAXFunc startElement;
    MetaParser_endElement,   //    endElementSAXFunc endElement;
    0, //    referenceSAXFunc reference;
    MetaParser_characters,   //    charactersSAXFunc characters;
    0, //    ignorableWhitespaceSAXFunc ignorableWhitespace;
    0, //    processingInstructionSAXFunc processingInstruction;
    0, //    commentSAXFunc comment;
    MetaParser_warningFunc,       //    warningSAXFunc warning;
    MetaParser_errorFunc,         //    errorSAXFunc error;
    MetaParser_fatalErrorSAXFunc, //    fatalErrorSAXFunc fatalError;
    0, //    getParameterEntitySAXFunc getParameterEntity;
    0, //    cdataBlockSAXFunc cdataBlock;
    0, //    externalSubsetSAXFunc externalSubset;
    1  //     int initialized;
  };

#ifdef __cplusplus
}
#endif

namespace miutil{
  namespace parsers{
    namespace meta{
      
      class MetaParserImpl{

	typedef enum{ START, IN_META, IN_META_PARAMS, IN_META_PARAMS_PARAM, 
			IN_META_PARAMS_PARAM_VALUE, ERROR, END} States;

	friend class MetaParser;

	States state;
	bool   errFlag;
	std::string        paramname; 
	std::ostringstream inputBuf;
	Meta               *meta;
	MetaParser         *parser;
	std::ostringstream &errMsg;
	std::ostringstream &warnMsg;


      public:

	MetaParserImpl(MetaParser *parser_, Meta *meta_)
	  :state(START), errFlag(false), meta(meta_), 
	   parser(parser_),errMsg(parser_->errMsg),
	   warnMsg(parser_->warnMsg)
	{}

	void startDocument();
	void endDocument();
	void startElement(const xmlChar *fullname,
			  const xmlChar **atts);
	void endElement(const xmlChar *name);
	void characters(const xmlChar *ch,
				int           len);
	void error(const std::string &msg);
	void warning(const std::string &msg);
	void fatalError(const std::string &msg);

      };
    }
  }
}


void 
miutil::parsers::meta::
Values::
checkRange(int index)const
{
  if(index>=value.size()){
    ostringstream ost;
    
    ost << "out_of_range: index=" << index << ".";
    
    if(value.size()==0)
      ost << " No values.";
    else
      ost << " Valid range [0," << value.size() << ">!";
    
    throw out_of_range(ost.str().c_str());
  }
}


void
miutil::parsers::meta::
Values::
addValue(const std::string &value_)
{
  value.push_back(value_);
}

std::string 
miutil::parsers::meta::
Values::
getValue(int index)const 
{
  checkRange(index);

  return value.at(index);
}


int
miutil::parsers::meta::
Values::
getValueAsInt(int index)const
{
  checkRange(index);

  if(value[index].empty())
    throw BadType("Empty value!");
  
  int n;

  if(sscanf(value[index].c_str(), "%d", &n)!=1)
    throw BadType("Not a integer!");
    
  return n;

}


long
miutil::parsers::meta::
Values::
getValueAsLong(int index)const
{
  checkRange(index);


  if(value[index].empty())
    throw BadType("Empty value!");
  
  long n;

  if(sscanf(value[index].c_str(), "%ld", &n)!=1)
    throw BadType("Not a long!");
    
  return n;
}
	

float 
miutil::parsers::meta::
Values::
getValueAsFloat(int index)const
{
  checkRange(index);

  
  if(value[index].empty())
    throw BadType("Empty value!");
  
  float n;

  if(sscanf(value[index].c_str(), "%f", &n)!=1)
    throw BadType("Not a float!");
    
  return n;

}

double
miutil::parsers::meta::
Values::
getValueAsDouble(int index)const
{
  checkRange(index);

  
  if(value[index].empty())
    throw BadType("Empty value!");
  
  double n;

  if(sscanf(value[index].c_str(), "%lf", &n)!=1)
    throw BadType("Not a double!");
    
  return n;

}

std::ostream& 
miutil::parsers::meta::
operator<<(std::ostream& out,
	   const miutil::parsers::meta::Values& vals)
{
  if(vals.value.size()==0){
    out << "[]";
    return out;
  }

  out << "[" << vals.value[0];

  for(int i=1; i<vals.value.size(); i++)
    out << ", " << vals.value[i];

  out << "]";

  return out;
}

void 
miutil::parsers::meta:: 
Meta::
addParamValue(const std::string &param, 
	      const std::string &value)
{
  params[param].addValue(value);
}


miutil::parsers::meta::
Meta::
Meta()
{
}

miutil::parsers::meta::
Meta::
~Meta()
{
}

miutil::parsers::meta::Values 
miutil::parsers::meta::
Meta::
getParamValue(const std::string &param,
	      const std::string &defval)const
{
  Values::Value values;

  //Init the values with the default values.
  if(!defval.empty()){
    std::string       def(defval);
    string::size_type i;
    string            val;
    
    do{
      i=def.find("|");
      
      if(i==string::npos){
	trimstr(def, TRIMBOTH);
	values.push_back(def);
	def.erase();
      }else{
	val=def.substr(0, i);
	trimstr(val, TRIMBOTH);
	def.erase(0, i+1);
	values.push_back(val);
      }
    }while(i!=string::npos);
  }

  CIParamMap it=params.find(param);

  if(it==params.end())
    return Values(values);
  
  if(it->second.value.size()>values.size())
    values.resize(it->second.value.size());
  

    //Overwrite any default values with the actual values if any.
  for(int i=0; i<it->second.value.size(); i++)
    values[i]=it->second.value[i];
    
  return Values(values);
}
    


miutil::parsers::meta::Meta::Params 
miutil::parsers::meta::
Meta::
getParams()const
{
  Params par;

  CIParamMap it=params.begin();

  for(; it!=params.end(); it++)
    par.push_back(it->first);

  return par;
}


miutil::parsers::meta::Meta& 
miutil::parsers::meta::Meta::
operator=(const miutil::parsers::meta::Meta &rhs)
{
  if(&rhs!=this){
    params=rhs.params;
  }

  return *this;
}



std::ostream& 
miutil::parsers::meta::
operator<<(std::ostream& out,
	   const miutil::parsers::meta::Meta& meta)
{
  if(meta.params.empty()){
    out << "Mata: No metadata!";
    return out;
  }

  out << "Meta: size=" << meta.params.size() << endl;
  
  for(Meta::CIParamMap it=meta.params.begin();
      it!=meta.params.end();
      it++)
    out << "  " << it->first << ": " << it->second << endl;


  return out;
}






void 
miutil::parsers::meta::
MetaParserImpl::
startDocument()
{
  //  cerr << "startDocument!\n";
}

void 
miutil::parsers::meta::
MetaParserImpl::
endDocument()
{
  //cerr << "endDocument!\n";
}

void 
miutil::parsers::meta::
MetaParserImpl::
startElement(const xmlChar *fullname,
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
  
  if(strcmp((const char*)fullname, "meta")==0){
    if(state!=START){
      errMsg<<"TAG: <meta>, but not state <START>\n"; 
      errFlag=true;
      return;
    }

    state=IN_META;
  }else if(strcmp((const char*)fullname, "params")==0){
    if(state!=IN_META){
      errMsg << "TAG: <" << fullname << ">, but not in state <IN_META>\n";
      errFlag=true;
      return;
    } 

    state=IN_META_PARAMS;
  }else if(strcmp((const char*)fullname, "param")==0){
    if(state!=IN_META_PARAMS){
      errMsg << "TAG: <" << fullname << ">, but not in state <IN_META_PARAMS>\n";
      errFlag=true;
      return;
    } 
    
    paramname.erase();
    
    if(!atts){
      warnMsg << "Metadata: <param>: Expecting 'name' attribute!\n";
      return;
    }
  
    if(strcmp((const char*)atts[0], "name")==0){
      if(atts[1]){
	std::string val((const char*)atts[1]);
	
	trimstr(val, TRIMBOTH);
	paramname=val;
      }
    }else{
      warnMsg << "Metadata: <param> '" <<(const char*)atts[0] << 
	"' invalid attribute!\n";
    }
      
    if(paramname.empty()){
      warnMsg << "Metadata: <param> attribute 'name' is empty!\n";
    }
    
    state=IN_META_PARAMS_PARAM;
  }else if(strcmp((const char*)fullname, "value")==0){
    if(state!=IN_META_PARAMS_PARAM){
      errMsg << "TAG: " << fullname << ", but not in state <IN_META_PARAMS_PARAM>\n";
      errFlag=true;
      return;
    } 

    state=IN_META_PARAMS_PARAM_VALUE;
  }else{
    errMsg << "UNKNOWN TAG: <" << fullname << ">\n";
    errFlag=true;
    
    state=ERROR;
    return;
  }
  
  return;
}

void 
miutil::parsers::meta::
MetaParserImpl::
endElement(const xmlChar *name)
{
  //cerr << "endElement: " << name << endl;
  
  if(errFlag){
    //cerr << "endElement (errFlag=true): " << name << endl;
    return;
  }
  
  string buf(inputBuf.str());
  trimstr(buf, TRIMBOTH);

  if(strcmp((const char*)name, "meta")==0){
    if(state!=IN_META){
      errMsg << "ENDTAG; metadata, but not in state <IN_METADATA>\n";
      errFlag=true;
      return;
    }

    state=END;
  }else if(strcmp((const char*)name, "params")==0){
    if(state!=IN_META_PARAMS_PARAM && state!=IN_META_PARAMS){
      errMsg << "ENDTAG; <param>, but not in state <IN_META_PARAMS_PARAM>\n";
      errFlag=true;
      return;
    }

    paramname.erase();

    state=IN_META;
  }else if(strcmp((const char*)name, "param")==0){
    if(state!=IN_META_PARAMS_PARAM){
      errMsg << "ENDTAG; <param>, but not in state <IN_META_PARAMS_PARAM>\n";
      errFlag=true;
      return;
    }

    paramname.erase();

    state=IN_META_PARAMS;
  }else if(strcmp((const char*)name, "value")==0){
    if(state!=IN_META_PARAMS_PARAM_VALUE){
      errMsg << "ENDTAG; <value>, but not in state <IN_META_PARAMS_PARAM_VALUE>\n";
      errFlag=true;
      return;
    }

    if(!paramname.empty()){
      meta->addParamValue(paramname, buf);
    }else{
      warnMsg << "ENDTAG: <value> state=IN_META_PARAMS_PARAM_VALUE.\n" <<
	"No 'paramname'\n";
    }
    
    state=IN_META_PARAMS_PARAM;
  }else{
    errMsg << "ENDTAG: " << name << " : in invalid state!";
    errFlag=true;
    return;
  }
}

void
miutil::parsers::meta::
MetaParserImpl::
characters(const xmlChar *ch,
			     int len)
{
  //cerr << "characters: [";
  
  //for(int i=0; i<len; i++)
  //  cerr << ch[i];

  //cerr << "]\n";

  inputBuf.write((const char*)ch, len);
}


void 
miutil::parsers::meta::
MetaParserImpl::
error(const std::string &msg)
{

  errMsg << "xml error: "<< msg << endl;
}

void 
miutil::parsers::meta::
MetaParserImpl::
warning(const std::string &msg)
{
  errMsg << "xml warning: "<< msg << endl;
}

void 
miutil::parsers::meta::
MetaParserImpl::
fatalError(const std::string &msg)
{
  errMsg << "xml fatal error: "<< msg << endl;
}

miutil::parsers::meta::
MetaParser::
MetaParser()
{
}

miutil::parsers::meta::
MetaParser::
~MetaParser()
{
}

miutil::parsers::meta::Meta* 
miutil::parsers::meta::
MetaParser::
parse(const std::string &melding)
{
  int ret;

  try{
    meta=new Meta();
  }
  catch(...){
    cerr << "MetaParser::parse: OUT OF MEMMORY\n";
    return 0;
  }

  try{
    impl=new MetaParserImpl(this, meta);
  }
  catch(...){
    cerr << "MetaParser::parse: OUT OF MEMMORY\n";
    delete meta;
    return 0;
  }
  
  errMsg.str("");
  ret=xmlSAXUserParseMemory(&MetaParser_xmlSaxHandler,
			    impl, melding.c_str(), melding.length());

  if(ret==0){

    if(!impl->errFlag){
      delete impl;
      return meta;
    }
  }

  delete impl;
  delete meta;
  
  return 0;
}

#ifdef __cplusplus
extern "C" {
#endif


static void 
MetaParser_startDoc(void *user_data)
{
  ((miutil::parsers::meta::MetaParserImpl*)user_data)->startDocument();
}

static void 
MetaParser_endDoc(void *user_data)
{
  ((miutil::parsers::meta::MetaParserImpl*)user_data)->endDocument();
}

static void 
MetaParser_startElement(void   *user_data,
			       const xmlChar *fullname,
			       const xmlChar **atts)
{
  ((miutil::parsers::meta::MetaParserImpl*)user_data)->startElement(fullname, atts);
}

static void    
MetaParser_endElement(void  *user_data,
                            const xmlChar *name)
{
  ((miutil::parsers::meta::MetaParserImpl*)user_data)->endElement(name);
}



static void        
MetaParser_characters(void *user_data,
                            const xmlChar *ch,
                            int len)
{
  ((miutil::parsers::meta::MetaParserImpl*)user_data)->characters(ch, len);
}


static void 
MetaParser_errorFunc(void *ctx,
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
      delete buf;
    }
    catch(...){
       cerr << "MetaParser_errorFunc: OUT OF MEMMORY!\n";
    }
  }else
    m=errBuf;

  va_end(args);
  
  ((miutil::parsers::meta::MetaParserImpl*)ctx)->error(m);
}

static void 
MetaParser_warningFunc(void *ctx,
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
      delete buf;
    }
    catch(...){
       cerr << "MetaParser_errorFunc: OUT OF MEMMORY!\n";
    }
  }else
    m=errBuf;

  va_end(args);

  ((miutil::parsers::meta::MetaParserImpl*)ctx)->warning(m);
}

static void 
MetaParser_fatalErrorSAXFunc(void *ctx,
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
      delete buf;
    }
    catch(...){
       cerr << "MetaParser_errorFunc: OUT OF MEMMORY!\n";
    }
  }else
    m=errBuf;

  va_end(args);


  ((miutil::parsers::meta::MetaParserImpl*)ctx)->fatalError(m);
}



#ifdef __cplusplus
}
#endif
