/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvPerlParser.cc,v 1.1.2.5 2007/09/27 09:02:21 paule Exp $                                                       

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
#ifdef USE_PYTHON
#include "kvPythonInterpreter.h"

#if !defined(USE_MOCK_QABASE)
#include <milog/milog.h>
#endif

kvPythonInterpreter* kvPythonInterpreter::my_interpreter = NULL;



#if PY_VERSION_HEX < 0x02050000 && !defined(PY_SSIZE_T_MIN)
typedef int Py_ssize_t;
#define PY_SSIZE_T_MAX INT_MAX
#define PY_SSIZE_T_MIN INT_MIN
#endif

// --------------------------------------------------------------------
// local helper classes

class PythonMethod {
  /* method name wrapper.  use in proxy methods to capture the context
     needed by the error handler, just in case */
  kvPythonInterpreter *interp;
  const char* method;
  const std::string& name;
public:
  inline PythonMethod(kvPythonInterpreter* interp_, const char* method_,
		      const std::string& name_)
    : interp(interp_), method(method_), name(name_) {
    // IDLOGDEBUG("html", "Enter '" << method << "'" << std::endl );
  }
  inline ~PythonMethod() {
    // IDLOGDEBUG("html", "Leave '" << method << "'" << std::endl );
  }
public:
  /* report error with given errcode.  generates a log message based
     on the current exception state. */
  int error(int errcode=-1);
};

// --------------------------------------------------------------------
// simple output buffer type, used for output redirection.  basically,
// this maps a Python 'write' method call to a += on a string object.

typedef struct {
    PyObject_HEAD
    std::string* buffer;
} OutputObject;

static void output_dealloc(OutputObject* self);
static PyObject* output_getattr(OutputObject* self, char* name);

static PyTypeObject OutputType = {
    PyObject_HEAD_INIT(&PyType_Type)
    0, "Output", sizeof(OutputObject), 0,
    /* methods */
    (destructor) output_dealloc, /* tp_dealloc */
    0, /* tp_print */
    (getattrfunc) output_getattr, /* tp_getattr */
    0, /* tp_setattr */
};

#define Output_Check(op) ((op) != NULL && (op)->ob_type == &OutputType)
#define Output_AsCString(op) (((OutputObject*) (op))->buffer->c_str())

static PyObject* output_new()
{
  OutputObject* self = PyObject_NEW(OutputObject, &OutputType);
  if (!self)
    return NULL;
  self->buffer = new std::string;
  return (PyObject*) self;
}

