/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: copyfile.h,v 1.1.2.2 2007/09/27 09:02:28 paule Exp $                                                       

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
#ifndef __COPYFILE_BM314_H__
#define __COPYFILE_BM314_H__

#include <stdio.h>
#include <string>

namespace miutil {
namespace file {

/**
 * copyfile copys nByte from the file srcfd, starting from frompos, to
 * the file destfd. The first byte is copied to the position startpos. On
 * return the filepointer is pointing to the position after the last 
 * character written/read with repect on destfd and srcfd.
 *
 * \param srcfd Copy from this file
 * \param frompos The first caracter to copy from srcfd.
 * \param nByte Number of byte to copy from srcfd. 
 *        Use -1 to copy the rest of the file.
 * \param destfd Copy to this file.
 * \param startpos The first position to copy to in destfd.
 *
 * \return true ons success. false otherwise.
 *         The filepointers in destfd and srcfd is undefined on failure.
 */

bool
copyfile(FILE *srcfd, int frompos, int nByte, FILE *destfd, int startpos);

/**
 * copyfile copy fromfile to tofile. tofile is over written if it 
 * exist. The modification time is set to the same of from file if 
 * set_time is true.
 *
 * \param fromfile Copy from this file.
 * \param tofile copy to this file.
 * \param set_mtime Set the modificationtime of tofile to the same 
 *                   as from file.
 *
 * \return true on success and false otherwise.
 */
bool
copyfile(const std::string &fromfile, const std::string &tofile,
         bool set_mtime = false);

/**
 * copyfile copy fromfile to tofile. tofile is over written if it 
 * exist. The modification time is set to the same of from file if 
 * set_time is true.
 *
 * The copy is done with the use of a temporary filewith the name
 * tofile_SafeCopy_tmp.tmp. The file tofile_SafeCopy_tmp.tmp is 
 * overwritten if it exist. After the copy is complite the tmp file
 * is renamet to tofile.
 *
 * The ide is that in case of a system failure (crash) there will be 
 * no incomplete written tofile.
 *
 * \param fromfile copy from this file.
 * \param tofile copy to this file.
 * \param set_mtime Set the modificationtime of tofile to the same 
 *                   as from file.
 *
 * \return true on success and false otherwise.
 */
bool
safecopy(const std::string &fromfile, const std::string &tofile,
         bool set_mtime = false);
}
}

#endif
