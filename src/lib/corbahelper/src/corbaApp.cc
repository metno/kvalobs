/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: corbaApp.cc,v 1.9.6.2 2007/09/27 09:02:25 paule Exp $                                                       

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
#include <dnmithread/mtcout.h>
#include "../include/corbahelper/corbaApp.h"

using namespace CorbaHelper;
using namespace std;

CorbaHelper::CorbaApp *CorbaHelper::CorbaApp::app=0;
    
CorbaApp::CorbaApp(int argn, char **argv, const char *options[][2])
{
  try{
    orb = CORBA::ORB_init(argn, argv, "omniORB4", options);
    CORBA::Object_var obj = orb->resolve_initial_references("RootPOA");
    poa = PortableServer::POA::_narrow(obj);
    pman = poa->the_POAManager();
    app=this;
  }
  catch(CORBA::SystemException&) {
    CERR("main: Caught CORBA::SystemException." << endl);
  }
  catch(CORBA::Exception&) {
    CERR("main: Caught CORBA::Exception." << endl);
  }
  catch(omniORB::fatalException& fe) {
    CERR("main: Caught omniORB::fatalException:" << endl
	 << "  file: " << fe.file() << endl
	 << "  line: " << fe.line() << endl
	 << "  mesg: " << fe.errmsg() << endl);
  }
  catch(...) {
    CERR("main: Caught unknown exception." << endl);
  }

}
CorbaApp::~CorbaApp()
{
}

CorbaApp*
CorbaApp::getCorbaApp()
{
  return app;
}

bool  
CorbaApp::isOk()const
{ 
  return app!=0;
}


#if 0
bool  
CorbaApp::putObjInNS(CORBA::Object_ptr objref, 
		     const std::string &name_)
{

  if(!app){
    CERR("FATAL CorbaApp::putObjInNS: No valid CorbaApp instance!\n");
    return false;
  }
  string name(name_);
  string nameContext;
  string sObject;
  string::size_type i;
  CosNaming::NamingContextExt_var rootContext;
  CosNaming::NamingContextExt_var toContext;
  CosNaming::Name_var contextName;
  CosNaming::Name_var objectName;

  if(name.length()==0){
    CERR("ERROR: name is empty!\n");
    return false;
  }

  if(name[0]=='/'){
    name.erase(0, 1);
   
    if(name.length()==0)
      return false;
  }

  i=name.find_last_of("/");
  
  if(i!=string::npos){
    nameContext=name.substr(0, i);
    
    if(i>=(name.length()-1)){
      CERR("ERROR: no object name <" << name_ << ">\n");
      return false;
    }
    
    sObject=name.substr(i+1);
  }



  try{
    try{
      // Obtain a reference to the root context of the Name service:
      CORBA::Object_var obj;
      obj = orb->resolve_initial_references("NameService");

      // Narrow the reference returned.
      rootContext = CosNaming::NamingContextExt::_narrow(obj);
      if( CORBA::is_nil(rootContext) ) {
	CERR("Failed to narrow the root naming context." << endl);
	return false;
      }
    }
    catch(CORBA::ORB::InvalidName& ex) {
      // This should not happen!
      cerr << "Service required is invalid [does not exist]." << endl;
      return 0;
    }


    if(!nameContext.empty()){
      try{
	contextName=rootContext->to_name(nameContext.c_str());
	CORBA::Object_var obj = rootContext->bind_new_context(contextName);
	toContext = CosNaming::NamingContextExt::_narrow(obj);
      }
      catch(CosNaming::NamingContext::AlreadyBound& ex) {
	// The context already exists.
	// Just resolve the name and assign toContext to the object returned.
	
	CERR("Context <" << nameContext << "> already exists , resolve context!\n");
	CORBA::Object_var obj;
	obj = rootContext->resolve(contextName);
	toContext = CosNaming::NamingContextExt::_narrow(obj);
	
	if( CORBA::is_nil(toContext) ){
	  CERR("ERROR: Failed to narrow naming context <" << nameContext << ">\n");
	  return false;
	}
      }
      catch(CosNaming::NamingContext::InvalidName &ex) {
	CERR("Exception: InvalidName <" << name_ << ">\n");
	return false;
      }
    }else
      toContext=rootContext;


    try{
      objectName=rootContext->to_name(sObject.c_str());
      toContext->bind(objectName, objref);
    }
    catch(CosNaming::NamingContext::AlreadyBound& ex) {
      CERR("Object <" << sObject << "> already exists, rebind!\n");
      toContext->rebind(objectName, objref);
    }
  }
  catch(CORBA::COMM_FAILURE& ex) {
    CERR("Caught system exception COMM_FAILURE -- unable to contact the "
         << "naming service." << endl);
    return false;
  }
  catch(CORBA::SystemException&) {
    CERR("Caught a CORBA::SystemException while using the naming service."
	 << endl);
    return false;
  }
    
  return true;
}

