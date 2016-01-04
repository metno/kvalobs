/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: cmprspace.h,v 1.1.2.2 2007/09/27 09:02:32 paule Exp $                                                       

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
#ifndef __cmprspace_BM314_h__
#define __cmprspace_BM314_h__

#include <string>
namespace miutil {
/**
 * \addtogroup pu_miutil
 * @{
 */
/**
 * \brief cmprspace komprimerer alle space slik at de bare opptar
 * en space. 
 * 
 * Dersom det er space før \n fjernes disse. Dersom
 * det er space i starten fjernes disse, dette gjelder også alle space før 
 * \n (newline). TAB og CR erstattes med SPACE.
 * Som space regnes SPACE, TAB og CR. Ved retur vil buf kun bestå av
 * ord separert med kun en SPACE og eventuelt \n (newline).
 *
 * Eks.
 *    "dette er  en \t  string\tmed space     \n  hei"
 *    blir komprimert til. "dette er en string med space\nhei"
 */

void
cmprspace(std::string &buf, bool newlineAsSpace = false);
/** @} */

}
#endif
