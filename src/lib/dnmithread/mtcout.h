/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: mtcout.h,v 1.1.2.2 2007/09/27 09:02:33 paule Exp $                                                       

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
#ifndef __MTCOUT_H__
#define  __MTCOUT_H__

#include <iostream>
#include <boost/thread/mutex.hpp>

namespace dnmi{
  namespace thread{
    namespace impldetails {
      extern boost::mutex __dnmi_std_out_mutex;
    }
  }
}

/**
 * \addtogroup threadutil
 * @{
 */

/**
 * \brief A threadsafe macro to access the std::cout stream.
 */
#define COUT(msg) {                           \
                     boost::mutex::scoped_lock  __dnmi_COUT_lock__(dnmi::thread::impldetails::__dnmi_std_out_mutex);           \
                     std::cout << msg;      \
                  }
/**
 * \brief A threadsafe macro to access the std::cerr stream.
 */
#define CERR(msg) {                           \
                     boost::mutex::scoped_lock  __dnmi_CERR_lock__(dnmi::thread::impldetails::__dnmi_std_out_mutex);           \
                     std::cerr << msg;      \
                  }

/** @} */
#endif
