/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: miTimeParse.h,v 1.1.2.2 2007/09/27 09:02:32 paule Exp $                                                       

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
#ifndef __MITIMEPARSE_H__
#define __MITIMEPARSE_H__

#include <exception>
#include <puTools/miTime.h>

namespace miutil{
	
	class miTimeParseException : std::exception{
		std::string reason;
		std::string::size_type ipos_;
		bool                   informat_;
    public:
	  	explicit miTimeParseException(const std::string &reason_,
	  								  std::string::size_type ipos,
	  								  bool informat)
	      : reason(reason_), ipos_(ipos), informat_(informat){}

		virtual ~miTimeParseException() throw(){};
	  
	  	const char *what()const throw()
	      			{ return reason.c_str();}
	      			
	    std::string::size_type ipos()const{ return ipos_;}
	    bool informat()const{ return informat_;}
	};
	

	/**
 	 * %Y year 4 digit, ex 2006.
 	 * %y year 2 digit, ex 06.
 	 * %m month (01-12)
 	 * %d day (01-31)
 	 * %H hour (00-23)
 	 * %M minute (00-59)
 	 * %S second (00-60), valid for leap second. 
 	 * 
 	 * @exception miTimeParseException
 	 */
	std::string::size_type 
	miTimeParse( const std::string &format, 
				 const std::string &stringToParse,
				 miTime &time,
				 const miTime &nearestToThisTime=miTime());
	
   /**
    * Decode an time string with microseconds. Valid time string format
    * YYYY-MM-DD hh:mm:ss[.mmmmmm] [±HHMM]
    * YYYY-MM-DDThh:mm:ss[.mmmmmm] [±HHMM]
    *
    * Where the parts in [] is optional parts.
    *
    * The returned time is in UTC.
    * @param timespec the timestring to decode.
    * @param[out] msec The microsecond part.
    * @return A UTC time.
    */
	miutil::miTime isoTimeWithMsec( const std::string &timespec, int &msec );

}

#endif /*MITIMEPARSE_H_*/
