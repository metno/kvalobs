/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: metadata.h,v 1.1.2.2 2007/09/27 09:02:27 paule Exp $                                                       

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
#ifndef __miutil_parsers_meta_metadata_h__
#define __miutil_parsers_meta_metadata_h__

#include <map>
#include <vector>
#include <list>
#include <string.h>
#include <sstream>
#include <stdexcept> //out_of_range


namespace miutil{
  namespace parsers{
    namespace meta{
 
      class BadType : public std::exception{
	std::string reason;
      public:
	  explicit BadType(const std::string &reason_)
	      : reason(reason_){}

	  virtual ~BadType()throw(){};
	  
	  const char *what()const throw()
	      { return reason.c_str();}
      };

      class Values{
	typedef std::vector<std::string>                 Value;
	typedef std::vector<std::string>::iterator       IValue;
	typedef std::vector<std::string>::const_iterator CIValue;

	Value value;

	friend class Meta;
	friend class MetaParser;

	Values(const Value &vals)
	  : value(vals)
	{
	}

	/**
	 *
	 * \exception std::out_of_range
	 */
	void checkRange(int index)const;

      public:
	Values(){};
	
	/**
	 * Add a value to the values.
	 *
	 * The first value added get the index 0, the second get the index 2, etc.
	 *
	 * \param value The value to add to the values.
	 */
	void addValue(const std::string &value);

	/**
	 * Get the value as a string.
	 *
	 * \return value at index as string.
	 * \exception std::out_of_range
	 */
	std::string getValue(int index=0)const ;

	/**
	 * Get the value as a int.
	 *
	 * \return value at index as int.
	 * \exception BadType, std::out_of_range
	 */
	int         getValueAsInt(int index=0)const;

	/**
	 * Get the value as a long.
	 *
	 * \return value at index as long.
	 * \exception BadType, std::out_of_range
	 */
	long        getValueAsLong(int index=0)const;
	
	/**
	 * Get the value as a float.
	 *
	 * \return value at index as float.
	 * \exception BadType, std::out_of_range
	 */
	float       getValueAsFloat(int index=0)const;

	/**
	 * Get the value as a double.
	 *
	 * \return value at index as double.
	 * \exception BadType, std::out_of_range
	 */
	double      getValueAsDouble(int index=0)const;


	/**
	 * Subscript access to a value as a string.
	 *
	 * \exception std::out_of_range
	 */
	std::string operator[] (int n) const{ return getValue(n);}

	int size()const { return value.size();}	

	friend std::ostream& operator<<(std::ostream& out,
					const miutil::parsers::meta::Values& vals);

      };

      std::ostream& operator<<(std::ostream& out,
			       const miutil::parsers::meta::Values& vals);

	  
      class Meta{
	typedef std::map<std::string, Values>                 ParamMap;
	typedef std::map<std::string, Values>::iterator       IParamMap;
	typedef std::map<std::string, Values>::const_iterator CIParamMap;

	friend class MetaParser;

	ParamMap params;

      public:
	Meta();
	Meta(const Meta &meta):params(meta.params){};
	
	~Meta();

	typedef std::list<std::string>                 Params;
	typedef std::list<std::string>::iterator       IParams;
	typedef std::list<std::string>::const_iterator CIParams;
	

	/**
	 * Add a value to the values of param.
	 */
	void  addParamValue(const std::string &param, 
			    const std::string &value);

	/**
	 * defalult values may be given as a list of values separated by |.
	 *
	 * Eks. To give a default valuelist of 1 and 2 -> "1|2".
	 */
	Values getParamValue(const std::string &param,
			     const std::string &defaultvalues="")const;

	
	/**
	 * Get a list of all params specified.
	 */
	Params getParams()const;

	friend std::ostream& operator<<(std::ostream& out,
					const Meta& meta);
	
	Meta& operator=(const Meta &rhs);
      };


      std::ostream& operator<<(std::ostream& out,
			       const miutil::parsers::meta::Meta& meta);

      class MetaParserImpl;

      class MetaParser{

      protected:
	
	MetaParserImpl *impl;
	
	friend class MetaParserImpl;

	std::ostringstream errMsg;
	std::ostringstream warnMsg;
	Meta               *meta;

      public:
	MetaParser();
	~MetaParser();

	std::string getErrMsg()const{ return errMsg.str();}
	std::string getWarnings()const{ return warnMsg.str();}

	Meta *parse(const std::string &xml2parse);
      };
    }
  }
}

#endif