#endif

bool  
CorbaApp::putObjInNS(CORBA::Object_ptr objref, 
		     const std::string &name_)
{
  CORBA::Object_var obj;

  if(!app){
    CERR("FATAL CorbaApp::putObjInNS: No valid CorbaApp instance!\n");
    return false;
  }

  CERR("CorbaApp::putObjInNS: name <" << name_ << ">!\n");

  string name(name_);
  string nameContext;
  string sObject;
  string::size_type i;
  CosNaming::NamingContextExt_var rootContext;
  CosNaming::NamingContextExt_var toContext;
  CosNaming::Name_var contextName;
  CosNaming::Name_var objectName;

  if(name.length()==0){
    CERR("ERROR: name is empty!\n");
    return false;
  }

  if(name[0]=='/'){
    name.erase(0, 1);
   
    if(name.length()==0)
      return false;
  }

  i=name.find_last_of("/");
  
  if(i!=string::npos){
    nameContext=name.substr(0, i);
    
    if(i>=(name.length()-1)){
      CERR("ERROR: no object name <" << name_ << ">\n");
      return false;
    }
    
    sObject=name.substr(i+1);
  }


  try{
    try{
      string nameService("corbaname:");

      if(nameservice_.empty())
	nameService+="rir";
      else{
	nameService+=":";
	nameService+=nameservice_;
      }
      nameService+=":/NameService";

      CERR("CorbaApp::putObjInNS: <" << nameService << ">!\n"); 

      obj=orb->string_to_object(nameService.c_str()); 
    }
    catch(...){
      CERR("Failed: can't look up CORBA nameservice!\n");
      return false;
    }

    try{
      // Narrow the reference returned.
      rootContext = CosNaming::NamingContextExt::_narrow(obj);
      
      if( CORBA::is_nil(rootContext) ) {
	CERR("Failed to narrow the root naming context (CORBA Nameservice)" 
	     << endl);
	return false;
      }
    }
    catch(CORBA::ORB::InvalidName& ex) {
      // This should not happen!
      CERR("Service required is invalid [does not exist]." << endl);
      return false;
    }
    catch(...){
      CERR("UNKNOWN exception: CorbaApp::putObjInNS!!!!\n");
      return false;
    }
      


    if(!nameContext.empty()){
      try{
	contextName=rootContext->to_name(nameContext.c_str());
	CORBA::Object_var obj = rootContext->bind_new_context(contextName);
	toContext = CosNaming::NamingContextExt::_narrow(obj);
      }
      catch(CosNaming::NamingContext::AlreadyBound& ex) {
	// The context already exists.
	// Just resolve the name and assign toContext to the object returned.
	
	CERR("Context <" << nameContext << "> already exists , resolve context!\n");
	CORBA::Object_var obj;
	obj = rootContext->resolve(contextName);
	toContext = CosNaming::NamingContextExt::_narrow(obj);
	
	if( CORBA::is_nil(toContext) ){
	  CERR("ERROR: Failed to narrow naming context <" << nameContext << ">\n");
	  return false;
	}
      }
      catch(CosNaming::NamingContext::InvalidName &ex) {
	CERR("Exception: InvalidName <" << name_ << ">\n");
	return false;
      }
      catch(...){
	CERR("Exception: CorbaApp::putObjInNS: unexpected exception!\n");
	return false;
      }
    }else
      toContext=rootContext;


    try{
      objectName=rootContext->to_name(sObject.c_str());
      toContext->bind(objectName, objref);
    }
    catch(CosNaming::NamingContext::AlreadyBound& ex) {
      CERR("Object <" << sObject << "> already exists, rebind!\n");
      toContext->rebind(objectName, objref);
    }
  }
  catch(CORBA::COMM_FAILURE& ex) {
    CERR("Caught system exception COMM_FAILURE -- unable to contact the "
         << "naming service." << endl);
    return false;
  }
  catch(CORBA::SystemException&) {
    CERR("Caught a CORBA::SystemException while using the naming service."
	 << endl);
    return false;
  }
    
  return true;
}



