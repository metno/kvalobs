/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: NameConvertDef.cc,v 1.5.6.3 2007/09/27 09:02:24 paule Exp $                                                       

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

namespace kvalobs{
  namespace decoder{
    namespace autoobs{

      NameConvertDef nameConvertDefArray[]={{"RR",   "RR_1"},
					    {"RT",   "RT_1"},
					    {"OT",   "OT_1"},
					    {"UUM",  "UM"},
					    {"RI",   "RR_01"},
					    {"FGti", "KLFG"},
					    {"FXti", "KLFX"},
					    {"FG",   "FG_1"},
					    {"FG_10","FG_010"},
					    {"FX",   "FX_1"},
					    {"DX",   "DX_1"},
					    {"$SIGN","signature"},
					    {0,     0}};

    }
  }
}
