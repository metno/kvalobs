/*
  Kvalobs - Free Quality Control Software for Meteorological Observations

  $Id: main.cc,v 1.4.2.9 2007/09/27 09:02:15 paule Exp $

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

#ifndef AGREGATORCONFIGURATION_H_
#define AGREGATORCONFIGURATION_H_

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <vector>
#include <iosfwd>

class AgregatorConfiguration
{
public:
	AgregatorConfiguration();
	AgregatorConfiguration(std::ostream & messageStream, std::ostream & errorStream);
	~AgregatorConfiguration();

	enum ParseResult
	{
		Exit_Success, Exit_Failure, No_Action
	};

	ParseResult parse(int & argc, char ** argv);

	std::ostream & version(std::ostream & s) const;
	std::ostream & help(std::ostream & s) const;

	bool backProduction() const;
	std::string backProductionSpec() const;

	bool daemonMode() const;
	bool runInDaemonMode() const { return backProduction() or daemonMode(); }

	const std::vector<int> & stations() const { return stations_; }
	const std::vector<int> & parameters() const { return parameters_; }

	std::string proxyDatabaseName() const;
	bool repopulateDatabase() const;

private:
	void setup_();

	std::ostream & messageStream_;
	std::ostream & errorStream_;

	/// Description of what options a user may give
	boost::program_options::options_description availableOptions_;

	/// The options that have actually been given. Only populated after parse have been called
	boost::program_options::variables_map givenOptions_;

	std::vector<int> stations_;
	std::vector<int> parameters_;
};

#endif /* AGREGATORCONFIGURATION_H_ */