static PyObject* output_write(OutputObject* self, PyObject* args)
{
  PyObject* text;
  if (!PyArg_ParseTuple(args, "O!:text", &PyString_Type, &text))
    return NULL;
  *self->buffer += PyString_AS_STRING(text);
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject* output_getattr(OutputObject* self, char* name)
{
  static PyMethodDef output_methods[] = {
    {"write", (PyCFunction) output_write, METH_VARARGS},
  };
  return Py_FindMethod(output_methods, (PyObject*) self, name);
}

static void output_dealloc(OutputObject* self)
{
  delete self->buffer;
  PyObject_DEL(self);
}

// --------------------------------------------------------------------
// convert helpers

static void add_int(PyObject* dict, const std::string& key, int value)
{
  /* add integer to dictionary */

  /* build key */
  PyObject* keyobj = PyString_FromStringAndSize(key.c_str(), key.size());
  if (!keyobj)
    return;

  /* build integer value */
  PyObject* valueobj = PyInt_FromLong(value);
  if (!valueobj) {
    Py_DECREF(keyobj);
    return;
  }

  PyDict_SetItem(dict, keyobj, valueobj);

  Py_DECREF(keyobj);
  Py_DECREF(valueobj);
}

static PyObject* open_list(PyObject* dict, const std::string& key)
{
  /* add list to dictionary.  returns a list object that can be
     populated with add_int (below) and add_value.  the resulting list
     object must be closed with close_list. */

  /* build key */
  PyObject* keyobj = PyString_FromStringAndSize(key.c_str(), key.size());
  if (!keyobj)
    return NULL;

  PyObject* valueobj = PyList_New(0);
  if (!valueobj) {
    Py_DECREF(keyobj);
    return NULL;
  }

  PyDict_SetItem(dict, keyobj, valueobj);

  Py_DECREF(keyobj);

  return valueobj;
}

static void add_int(PyObject* list, int value)
{
  /* add integer to list object */

  if (!list)
    return;

  PyObject* valueobj = PyInt_FromLong(value);
  if (!valueobj)
    return;

  PyList_Append(list, valueobj);

  Py_DECREF(valueobj);
}

static void add_value(PyObject* list, const std::string& value, int missing)
{
  /* add value to list object.  values are given as strings, but are
     converted to floats by this layer */

  if (!list)
    return;

#if 1
  /* map missing values to None */
  PyObject* valueobj;
  if (missing > 0) {
    Py_INCREF(Py_None);
    valueobj = Py_None;
  } else {
    valueobj = PyFloat_FromDouble(atof(value.c_str()));
    if (!valueobj)
      return;
  }
#else
  /* pass through missing objects */
  PyObject* valueobj = PyFloat_FromDouble(atof(value.c_str()));
  if (!valueobj)
    return;
#endif

  PyList_Append(list, valueobj);

  Py_DECREF(valueobj);
}

static inline void close_list(PyObject* list)
{
  /* close a list object that was opened with open_list */

  Py_XDECREF(list);
}

static int add_script_var(PyObject* dict, const kvQABase::script_var& var)
{
  /* add contents of a script_var object to dictionary */

  if (var.allpos.empty() || var.alltimes.empty() || var.pars.empty())
    return 0; // ignore empty records

  /* iterator types used below */
  typedef std::vector<int>::const_iterator int_iter;
  typedef std::vector<kvQABase::script_par>::const_iterator script_par_iter;
  typedef std::map<int, std::map<int, kvQABase::par_values> >::const_iterator
    station_map_iter;
  typedef std::map<int, kvQABase::par_values>::const_iterator
    timeslot_map_iter;

  PyObject* obj;

  /* timeslots */
  add_int(dict, var.source + "_numtimes", var.alltimes.size());
  obj = open_list(dict, var.source + "_timeoffset");
  for (int_iter tp = var.alltimes.begin(); tp != var.alltimes.end(); tp++)
    add_int(obj, *tp);
  close_list(obj);

  /* stations */
  add_int(dict, var.source + "_numstations", var.allpos.size());
  obj = open_list(dict, var.source + "_stations");
  for (int_iter pp = var.allpos.begin(); pp != var.allpos.end(); pp++)
    add_int(obj, *pp);
  close_list(obj);

  /* global missing flag (observation data only) */
  if (var.dsource == kvQABase::obs_data ||
      var.dsource == kvQABase::refobs_data)
    add_int(dict, var.source + "_missing", var.missing_data);

  /* parameters */
  for (script_par_iter par = var.pars.begin(); par != var.pars.end(); par++) {

    PyObject* missing;
    PyObject* controlinfo;

    obj = open_list(dict, par->signature);

    if (var.dsource == kvQABase::obs_data ||
	var.dsource == kvQABase::refobs_data) {
      /* missing and controlinfo are only used for observations */
      missing = open_list(dict, par->signature + "_missing");
      controlinfo = open_list(dict, par->signature + "_controlinfo");
    } else
      missing = controlinfo = NULL;

    /* values: loop over stations first, time slots second */
    for (int_iter pp = var.allpos.begin(); pp != var.allpos.end(); pp++) {
      for (int_iter tp = var.alltimes.begin(); tp != var.alltimes.end(); tp++){

	/* fetch parameter value from value map */
	kvQABase::par_values p; /* default is "missing" */

	/* look for it in the station map.  note that we cannot use
	   operator[] here; it may modify the map, and so doesn't work
	   on const maps */

	station_map_iter is = par->values.find(*pp);
	if (is != par->values.end()) {
	  timeslot_map_iter it = (is->second).find(*tp);
	  if (it != (is->second).end())
	    p = it->second;
	}

	add_value(obj, p.value, p.status);

	if (missing)
	  add_int(missing, p.status);

	if (controlinfo) {
          for (int f = 0; f < 16; f++)
	    add_int(controlinfo, p.cinfo.flag(f));
	}
      }
    }

    close_list(obj);
    close_list(missing);
    close_list(controlinfo);

  }

  return 0;
}

// --------------------------------------------------------------------
// implementation

int PythonMethod::error(int errcode)
{
  /* internal: generates a log message based on the current exception
     status */

  const char* message = "Internal Error";
  PyObject* message_obj = NULL;

  if (PyErr_Occurred()) {
    /* generate a traceback message for this error.  the PyErr_Print
       function prints to sys.stderr, so we need to plug in our own
       output object to get the output as a string */
    PyObject* sys_stderr = PySys_GetObject("stderr");
    PyObject* output = output_new();
    if (sys_stderr && output) {
      PySys_SetObject("stderr", output);
      PyErr_Print();
      PySys_SetObject("stderr", sys_stderr);
      message = Output_AsCString(output);
    } else
      message = "Internal Error: cannot retrieve error message";
    Py_XDECREF(output);
    PyErr_Clear();
  } else if (!interp->dispatcher)
    message = "Internal Error: dispatcher not available";
  else
    message = "Internal Error: unknown error, exception state not set";

  if (method) {
    IDLOGERROR("html", "Error in method '" << method <<
	       "' when processing script '" << name << "':" << std::endl
	       << message << std::endl);
  } else {
    /* FIXME: this is a fatal error.  shut down server? */
    IDLOGERROR("html", "Error when loading bootstrap script:" << std::endl
	       << message << std::endl);
  }

  Py_XDECREF(message_obj);

  return errcode;
}

// --------------------------------------------------------------------

/* python call format: undefine to use global variables instead of arguments */
#define USE_ARGS

kvPythonInterpreter::kvPythonInterpreter()
{
  if (!Py_IsInitialized())
    Py_Initialize();

  PyObject* g = PyDict_New();
  PyDict_SetItemString(g, "__builtins__", PyEval_GetBuiltins());

  char* bootstrap =
    "def missing(x): return x is None\n"
    "class Dispatcher:\n"
    "  def __init__(self):\n"
    "    self.cache = {}\n"
    "  def register(self, name, script):\n"
    "    # print 'CALL register(%r, %r)' % (name, script[:40])\n"
#if defined(USE_ARGS)
    "    context = dict(missing=missing)\n"
    "    exec compile(script, name + '.py', 'exec') in context\n"
    "    self.cache[name] = context['check']\n"
    "    return\n"
#else
    "    self.cache[name] = compile(script, name + '.py', 'exec')\n"
#endif
    "  def is_registered(self, name):\n"
    "    # print 'CALL is_registered(%r) -> %d' % (name, name in self.cache)\n"
    "    return name in self.cache\n"
    "  def dispatch(self, name, data):\n"
	"    # print 'CALL dispatch(%r, %r)' % (name, data)\n"
#if defined(USE_ARGS)
    "    result = dict()\n"
    "    self.cache[name](result, **data)\n"
    "    return result\n"
#else
    "    context = dict(missing=missing)\n"
    "    context.update(data)\n"
    "    exec self.cache[name] in context\n"
    "    result = dict()\n"
    "    context['check'](result)\n"
    "    return result\n"
#endif
    "dispatcher = Dispatcher()\n"
    ;

  PyRun_String(bootstrap, Py_file_input, g, NULL);

  dispatcher = PyDict_GetItemString(g, "dispatcher");

  if (PyErr_Occurred()) {
    PythonMethod method(this, NULL, "");
    method.error();
  }
}


kvPythonInterpreter::~kvPythonInterpreter()
{
  Py_XDECREF(dispatcher);
  dispatcher = NULL;
  if (Py_IsInitialized())
    Py_Finalize();
}

int kvPythonInterpreter::is_registered(const std::string& name)
{
  /* check if a script with the given name is registered.  this method
     may return false if the name is missing, or if it needs to be re-
     loaded for some reason (e.g. because it's too old) */

  PythonMethod method(this, "is_registered", name);

  if (!dispatcher)
    return method.error();

  PyObject* res;
  res = PyObject_CallMethod(dispatcher, "is_registered", "s", name.c_str());
  if (!res)
    return method.error();

  int status = PyObject_IsTrue(res);

  Py_DECREF(res);

  if (status < 0)
    return method.error();

  return status;
}

kvPythonInterpreter * kvPythonInterpreter::getInterpreter()
{
  if (!my_interpreter)
    my_interpreter = new kvPythonInterpreter();
  return my_interpreter;
}

int kvPythonInterpreter::register_script(const std::string & name, const std::string &script)
{
  /* registers a script with the Python runtime.  to avoid extra work,
     it's recommended to call is_registered before calling this method.*/

  PythonMethod method(this, "register", name);

  if (!dispatcher)
    return method.error();

  PyObject* res;
  res = PyObject_CallMethod(dispatcher, "register", "ss",
			    name.c_str(), script.c_str());
  if (!res)
    return method.error(-2); /* compile time error in user script */

  Py_DECREF(res);
  return 0;
}

int kvPythonInterpreter::dispatch(const std::string& name,
				const miutil::miTime& obstime,
				const std::list<kvQABase::script_var>& data,
				std::map<std::string, double>& retvalues)
{
  /* calls the dispatcher with a set of script variables.  the result
     is collected in a property map. */

  PythonMethod method(this, "dispatch", name);

  if (!dispatcher)
    return method.error();

  PyObject* dict = PyDict_New();
  if (!dict)
    return method.error();

  typedef std::list<kvQABase::script_var>::const_iterator script_var_iter;

  PyObject* obj = open_list(dict, "obstime");
  add_int(obj, obstime.year());
  add_int(obj, obstime.month());
  add_int(obj, obstime.day());
  add_int(obj, obstime.hour());
  add_int(obj, obstime.min());
  add_int(obj, obstime.sec());
  close_list(obj);

  for (script_var_iter var = data.begin(); var != data.end(); var++)
    add_script_var(dict, *var);

  if (PyErr_Occurred()) {
    Py_DECREF(dict);
    return method.error();
  }

  /* call dispatch method */
  PyObject* res;
  res = PyObject_CallMethod(dispatcher, "dispatch", "sO", name.c_str(), dict);
  Py_DECREF(dict);
  if (!res)
    return method.error(-3); /* runtime error in user script */

  /* copy items from Python dictionary to map.  the dictionary must
     contain (string, number) pairs, where number is either float or
     int */
  PyObject *key, *value; Py_ssize_t pos = 0;
  while (PyDict_Next(res, &pos, &key, &value)) {
    if (PyString_Check(key)) {
      if (PyFloat_Check(value))
	retvalues[PyString_AS_STRING(key)] = PyFloat_AS_DOUBLE(value);
      else if (PyInt_Check(value))
	retvalues[PyString_AS_STRING(key)] = (double) PyInt_AS_LONG(value);
      /* else ignore */
    }
  }

  Py_DECREF(res);

  return 0;
}

void kvPythonInterpreter::freeInterpreter()
{
  if (my_interpreter)
    delete my_interpreter;
  my_interpreter = NULL;
}
#endif