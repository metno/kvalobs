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

#include "PerlInterpreter.h"
#include <scriptrunner/language/interpreterRegistration.h>
#include "kvPerlParser.h"
#include <scriptrunner/Script.h>
#include <fstream>
#include <sstream>
#include <cstdlib>

#include <EXTERN.h>
#include <perl.h>

namespace
{
PerlInterpreter * my_perl = 0;

void initPerl()
{
	my_perl = perl_alloc();
	//PERL_SET_CONTEXT( my_perl ); // should not be necessary
	perl_construct(my_perl);
	PL_exit_flags |= PERL_EXIT_DESTRUCT_END;
}
void freePerl()
{
	if (!my_perl)
		return;

	perl_destruct(my_perl);
	perl_free(my_perl);

	my_perl = 0;
}

}

namespace scriptrunner
{

namespace language
{

namespace perl
{

PerlInterpreter::PerlInterpreter()
{
}

PerlInterpreter::~PerlInterpreter()
{
}

RunResult PerlInterpreter::run(const Script & s) const
{
	kvPerlParser scriptRunner;
	std::map<std::string, double> scriptReturn;

	bool ok = scriptRunner.runScript(fullScript(s), scriptReturn);

	return RunResult(not ok, scriptReturn);
}

std::string PerlInterpreter::fullScript(const Script & script) const
{
	std::ostringstream ss;

	ss << "use strict;\n\n";

	for ( std::vector<Script::Input>::const_iterator it = script.getInput().begin(); it != script.getInput().end(); ++ it )
	{
		ss << "# " << it->name() << '\n';
		numericDeclarations(ss, * it);
		stringDeclarations(ss, * it);
		numericListDeclarations(ss, * it);
		stringListDeclarations(ss, * it);
		ss << "\n";
	}
	ss << "\n" << script.baseScript() << std::endl;
	return ss.str();
}

std::ostream & PerlInterpreter::numericDeclarations(std::ostream & s, const Script::Input & input) const
{
	for ( Script::Input::NumericParameters::const_iterator it = input.numeric().begin(); it != input.numeric().end(); ++ it )
		s << "my $" << it->first << " = " << it->second << ";\n";
	return s;
}

std::ostream & PerlInterpreter::stringDeclarations(std::ostream & s, const Script::Input & input) const
{
	for ( Script::Input::StringParameters::const_iterator it = input.strings().begin(); it != input.strings().end(); ++ it )
		s << "my $" << it->first << " = " << quote(it->second) << ";\n";
	return s;
}

std::ostream & PerlInterpreter::numericListDeclarations(std::ostream & s, const Script::Input & input) const
{
	for ( Script::Input::NumericListParameters::const_iterator it = input.numericList().begin(); it != input.numericList().end(); ++ it )
	{
		s << "my @" << it->first << " = (";
		const Script::Input::ValueList & valueList = it->second;
		if ( not valueList.empty() )
		{
			s << valueList.front();
			for ( unsigned i = 1; i < valueList.size(); ++ i )
				s << ", " << valueList[i];
		}
		s << ");\n";
	}
	return s;
}

std::ostream & PerlInterpreter::stringListDeclarations(std::ostream & s, const Script::Input & input) const
{
	for ( Script::Input::StringListParameters::const_iterator it = input.stringList().begin(); it != input.stringList().end(); ++ it )
	{
		s << "my @" << it->first << " = (";
		const Script::Input::StringList & stringList = it->second;
		if ( not stringList.empty() )
		{
			s << '"' << stringList.front() << '"';
			for ( unsigned i = 1; i < stringList.size(); ++ i )
				s << ", \"" << stringList[i] << '"';
		}
		s << ");\n";
	}
	return s;
}

std::string PerlInterpreter::quote(const std::string & s) const
{
	// todo escape aswell
	return '"' + s + '"';
}

namespace
{
Interpreter * getPerlInterpreter()
{
	return new PerlInterpreter;
}
std::string registration = internal::registerInterpreter("perl", getPerlInterpreter);
}

}

}

}
