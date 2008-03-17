/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: trimstr.h,v 1.1.2.2 2007/09/27 09:02:32 paule Exp $                                                       

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
#ifndef __TRIMSTR_BM314_H__
#define __TRIMSTR_BM314_H__

#include <string>

namespace miutil{

  /**
   * \addtogroup pu_miutil
   * @{
   */
typedef enum{TRIMFRONT, TRIMBACK, TRIMBOTH}ETrimStrWhere;

/**
 * \brief trimstr trimmer en streng for space i begynnelsen
 * og slutten avhenging av parameteren where. 
 *
 * Default trimmes både begynnelse å slutt. Hva som regnes som 
 * space er konfigurerbart.
 *
 * \param str   strengen som skal trimmes.
 * \param where er en enum som angir hvor det skal trimmes.
 *              TRIMFRONT i begynnelsen.
 *              TRIMBACK  på slutten.
 *              TRIMBOTH  både i begynnelsen og på slutten.
 * \param trimset angir hvilke tegn som skal regnes som space.
 *                default trimset er (0x20), tab (0x09), CR (0x0d) 
 *                og NL (0x0A). 
 */

void
trimstr(std::string &str, ETrimStrWhere where=TRIMBOTH, 
	const char *trimset=" \t\r\n");

/** @} */ 
}
#endif
