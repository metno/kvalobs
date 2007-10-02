/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: synopElements.h,v 1.1.6.1 2007/09/27 09:02:26 paule Exp $                                                       

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
#ifndef _synopElements_h
#define _synopElements_h


/* Created by DNMI/FoU/PU: j.schulze@dnmi.no
   at Fri Dec 28 10:19:04 2001 */

// do not use the syn namespace! There are too much
// variables which could corrupt with other parts of
// your program. This header is created as a kind
// of synop-index! use syn::h instead of h or "h"
// to pick the h-parameter from your synop!


using namespace std; 

namespace syn {
  

  typedef enum windunit { MS, KT };

  typedef enum synopTokenType { 
    NORMALTOKEN,
    ICETEXTTOKEN,
    ICINGTEXTTOKEN,
  };

  typedef enum synopSection {
    S000,
    S111,
    S222,
    S333,
    S444,
    S555
  };
  
  typedef enum synopType {
    SYNOP,
    SHIP,
    MOBIL
  };

  /// SIGNS: 

  static const string sn="sn";        /// +- (not stored! as is)
  static const string sw="sw";        /// +- and wet bulb type
  static const string ss="ss";        /// +- and SST type


  /// SECTION 000

  /// all

  static const string YY="YY";         /// obs day
  static const string GG="GG";         /// obs hour
  static const string iw="iw";         /// wind unit (kt/ms etc)

  /// synop 

  static const string IIiii="IIiii";   /// synop nr.

  /// ship 
  
  static const string A1BwNb="A1BwNb"; /// ship ident numbers          

  /// ship / mobil
  
  static const string LaLaLa="LaLaLa";
  static const string LoLoLoLo="LoLoLoLo";
  static const string Qc="Qc";

  /// mobil

  static const string MMM="MMM";
  static const string UlaUlo="UlaUlo";
  static const string h0h0h0h0="h0h0h0h0";
  static const string im="im";
  

  
  /// SECTION 111
  static const string GGgg="GGgg";      /// diff obs/send

  static const string ir="ir";		/// Exists precip. treshold ?
  static const string ix="ix";		/// Exists weather treshold ?
  static const string h="h";		/// Ceiling height
  static const string VV="VV";		/// visibility
  static const string N="N";		/// cloud cover
  static const string dd="dd";		/// degree
  static const string ff="ff";		/// knot/ms
  static const string TTT="TTT";	/// Celsius
  static const string TdTdTd="TdTdTd";  /// Celsius
  static const string PPPP="PPPP";	/// mb
  static const string P0P0P0P0="P0P0P0P0"; /// 
  static const string a="a";		/// pressure tendency
  static const string ppp="ppp";	/// Tendency
  static const string RRR="RRR";	/// mm
  static const string tr="tr";		/// precipitation indikator
  static const string hhh="hhh";	/// m
  static const string a3="a3";          /// standard isobaric surface
  static const string ww="ww";		/// Weather at obs.time
  static const string W1="W1";		/// Weather 1 before obs.time
  static const string W2="W2";		/// Weather 2 before obs.time
  static const string Nh="Nh";		/// Amount of low/medium clouds
  static const string Cl="Cl";		/// Type (low clouds)
  static const string Cm="Cm";		/// Type (medium clouds)
  static const string Ch="Ch";		/// Type (high clouds)
  static const string UUU="UUU";        /// 

  /// SECTION 222
  static const string vs="vs";		/// Velocity of the ship
  static const string ds="ds";		/// Direction of the ship
  static const string TwTwTw="TwTwTw"; 	/// Water Temperature
  static const string TbTbTb="TbTbTb";  /// Wet bulb Temperature
  static const string PwaPwa="PwaPwa";  /// Wave period
  static const string HwaHwa="HwaHwa";  /// Wave height 0.5 m
  static const string HwaHwaHwa="HwaHwaHwa";/// Wave height 0.1 m
  static const string PwPw="PwPw";	/// Wave period
  static const string HwHw="HwHw";	/// Wave height 0.5 m
  static const string dw1dw1="dw1dw1";
  static const string dw2dw2="dw2dw2";
  static const string Pw1Pw1="Pw1Pw1";
  static const string Hw1Hw1="Hw1Hw1";
  static const string Pw2Pw2="Pw2Pw2";
  static const string Hw2Hw2="Hw2Hw2";
  static const string Is="Is";
  static const string EsEs="EsEs";
  static const string Rs="Rs";
  static const string Ci="Ci";
  static const string Si="Si";
  static const string Bi="bi";
  static const string Di="Di";
  static const string Zi="Zi";
 

  /// SECTION 333
  static const string TxTxTx="TxTxTx"; /// Max.temp. (day)
  static const string TnTnTn="TnTnTn"; /// minimum temp. (night)
  static const string E="E";           /// soil condition
  static const string jjj="jjj";       /// nat.dev.par. (not in Norway?)
  static const string E_="E_";	       /// soil condition in snow E'
  static const string sss="sss";       /// Snow depth in cm
  static const string Ns="Ns";         /// Cloud amount (spec. station)
  static const string C="C";	       /// Cloud type   (spec. station)
  static const string hshs="hshs";     /// Cloud height (spec. station)
  static const string ff_911="ff_911"; /// Max gust
  static const string EEE="EEE";       /// evaporation or evapotransp.
  static const string ie="ie";         /// part of the above

  static const string SSS="SSS";       /// daily hours of sunshine
  static const string FF24="FF24";     /// 24 hour net radiation 
  
 
  /// SECTION 444  (_ means ') > N_ = N' etc.

  static const string N_="N_";           
  static const string C_="C_";
  static const string H_H_="H_H_";
  static const string Ct="Ct";

  /// SECTION 555

  static const string S="S";	       /// Sea
  static const string tz="tz";	       /// time of last max middle wind 
  static const string fxfx="fxfx";     /// Maks. mid. wind since last obs
  static const string Tx_Tx_="Tx_Tx_"; /// Max temp. (night)
  static const string Tn_Tn_="Tn_Tn_"; /// Min temp. (day)
  static const string TgTgTg="TgTgTg"; /// Min temp. (grass kl.07)
  static const string Rt="Rt";	       /// Precipitation (1/10)
  static const string wdwdwd="wdwdwd"; /// Weather amend. since last obs
  static const string RR24="RR24";     /// RR24






};

#endif
