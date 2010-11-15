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

#include "interpreterRegistration.h"
#include <map>
#include <stdexcept>

namespace scriptrunner
{
namespace language
{
namespace internal
{

namespace
{
Interpreter * getNothing()
{
	return 0;
}

class InterpreterControl
{
public:
	InterpreterControl() : f_(getNothing) {}
	explicit InterpreterControl(InterpreterCreationFunction f) : f_(f) {}

	const Interpreter::Ptr get()
	{
		if ( ! p_ )
			p_ = Interpreter::Ptr(f_());
		return p_;
	}

private:
	InterpreterCreationFunction f_;
	Interpreter::Ptr p_;
};


typedef std::map<std::string, InterpreterControl> InterpreterList;
InterpreterList & getInterpreterList()
{
	static InterpreterList * availableInterpreters = 0;
	if ( ! availableInterpreters )
		availableInterpreters = new InterpreterList;
	return * availableInterpreters;

}
}

std::string registerInterpreter(const std::string & name, InterpreterCreationFunction interpreterCreator)
{
	InterpreterList & availableInterpreters = getInterpreterList();

	if ( availableInterpreters.find(name) != availableInterpreters.end() )
		throw std::logic_error("Multiple registrations of same interpreter name");

	InterpreterControl ic(interpreterCreator);
	availableInterpreters[name] = ic;
	return name;
}

const Interpreter::Ptr getInterpreter(const std::string & name)
{
	InterpreterList & availableInterpreters = getInterpreterList();

	InterpreterList::iterator interpreter = availableInterpreters.find(name);
	if ( interpreter == availableInterpreters.end() )
		return Interpreter::Ptr();
	return interpreter->second.get();
}

}
}
}
