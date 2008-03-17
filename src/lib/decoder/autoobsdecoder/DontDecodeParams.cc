/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: DontDecodeParams.cc,v 1.5.2.4 2007/09/27 09:02:24 paule Exp $                                                       

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
#include "convert.h"

/*
 * This file define a list of parameteres that shall not be decoded in the function
 * DataConvert::decodeParam in the file convert.cc
 */


namespace kvalobs{
  namespace decoder{
    namespace autoobs{
      
      NameDef DontDecodeParamArray[]={{"DD_02"},
				      {"EV_24"},
				      {"OT_24"},
				      {"OT_1"},
				      {"RR_01"},
				      {"RR_03"},
				      {"RR_06"},
				      {"RR_1"},
				      {"RR_2"},
				      {"RR_3"},
				      {"RR_6"},
				      {"RR_9"},
				      {"RR_12"},
				      {"RR_15"},
				      {"RR_18"},
				      {"RR_24"},
				      {"RR_X"},
				      {"RT_24"},
				      {"SS_24"},
				      {"TAN_12"},
				      {"TAX_12"},
				      {"TGN_12"},
				      {"WA_01"},
				      {"WA_15"},
				      {"WA_60"},
				      {"FG_10"},
				      {"FG_010"},
				      {"FG_1"},
				      {"FG_6"},
				      {"FG_X"},
				      {"DG_010"},
				      {"DG_1"},
				      {"DG_6"},
				      {"DG_12"},
				      {"FX_1"},
				      {"FX_3"},
				      {"FX_6"},
				      {"FX_12"},
				      {"DX_1"},
				      {"DX_3"},
				      {"DX_6"},
				      {"DX_12"},
				      {"X1WD"},
				      {"X2WD"},
				      {"X3WD"},
				      {0}};
      
    }
  }
}
