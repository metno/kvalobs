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

#ifndef INTERPRETERREGISTRATION_H_
#define INTERPRETERREGISTRATION_H_

#include "Interpreter.h"

namespace scriptrunner {
namespace language {

namespace internal {

/**
 * \addtogroup group_scriptrunner
 * \{
 *
 * \defgroup group_scriptrunner_registration Interpreter registration
 * \{
 */

typedef Interpreter * (*InterpreterCreationFunction)();

/**
 * Register a new interpreter, which may later be accessed by using the given name.
 *
 * @warning If you register multiple interpreters with the same name, the
 *          older ones will be removed.
 *
 * @see Interpreter and getInterpreter().
 *
 * @param name Interpreter identification string, which may later be used to
 *             get access to the interpreter.
 * @param interpreterCreator Pointer to a function which should return a
 *                           heap-allocated Interpreter object pointer. This
 *                           function will be called when needed.
 * @return The name that the interpreter is registered with.
 */
std::string registerInterpreter(const std::string & name,
                                InterpreterCreationFunction interpreterCreator);

/**
 * Get access to a prevously registered interpreter.
 *
 * @see Interpreter
 *
 * @param name The identification name that the wanted interpreter was
 *             registered with.
 * @return An interpreter (smart) pointer, or a NULL pointer if the name have
 *         not been registered.
 */
const Interpreter::Ptr getInterpreter(const std::string & name);

/**
 * \}
 * \}
 */

}
}
}

#endif /* INTERPRETERREGISTRATION_H_ */
