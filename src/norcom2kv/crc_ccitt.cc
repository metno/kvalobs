/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: crc_ccitt.cc,v 1.1.6.1 2007/09/27 09:02:37 paule Exp $                                                       

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
#include "crc_ccitt.h"


namespace {
/* Tabeller med "magiske" verdier for ccitt funksjon */
  unsigned int ccitt_h[] = 
    {
      0x0000,
      0x1081,
      0x2102,
      0x3183,
      0x4204,
      0x5285,
      0x6306,
      0x7387,
      0x8408,
      0x9489,
      0xa50a,
      0xb58b,
      0xc60c,
      0xd68d,
      0xe70e,
      0xf78f,
    };

  unsigned int ccitt_l[] =
    {
      0x0000,
      0x1189,
      0x2312,
      0x329b,
      0x4624,
      0x57ad,
      0x6536,
      0x74bf,
      0x8c48,
      0x9dc1,
      0xaf5a,
      0xbed3,
      0xca6c,
      0xdbe5,
      0xe97e,
      0xf8f7,
    };
}


unsigned int  
crc_ccitt(const char *buf)
{
    unsigned int   n, crc;
    char *in=const_cast<char*>(buf);

    crc = 0;

    while (*in)
    {
	n = *in++ ^ crc;

	/* Henter verdier fra tabellene ccitt_l og ccitt_h */
	crc = ccitt_l[n&0x0f] ^ ccitt_h[(n>>4)&0x0f] ^ (crc>>8); 
    };
    
    return (unsigned short)crc;
}

unsigned int  
crc_ccitt(const std::string &buf)
{
  unsigned int   n, crc;
  std::string::const_iterator it=buf.begin();
  
  crc = 0;
  
  while (it!=buf.end()){
    n = *it ^ crc;
    
    /* Henter verdier fra tabellene ccitt_l og ccitt_h */
    crc = ccitt_l[n&0x0f] ^ ccitt_h[(n>>4)&0x0f] ^ (crc>>8); 
    it++;
  };
    
  return (unsigned short)crc;
}
