/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: milogtypes.h,v 1.1.2.2 2007/09/27 09:02:31 paule Exp $                                                       

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
#ifndef __milog_milogtypes_h__
#define __milog_milogtypes_h__
#ifdef SMHI_LOG
/* Common defines for new FDLogStream*/

#define SECOND 1
#define MINUTE 2
#define HOUR 3
#define DAY 4
#define DEFAULT_DAY_FORMAT "%Y-%m-%d"
#define DEFAULT_HOUR_FORMAT "%Y-%m-%d-%H"
#define DEFAULT_MINUTE_FORMAT "%Y-%m-%d-%H-%M"
#define DEFAULT_SECOND_FORMAT "%Y-%m-%d-%H-%M-%S"
#endif
namespace milog {

  /**
   * \addtogroup milog
   * @{
   */

  /**
   * \brief The logging levels, ie the thresholds value
   */
  typedef enum{FATAL, ERROR, WARN, INFO, DEBUG, DEBUG1,
	       DEBUG2, DEBUG3, DEBUG4, DEBUG5, DEBUG6, NOTSET} LogLevel;
  /** @} */
}

#endif
