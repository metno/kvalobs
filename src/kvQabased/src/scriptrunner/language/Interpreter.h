/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 Copyright (C) 2010 met.no

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

#ifndef INTERPRETER_H_
#define INTERPRETER_H_

#include <scriptrunner/RunResult.h>
#include <boost/shared_ptr.hpp>
#include <string>
#include <exception>

namespace scriptrunner {
class Script;

namespace language {

/**
 * Implements what is language-dependent of a script. Subclasses know how to
 * generate and run scripts, based on Script objects.
 *
 * \ingroup group_scriptrunner
 */
class Interpreter {
 public:
  typedef boost::shared_ptr<Interpreter> Ptr;

  /**
   * Get a registered interpreter
   *
   * @see internal::registerInterpreter()
   *
   * @param interpreterName The name that the interpreter was registered with
   * @return Pointer to ai interpreter, or a NULL pointer if no such interpreter is available.
   */
  static Ptr get(const std::string & interpreterName);

  virtual ~Interpreter() {
  }

  /**
   * Base class for interpreter related exceptions
   */
  class Error;

  /**
   * Execute the given script.
   *
   * @throws Error, or a subclass of Error if unable to run script for any reason.
   */
  virtual RunResult run(const Script & s) const =0;

  /**
   * Get the string representation of the Script.
   */
  virtual std::string fullScript(const Script & s) const =0;

};

class Interpreter::Error : public std::exception {
  std::string msg_;
 public:
  Error(const std::string & msg)
      : msg_(msg) {
  }
  ~Error() throw () {
  }
  const char * what() const throw () {
    return msg_.c_str();
  }
};

}
}

#define INTERPRETER_EXCEPTION(Name) \
	class Name : public ::scriptrunner::language::Interpreter::Error { \
		public: \
		Name(const std::string & msg) : \
			::scriptrunner::language::Interpreter::Error(msg) \
		 {} \
	};

#endif /* INTERPRETER_H_ */
