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

#ifndef RUNRESULT_H_
#define RUNRESULT_H_

#include <map>
#include <string>

namespace scriptrunner
{

/**
 * The return values after having run a script.
 *
 * \ingroup group_scriptrunner
 */
class RunResult
{
public:
	typedef std::map<std::string, double> RunReturn;

	RunResult(int exitCode, const RunReturn & returnValues);
	~RunResult();

	/**
	 * script's exit status
	 */
	int exitCode() const { return exitCode_; }

	operator bool () const { return exitCode() == 0; }

	/**
	 * Any return values, given as a set of name-value pairs
	 */
	const RunReturn & returnValues() const { return returnValues_; }

	std::string str() const;

private:
	int exitCode_;
	RunReturn returnValues_;
};

}

#endif /* RUNRESULT_H_ */
