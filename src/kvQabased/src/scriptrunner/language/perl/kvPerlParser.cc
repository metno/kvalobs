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
#include "kvPerlParser.h"

#include <milog/milog.h>
#include <iostream>
#include <EXTERN.h>
#include <perl.h>

namespace scriptrunner
{
namespace language
{
namespace perl
{

interpreter * kvPerlParser::my_perl = 0;

kvPerlParser::kvPerlParser()
{
	if (!my_perl)
		initPerl();
}

kvPerlParser::~kvPerlParser()
{
	if (my_perl)
		freePerl();
}

void kvPerlParser::initPerl()
{
	//std::clog << "Initialising PERL..." << std::endl;
	my_perl = perl_alloc();
	//PERL_SET_CONTEXT( my_perl ); // should not be necessary
	perl_construct(my_perl);
	PL_exit_flags |= PERL_EXIT_DESTRUCT_END;
	//std::clog << "...PERL initialised" << std::endl;
}

void kvPerlParser::freePerl()
{
	if (!my_perl)
		return;

	//std::clog << "killing PERL..." << std::endl;
	perl_destruct(my_perl);
	perl_free(my_perl);

	my_perl = 0;
	//std::clog << "...PERL killed" << std::endl;
}

bool kvPerlParser::runScript(const std::string& script, std::map<std::string,
		double>& params)
{
	//std::clog << "kvPerlParser::runScript start" << std::endl;
	bool result = false;
	bool parse_errors = false;
	int res;

	int numa = 3;
	//   char *thescript= (char*) malloc(sizeof(char)*(script.length()+1));
	//   strcpy(thescript, script.c_str());

	const char *embedding[] =
	{ "", "-e", script.c_str() };
	//   char *embedding[] = { "", "-e", thescript };

	//std::clog << "  Start PERL parsing " << std::endl;

	if (!my_perl)
	{
		//IDLOGWARN("html", "WARNING: PERL not initialised" << std::endl);
		initPerl();
	}

	//IDLOGDEBUG("html", "before perl_parse: "<<	std::endl <<embedding[2]);

	res	= perl_parse(my_perl, NULL, numa, const_cast<char **> (embedding),
					NULL );
	if (res != 0)
	{
		std::string errstr;
		//     STRLEN n_a;
		//     IDLOGERROR("html"," kvPerlParser::ERROR from Perl-parse:"
		//         << SvPV(ERRSV, n_a) << std::endl);
		//     IDLOGERROR("html"," kvPerlParser::ERROR from Perl-parse:"
		//         << errstr << std::endl);
		LOGERROR("Parse error");
		parse_errors = true;
		return false;
	}

	PL_exit_flags |= PERL_EXIT_DESTRUCT_END;
	res = perl_run(my_perl);
	if (res != 0)
	{
		LOGERROR("Script run error");
		parse_errors = true;
		return false;
	}

	//std::clog << "  End PERL parsing" << std::endl;
	dSP; /* initialize stack pointer      */
	ENTER; /* everything created after here */
	SAVETMPS; /* ...is a temporary variable.   */
	PUSHMARK( SP ); /* remember the stack pointer    */

	/* NO INPUT ARGUMENTS FOR NOW */
	// push arguments on the stack
	//XPUSHs(sv_2mortal(newSViv(flag))); // flag
	//   XPUSHs(sv_2mortal(newSViv(numvars))); // number of variables
	//   for (int i=0; i<numvars; i++){ // name/value pairs
	//     XPUSHs(sv_2mortal(newSVnv(varvals[i])));     // nv = double
	//     XPUSHs(sv_2mortal(newSVpv(varnames[i], 0))); // pv = string
	//     // andre argument=strenglengden, 0 betyr at Perl bruker strlen
	//   }

	PUTBACK; // make local stack pointer global


	//std::clog << "  Calling perl-routine.." << std::endl << std::endl;
	int count = call_pv( "check", G_EVAL | G_ARRAY ); // call the function
	//std::clog << std::endl << "  ..finished, count=" << count << std::endl;

	SPAGAIN; // refresh stack pointer

	params.clear(); // clear the parameter-hash

	/* Check the eval first */
	if (SvTRUE( ERRSV ))
	{
		//STRLEN n_a;
		//std::clog << " kvPerlParser::ERROR from Perl-evaluation:"	<< SvPV( ERRSV, n_a ) << std::endl;
		//     printf ("  Uh oh - %s\n", SvPV(ERRSV, n_a)) ;
		POPs;

	}
	else
	{

		/* check number of return-arguments */
		if (count > 1)
		{
			/* pop any return values from stack */
			int numreturns = POPi;
			//std::clog << "  Number of return-values:" << numreturns << std::endl;

			/* the stack should now contain "numreturns/2" name/value-pairs */
			if (numreturns > count)
			{
				//std::clog << "ERROR: too few arguments on perl-stack"	<< std::endl;

			}
			else if (numreturns % 2 != 0)
			{
				//std::clog << "ERROR: odd number of arguments on perl-stack" << std::endl;

			}
			else
			{
				numreturns = numreturns / 2;

				for (int i = 0; i < numreturns; i++)
				{
					double value = POPn;
					SV* svname = POPs; // get name first as a SV-pointer
					std::string name = SvPV_nolen( svname ); // convert to char*
					//std::clog << "  Got:" << name << " = " << value<< std::endl;

					params[name] = value;
				}
				result = true;
			}

		}
		else
		{
			//std::clog << "Only " << count << " argument(s) on perl-stack," << " exit normally" << std::endl;
			result = true;
		}

	}

	PUTBACK;
	FREETMPS; // free that return value
	LEAVE; // ...and the XPUSHed "mortal" args.

	//   free(thescript);

	//std::clog << "--runScript:: Ending" << std::endl;

	return result;
}

}
}
}
