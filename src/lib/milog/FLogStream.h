/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: FLogStream.h,v 1.1.2.2 2007/09/27 09:02:31 paule Exp $                                                       

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
#ifndef __milog_flogstream_h__
#define __milog_flogstream_h__

#include <stdio.h>
#include <string.h>
#include <milog/LogStream.h>


namespace milog{

  /**
   * \addtogroup milog
   * 
   * @{
   */

  /**
   * \brief A log stream to log to a file.
   *
   * Every FLogStream has two values that is used to controll
   * the diskspace a log file will occupie. The two values are
   *  - Maxsize
   *  - rotate.
   *
   * When a file has reached the \em maxsize it is renamed
   * to filename.1. This mean that \em rotate can never be smaller than
   * 1. If \em rotate is greater than 1, the file with the name filename.1 
   * will be renamed to filename.2 etc.
   */
    class FLogStream : public virtual milog::LogStream{
	std::string fname_;
	int         nRotate_;
	int         maxSize_;
	int         currentSize_;
	FILE        *fd;
	
      bool logRotate();      

    protected:
	virtual void write(const std::string &message);
	
    public:
	/**
	 * \brief Default contructor. Use StdLayout.
	 *
	 * Max filesize is 100 kB.
	 * Rotate logfiles with 1. Ie. there will only
	 * be two logFiles. 
	 *
	 * You must open a LogFile with the function 
	 * open(const std::string &fname).
	 */
	FLogStream();

	/**
	 * \brief Use StdLayout.
	 *
	 * You must open a LogFile with the function 
	 * open(const std::string &fname).
	 *
	 * \param nRotet Number of logfiles.
	 * \maxSize Set the maxsize for when we shall 
	 *          create a new logfile.
	 */
	FLogStream(int nRotate,
		   int maxSize=102400);

	/**
	 * \brief Create a FLogStream with a user specified Layout.
	 *
	 * You must open a LogFile with the function 
	 * open(const std::string &fname).
	 *
	 * \param layout The Layout to use. FLogStream take over
	 *        the ownership of the layout pointer.
	 *        
	 * \param nRotet Number of logfiles.
	 * \maxSize Set the maxsize for when we shall 
	 *          create a new logfile.
	 */
	FLogStream(Layout *layout, 
		   int nRotate=1,
		   int maxSize=102400);

	/**
	 * \brief Create a FLogStream with StdLayout.
	 *
	 * You must open a LogFile with the function 
	 * open(const std::string &fname).
	 *
	 * \param fname Use this as the logfile.
	 * \param nRotet Number of logfiles.
	 * \maxSize Set the maxsize for when we shall 
	 *          create a new logfile.
	 */
	FLogStream(const std::string &fname, 
		   int               nRotate=1,
		   int               maxSize=102400);
		  

 	virtual ~FLogStream();
	
	/**
	 * \brief open \em fname as the logfile.
	 *
	 * \param fname The log file.
	 * \return True on success, false otherwise.
	 */
	bool open(const std::string &fname);

	/**
	 * \brief Close this LogStream.
	 */
	void close();
    };
    
    /** @} */
};


#endif
