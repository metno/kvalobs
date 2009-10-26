/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvPythonInterpreter.h,v 1.1.2.2 2007/09/27 09:02:21 paule Exp $                                                       

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
#ifndef _kvPythonInterpreter_h
#define _kvPythonInterpreter_h
#ifdef USE_PYTHON
#if !defined(USE_MOCK_QABASE)
#include "kvQABaseTypes.h"
#endif

#include <Python.h>

#include <string>
#include <map>

using namespace std;
/*  Created by DNMI/FoU/PU: a.christoffersen@dnmi.no
    at Wed May  8 08:58:51 2002
*/


/**

  \brief QABase python-interpreter

*/

class kvPythonInterpreter
{
 public:
    /// The Perl interpreter instance
    static kvPythonInterpreter * my_interpreter;

    /// Do the necessary initialisation of python before usage
    static kvPythonInterpreter * getInterpreter();

    /// the clean up
    static void freeInterpreter();

	PyObject* dispatcher;

  public:
    kvPythonInterpreter();
    ~kvPythonInterpreter();

	/* check if runtime is initialized (or rather, if init was successful) */
    bool is_initialized() const { return dispatcher != NULL; }

	/* check if named script is registered.  returns 0 if not registed or
     needs to be reloaded, 1 if registered, and <0 on error.  all errors
     are logged.  */
  /* note that long-running clients should use this method regularily, to
     make sure that scripts are reloaded at regular intervals. */
    int is_registered(const std::string& name);

	
	/* register script with python dispatcher.  returns 0 if successful,
     <0 on error.  all errors are logged. */

    int register_script(const std::string & name, const std::string & script);
   
	/* call python dispatcher with named script and given data.  updates 
     the result property map.  returns 0 if successful, <0 on error.
     all errors are logged. */
    int dispatch(const std::string& name,
				const miutil::miTime& obstime,
				const std::list<kvQABase::script_var>& data,
				std::map<std::string, double>& retvalues);
    
};
#endif
#endif
