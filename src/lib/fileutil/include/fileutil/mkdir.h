/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: mkdir.h,v 1.1.2.2 2007/09/27 09:02:28 paule Exp $                                                       

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
#ifndef __DNMI_FILE_MKDIR_H__
#define __DNMI_FILE_MKDIR_H__

#include <string>

namespace dnmi{
  namespace file {
    
    /**
     * \addtogroup  fileutil
     *
     * @{
     */
	/** Creates a new directory.
	 * mkdir try to create a new directory in the directory given 
	 * with path. creates parrent directory as needed.
	 * 
	 * The directory given with path must exist. If path is not given
	 * current working directory is used.
	 *	
	 * If the newdir directory allready exist, true is returned.
	 * 
	 * It try to create the directory with the permisions owner (rwx), group (rwx),
	 * and other(r-x). But it is modified by the users umask.
	 * 
	 * \param newdir The new directory to create.
	 * \path  create the new directoryes in this dierectory.
	 * \return true if the newdir exist or was created. And false otherwise.
	 */
	bool mkdir(const std::string &newdir, const std::string &path=std::string());    
    
    /** @} */
  }
}

#endif
