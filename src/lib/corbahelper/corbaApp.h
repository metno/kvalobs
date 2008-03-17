/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: corbaApp.h,v 1.1.2.2 2007/09/27 09:02:25 paule Exp $                                                       

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
#ifndef __corbaApp_h__
#define __corbaApp_h__

#include <string>
#include <omniORB4/CORBA.h>

/**
 * \brief The namespace the CORBA helper utilities is in.
 */
namespace CorbaHelper{
  
  /**
   * \defgroup corbahelper CORBA helper classes and macros
   *
   * @{
   */
  
  /**
   * CORBA helper class. There must be only one of this in an 
   * application (Singelton). There is nothing to enforce this 
   * requierment, it is the programmers responsibility.
   */

  class CorbaApp{
    static CorbaApp                *app;
    
    PortableServer::POA_ptr        poa;
    CORBA::ORB_ptr                 orb;
    PortableServer::POAManager_ptr pman;
    std::string                    nameservice_;
    
  public:
    CorbaApp(int argn, char **argv, const char *options[][2]=0);
    virtual ~CorbaApp();

    /// Is the CORBA subsystem initialized.
    virtual bool  isOk()const;
    
    ///Get a referance too this instance.
    static CorbaApp *getCorbaApp();
    
    ///Get the root poa.
    PortableServer::POA_ptr        getPoa()const{ return poa;}
    
    /// Get the orb.
    CORBA::ORB_ptr                 getOrb()const{ return orb;}
    
    ///Get the poa manager.
    PortableServer::POAManager_ptr getPoaMgr()const { return pman;}
    
    /**
     * \brief putObjFromNS is a helper function that can be used 
     * to put referances to objects in the CORBA name server.
     * 
     * \param objref the object referance to be put in the CORBA nameserver.
     * \param name is a string on the form /path/to/name. Where /path/to 
     *    represent the context the name shall be put into. And name is the 
     *    name we want the objref to be known by.
     * \return true on succees and false otherwise.
     */  
    bool   putObjInNS(CORBA::Object_ptr objref, 
		      const std::string &name);
    
    
    /**
     * \brief getObjInNS is a helper function that can be used 
     * to get referances to an object in the CORBA name server.
     * 
     * \param name is a string on the form /path/to/name. Where /path/to 
     *    represent the context the name shall be get from. And name is the 
     *    name the objref is known by.
     * \return A corba Object, may be an NULL referans.
     */  
    CORBA::Object_ptr getObjFromNS(const std::string &name);

    ///Get a stringified object referance for the object.
    std::string corbaRef(CORBA::Object_ptr ptr);
    
    ///Create a CORBA object from a stringified objet referance. 
    CORBA::Object_ptr corbaRef(const std::string &ref);

    ///Set the nameservice server to be used.
    std::string setNameservice(const std::string &host);
    
    ///Get the nameservice erver that is used.
    std::string getNameservice()const{ return nameservice_;};
  };
  
  /** @} */
};

#endif
