/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: comobsentry.cc,v 1.1.2.1 2007/09/27 09:02:24 paule Exp $

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

#ifndef __KVPARAMDEFS_H__
#define __KVPARAMDEFS_H__

namespace kvalobs {
namespace decoder {
namespace bufr {
namespace paramid {

enum {
  IR = 9,   ///< Exists precip. treshold ?
  IX = 10,  ///< Exists weather treshold ?
  HL = 55,  ///< Ceiling height
  VV = 273,  ///< visibility
  NN = 15,  ///< cloud cover
  DD = 61,  ///< degree
  FF = 81,  ///< knot/ms
  TA = 211,  ///< Celsius
  TD = 217,  ///< Celsius
  PR = 178,  ///< mb
  PO = 173,  ///< mb
  AA = 1,   ///< pressure charakteristics
  PP = 177,  ///< Tendency
  ITR = 12,  ///< precipitation indikator  [ setPreciptiation() ]
  WW = 41,  ///< Weather at obs.time
  W1 = 42,  ///< Weather 1  before obs.time
  W2 = 43,  ///< Weather 2  before obs.time
  WAWA = 49,  ///< Auto-Weather at obs.time
  WA1 = 47,  ///< Auto-weather 1  before obs.time
  WA2 = 48,  ///< Auto-weather 2  before obs.time
  NH = 14,  ///< Amount of low/medium clouds
  CL = 23,  ///< Type (low clouds)
  CM = 24,  ///< Type (medium clouds)
  CH = 22,  ///< Type (high clouds)
  UU = 262,  ///< Relative humidity
  MDIR = 403,  ///< Direction of the ship  deg
  MSPEED = 404,  ///< Velocity of the ship   m/s
  TW = 242,  ///< Water Temperature
  PWA = 154,  ///< Wave period
  HWA = 134,  ///< Wave height 0.5 m
  HwaHwaHwa = 134,  ///< Wave height 0.1 m
  PW = 151,  ///< Wave period
  HW = 131,  ///< Wave height 0.5 m
  DW1 = 65,
  DW2 = 66,
  PW1 = 152,
  HW1 = 132,
  PW2 = 153,
  HW2 = 133,
  XIS = 11,
  ES = 101,
  RS = 17,
  CI = 4,
  SI = 20,
  BI = 2,
  DI = 6,
  ZI = 21,
  TAX_12 = 216,  ///< Max.temp. (day)
  TAX = 215,
  TAN = 213,
  TAN_12 = 214,  ///< minimum temp. (night)
  TGN_12 = 224,  ///< Min temp. (grass kl.07)
  E = 129,   ///< soil condition
  EM = 7,   ///< soil condition in snow E'
  SA = 112,  ///< Snow depth in cm
  NS1 = 25,  ///< Cloud amount  lvl 1
  NS2 = 26,  ///< Cloud amount  lvl 2
  NS3 = 27,  ///< Cloud amount  lvl 3
  NS4 = 28,  ///< Cloud amount  lvl 4
  CC1 = 305,  ///< Cloud type    lvl 1
  CC2 = 306,  ///< Cloud type    lvl 2
  CC3 = 307,  ///< Cloud type    lvl 3
  CC4 = 308,  ///< Cloud type    lvl 4
  HS1 = 301,  ///< Cloud height  lvl 1
  HS2 = 302,  ///< Cloud height  lvl 2
  HS3 = 303,  ///< Cloud height  lvl 3
  HS4 = 304,  ///< Cloud height  lvl 4
  FG = 83,  ///< Max gust
  FG_010 = 84,
  FG_1 = 90,
  FG_6 = 91,
  FG_12 = 92,
  DG_010 = 77,
  DG_1 = 78,
  DG_6 = 79,
  DG_12 = 80,
  EV_24 = 103,  ///< evaporation or evapotransp.
  OT_24 = 122,  ///< daily hours of sunshine
  SG = 19,  ///< Sea
  TZ = 13,  ///< time of last max middle wind
  FX = 86,  ///< Maks. mid. wind since last obs
  FX_1 = 87,
  FX_3 = 95,
  FX_6 = 88,
  FX_12 = 89,
  DX_1 = 73,
  DX_3 = 74,
  DX_6 = 75,
  DX_12 = 76,
  X1WD = 44,  ///< Weather amend. since last obs
  X2WD = 45,  ///< Weather amend. since last obs
  X3WD = 46,  ///< Weather amend. since last obs
  RR_1 = 106,
  RR_3 = 107,
  RR_6 = 108,
  RR_12 = 109,
  RR_24 = 110,  ///< RR24
  SD = 18  ///< Snow cover extracted from soil condition (E_)
};

const char*
paramName(int paramid);

}
}
}
}

#endif /* KVPARAMID_H_ */
