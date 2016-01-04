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

#ifndef SCRIPT_H_
#define SCRIPT_H_

#include "language/Interpreter.h"
#include "ScriptInput.h"
#include <string>
#include <vector>

/**
 * Namespace for generic script creation and -running functionalty.
 *
 * \ingroup group_scriptrunner
 */
namespace scriptrunner {
/**
 * \defgroup group_scriptrunner Script running
 *
 * Generic functions and classes for performing the actual creation and
 * running of scripts.
 *
 * The most important classes here are Script and language::Interpreter.
 */

/**
 * Abstraction of a script, independent of language
 *
 * \ingroup group_scriptrunner
 */
class Script {
 public:
  /**
   * Instatiate script with the given base script (in a specific language),
   * and the interpreter to use with it.
   *
   * \param baseScript Basic language-speccific definition of the script to run.
   * \param interpreter The language interpreter to use with this script.
   */
  Script(const std::string & baseScript,
         const language::Interpreter::Ptr & interpreter);

  ~Script();

  typedef ScriptInput Input;

  /**
   * Add a set of input variables to this script. These variables may
   * neither be modified nor removed after they have been added to *this.
   *
   * \note a Script may have several Input objects attached.
   */
  void addInput(const Input & input) {
    input_.push_back(input);
  }

  /**
   * Get the complete list of input sets.
   */
  const std::vector<Input> & getInput() const {
    return input_;
  }

  /**
   * Execute *this.
   *
   * \returns the result of having run *this with the given data.
   */
  RunResult run() const;

  /**
   * Get a this script in it's native form, as a string.
   */
  std::string str() const;

  /**
   * Get the base of the script, without any input variables.
   */
  std::string baseScript() const {
    return baseScript_;
  }

 private:
  const std::string baseScript_;
  std::vector<Input> input_;
  language::Interpreter::Ptr interpreter_;
};

}

#endif /* SCRIPT_H_ */
