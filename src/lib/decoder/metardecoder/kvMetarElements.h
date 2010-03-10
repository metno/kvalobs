/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvMetarElements.h,v 1.2.2.2 2007/09/27 09:02:37 paule Exp $                                                       

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
#ifndef _kvMetarElements_h
#define _kvMetarElements_h

#include <puTools/miString.h>

/// Created by met.no/FoU/PU: j.schulze@met.no
/// at Tue Apr  1 11:28:21 2003

/// Do not use the kmet namespace! There are too much
/// variables which could corrupt with other parts of
/// your program. This header is created as a
/// METAR parameterindex! use kmet::h instead of h

/// REMARK: parameter = 0 means the parameter is not defined in the KVALOBS base
/// on encountering one og those triggers that the complete Synop goes to the text
/// table

namespace kmet {
  
  static const int kvalobsType = 2;

  
  typedef struct obsbuf { 
    int   par;
    int   lvl; 
    float val;
  };

  typedef struct txtbuf { 
    int              par;
    miutil::miString val;
  };

  static const int MESS    = 1001; ///< The message itself ...               

  /// WIND

  static const int DD      = 61;   ///< Dir  (deg )                          
  static const int FF      = 81;   ///< FF   (KT-> m/s )                     
  static const int FG_10   = 84;   ///< Gust (KT-> m/s )                     
  static const int DVRBDN  = 69;   ///< Var. winddir, lower limit            
  static const int DVRBDX  = 70;   ///< Var. winddir, upper limit            

  /// VISIBILITY

  static const int VV      = 273;  ///< Visibility                           
  static const int DVV     = 71;   ///< direction of  VV                     
  static const int VVX     = 275;  ///< Visib. best direction                
  static const int DVX     = 72;   ///< direction of VVX                     

  /// WEATHER

  static const int WWB     = 1041; ///< Signifikant weather, TXT             
  static const int WWCAVOK = 1042; ///< Cloud and visibillity OK (boolean )  
 
  /// TEMPERATURE

  static const int TA      = 211;  ///< Air Temperature                      
  static const int TD      = 217;  ///< Dew point Temperature                

  /// DIVERSE

  static const int PH      = 172;  ///< Air pressure (reduced to NN )        
  static const int HL      = 55;   ///< Vertikal Visib  from hft to m

  /// CLOUD AMOUNT  SKC -> 0, FEW -> 2, SCT -> 4, BKN -> 6 og OVC -> 8.

  static const int NS1     = 25;   ///< 1. cloud amount
  static const int NS2     = 26;   ///< 2. cloud amount
  static const int NS3     = 27;   ///< 3. cloud amount
  static const int NS4     = 28;   ///< 4. cloud amount

  /// CLOUD HEIGHT

  static const int HS1     = 301;  ///< 1. cloud height
  static const int HS2     = 302;  ///< 2. cloud height
  static const int HS3     = 303;  ///< 3. cloud height
  static const int HS4     = 304;  ///< 4. cloud height


  /// CBTYPE : TCU -> 8, CB -> 9

  static const int CC1      = 305; ///< 1. cloud CBTYPE 
  static const int CC2      = 306; ///< 2. cloud CBTYPE  
  static const int CC3      = 307; ///< 3. cloud CBTYPE
  static const int CC4      = 308; ///< 4. cloud CBTYPE

};
#endif










