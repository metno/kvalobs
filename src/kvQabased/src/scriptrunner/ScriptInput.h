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

#ifndef SCRIPTINPUT_H_
#define SCRIPTINPUT_H_

#include <map>
#include <string>
#include <vector>

namespace scriptrunner
{

/**
 * A collection of input variables to a Script. Everything in this class must
 * be language independent.
 *
 * \ingroup group_scriptrunner
 */
class ScriptInput
{
public:
	typedef std::map<std::string, double> NumericParameters;
	typedef std::map<std::string, std::string> StringParameters;
	typedef std::vector<double> ValueList;
	typedef std::map<std::string, ValueList> NumericListParameters;
	typedef std::vector<std::string> StringList;
	typedef std::map<std::string, StringList> StringListParameters;

	ScriptInput();

	/**
	 * Construct with the given name as identifier. The name will be displayed
	 * in the script in relation to its parameters. Exactly how this is done
	 * is up to the interpreter.
	 */
	explicit ScriptInput(const std::string & name);

	~ScriptInput();

	/**
	 * Insert a parameter with a numeric value into the script
	 */
	void add(const std::string & name, double value);

	/**
	 * Insert a parameter with a string value into the script
	 */
	void add(const std::string & name, const std::string & value);

	/**
	 * Insert a parameter with a list of numeric value into the script
	 *
	 * \param name The parameter name
	 * \param start Start of iterator to get list from
	 * \param stop End of iterator to get list from
	 */
	template<class OutputIterator>
	void add(const std::string & name, OutputIterator start, OutputIterator stop)
	{
		numericList_[name] = ValueList(start, stop);
	}
	/**
	 * Insert a parameter with a list of string values into the script
	 *
	 * \param name The parameter name
	 * \param start Start of iterator to get list from
	 * \param stop End of iterator to get list from
	 */
	template<class OutputIterator>
	void adds(const std::string & name, OutputIterator start, OutputIterator stop)
	{
		stringList_[name] = StringList(start, stop);
	}

	const std::string & name() const { return name_; }
	const NumericParameters & numeric() const { return numeric_; }
	const StringParameters & strings() const { return strings_; }
	const NumericListParameters & numericList() const { return numericList_; }
	const StringListParameters & stringList() const { return stringList_; }

private:
	std::string name_;
	NumericParameters numeric_;
	StringParameters strings_;
	NumericListParameters numericList_;
	StringListParameters stringList_;
};

}

#endif /* SCRIPTINPUT_H_ */