std::string 
CorbaApp::corbaRef(CORBA::Object_ptr ptr)
{
  CORBA::String_var s;

  try{
    s=orb->object_to_string(ptr);
  }
  catch(...){
    return string("");
  }

  return string(s);
}

CORBA::Object_ptr 
CorbaApp::corbaRef(const std::string &ref)
{
  CORBA::Object_ptr ptr;

  try{
    ptr=orb->string_to_object(ref.c_str());
  }
  catch(...){
    return CORBA::Object::_nil();
  }

  return ptr;
}






#if 0
CORBA::Object_ptr
CorbaApp::getObjFromNS(const std::string &name_)
{
  CORBA::Object_ptr obj;
  string name("corbaname:rir:#");

  

 if(!app){
    CERR("FATAL CorbaApp::getObjFromNS: No valid CorbaApp instance!\n");
    return CORBA::Object::_nil();
  }


  if(name_.length()==0){
    CERR("ERROR: name is empty!\n");
    return CORBA::Object::_nil();
  }


  string::size_type i=name_.find_first_not_of('/');
  
  if(i==string::npos )
    return CORBA::Object::_nil();
    
  name+=name_.substr(i);
  
  CERR("Looking up object in CORBA nameservice: " << name << endl); 

  try{
    obj=orb->string_to_object(name.c_str()); 
  }
  catch(...){
    CERR("Exception: cant find object or nameservice\n");
    return CORBA::Object::_nil();
  }

  return obj;

}
#endif

CORBA::Object_ptr
CorbaApp::getObjFromNS(const std::string &name_)
{
  CORBA::Object_ptr obj;
  string name("corbaname:");

  if(nameservice_.empty())
    name+="rir";
  else{
    name+=":";
    name+=nameservice_;
  }
  
  name+=":#";


 if(!app){
    CERR("FATAL CorbaApp::getObjFromNS: No valid CorbaApp instance!\n");
    return CORBA::Object::_nil();
  }


  if(name_.length()==0){
    CERR("ERROR: name is empty!\n");
    return CORBA::Object::_nil();
  }


  string::size_type i=name_.find_first_not_of('/');
  
  if(i==string::npos )
    return CORBA::Object::_nil();
    
  name+=name_.substr(i);
  
  CERR("Looking up object in CORBA nameservice: " << name << endl); 

  try{
    obj=orb->string_to_object(name.c_str()); 
  }
  catch(...){
    CERR("Exception: cant find object or nameservice\n");
    return CORBA::Object::_nil();
  }

  return obj;

}




std::string 
CorbaApp::setNameservice(const std::string &host)
{
  std::string tmp=nameservice_;

  nameservice_=host;

  return tmp;
}
