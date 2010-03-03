/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: checkrunner.h,v 1.1.2.5 2007/09/27 09:02:21 paule Exp $

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
#ifndef _checkrunner_h
#define _checkrunner_h

#include "CheckCreator.h"
#include "kvQABaseDBConnection.h"
#include "kvQABaseMeteodata.h"
#include "kvQABaseTypes.h"
#include <puTools/miString>
#include <puTools/miTime>
#include <kvalobs/kvStationInfo.h>
#include <milog/milog.h>
#include <kvalobs/kvChecks.h>
#include <boost/filesystem/path.hpp>

/*
 TODO:
 add a way to insert replacement (model) values after all checks have run.
 - How to:
 - Call a function after all checks have been run.
 - Loop trough all variables from check
 - if missing original value:
 - insert value, after some criteria (model value)
 - Implementation tightly coupled with QABase.
 - Use a perl script, fetched from database
 - Make sure the check is run last.
 - Loosely coupled missing substitution.
 - Easy for anyone to write a new algorithm
 - Possible problem: The check will need to know about corrected value (if it is missing)
 - Solved by giving fmis to the check.(?)
 */

/**
 * Controls check running for a single message.
 */
class CheckRunner
{
public:
	/**
	 * Constructor.
	 *
	 * @param params The message to be controlled
	 * @param con A database connection to the kvalobs database.
	 * @param logpath path for logging.
	 */
			CheckRunner(const kvalobs::kvStationInfo & params,
					kvQABaseDBConnection & con,
					const boost::filesystem::path & logpath);

	~CheckRunner();

	/**
	 * Set logpath (before first runchecks)
	 */
	void logpath(const boost::filesystem::path & logp)
	{
		logpath_ = logp;
	}

	/**
	 * Main routine. Runs all relevant checks for station
	 *
	 * @param forceCheck If true, run checks on data, even if it has been
	 * modified by HQC, or otherwise should not normally be checked.
	 *
	 * @throw std::exception of some sort on error.
	 */
	void operator()(bool forceCheck = false);

private:
	bool shouldProcess();
	bool hqcCorrected();
	bool dataWasCheckedBefore();
	void updateStaticVariables();
	void findChecks(std::list<kvalobs::kvChecks> & out);

	void runCheck(const CheckCreator::Script & checkScript,
			const kvalobs::kvChecks & check);

	CheckCreator checkCreator_;

	const kvalobs::kvStationInfo & stinfo;
	kvQABaseDBConnection dbcon;

	kvObsPgmList oprogramlist;
	kvQABaseMeteodata meteod; // Meteorological data manager

	/// Path to logfile(s)
	boost::filesystem::path logpath_;
	/// The next time the static tables should be updated.
	miutil::miTime updateStaticTime;

	milog::HtmlStream * openHTMLStream();
};

#endif
