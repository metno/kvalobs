/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: Layout.h,v 1.1.2.2 2007/09/27 09:02:31 paule Exp $                                                       

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
#ifndef __milog_layout_h__
#define __milog_layout_h__

#include <string>
#include <milog/milogtypes.h>

namespace milog{

  /**
   * \addtogroup milog
   *
   * @{
   */
  /**
   * \brief Baseclass to use to implement a new layout class.
   *
   * A layout class format the log message.
   */
  class Layout{

  public:
    Layout();
    virtual ~Layout();

    /**
     * \brief Format the message \em msg.
     *
     * This is an virtual function tha all layout classes must implement.
     * The message to format is given with \em msg the threshold loglevel 
     * is given with \em ll and the context stack is given as a string
     * on the form /context1/context2/../contextN.
     *
     * The function must return the logmessage to be used.
     *
     * \param msg The log message.
     * \param ll The log level.
     * \param context The context string.
     * \return The formatted log message.
     */
    virtual std::string formatMessage(const std::string &msg, 
				      LogLevel ll,
				      const std::string &context)=0;
  };

  /** @} */
}


#endif
