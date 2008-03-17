/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: synopElements.h,v 1.10.2.3 2007/09/27 09:02:18 paule Exp $                                                       

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


/** \brief Created by DNMI/FoU/PU: j.schulze@met.no
     at Fri Dec 28 10:19:04 2001 */

/*! Do not use the syn namespace! There are too much
 *  variables which could corrupt other parts of
 *  your program. This header is created as a kind
 *  of synop-index! use syn::h instead of h or "h"
 *  to pick the h-parameter from your synop!
 *  REMARK: parameter = 0 means the parameter is not defined in the KVALOBS base
 *  on encountering one of those triggers means that the complete Synop goes 
 *  to the text table
 */
namespace syn {
  
  static const int kvalobsTypeID = 1;
  static const int shipTypeID    = 11;


  /// contains all extra information in the message
  /// that isn't defined as a parameter (like time etc.)

  typedef struct info { 
    int YY;                 ///< obs day 
    int GG;		    ///< obs hour
    int IIiii;              ///< wmo-nr
    int A1BwNb;             ///< ships ident-nr
    int Qc;                 ///< Quadrant of the Globe 
    int MMM;                ///< marsden square
    int UlaUlo;
    int h0h0h0h0;
    int im;
    int GGgg;               ///< diff obs/send
    std::string callSign;   ///< literal SHIP/MOBIL or call sign

    info() : YY(0), GG(0), IIiii(0), A1BwNb(0),  Qc(0),
	     MMM(0),  UlaUlo(0), h0h0h0h0(0), im(0), GGgg(0) {}
  };
 
  typedef enum windunit { MS, KT, NOWINDUNIT };

  /// PARSE ELEMENT
  typedef enum synopTokenType { 
    NORMALTOKEN,
    ICETEXTTOKEN,
    ICINGTEXTTOKEN
  };

  /// PARSE ELEMENT
  typedef enum synopSection {
    S000,
    S111,
    S222,
    S333,
    S444,
    S555,
    S999     ///< this section doesn't exist for the wmo,
             ///< but is used in germany - ignored
  };
  
  /// PARSE ELEMENT
  typedef enum synopType {
    SYNOP,
    SHIP,
    MOBIL
  };

  /// SIGNS: 
  typedef enum signtype {
    sn,        ///< +- (not stored! as is)
    sw,        ///< +- and wet bulb type
    ss,        ///< +- and SST type
  };

  // SECTION 000
  // ALL -----------------
  //  YYGG;      in struct info
  //  iw;        wind unit (kt/ms etc) not stored! allways transformed into m/s

  // SYNOP --------------- 
  // IIiii;     in struct info

  // SHIP ----------------
   
  // A1BwNb;    in struct info          

  // SHIP / MOBIL --------

  static const int LaLaLa    = 401; ///< latitude  
  static const int LoLoLoLo  = 402; ///< longitude 

  // Qc;        in struct info      

  // MOBIL ---------------

  // MMM;       in struct info
  // UlaUlo;    in struct info
  // h0h0h0h0;  in struct info
  // im;        in struct info
  

  /// SECTION 111 ---------
  
  /// GGgg;      in struct info

  static const int ir       = 9;   ///< Exists precip. treshold ?
  static const int ix       = 10;  ///< Exists weather treshold ?
  static const int h        = 55;  ///< Ceiling height
  static const int VV       = 273; ///< visibility
  static const int N        = 15;  ///< cloud cover
  static const int dd       = 61;  ///< degree
  static const int ff       = 81;  ///< knot/ms
  static const int TTT      = 211; ///< Celsius
  static const int TdTdTd   = 217; ///< Celsius
  static const int PPPP     = 178; ///< mb
  static const int P0P0P0P0 = 173; ///< mb
  static const int a        = 1;   ///< pressure charakteristics
  static const int ppp      = 177; ///< Tendency
  //    RRR;                            mm  [check function setPrecipitation() ]
  static const int tr       = 12;  ///< precipitation indikator  [ setPreciptiation() ]  
  static const int hhh      = 0;   ///< m  (height of a3)         : not used                  
  static const int a3       = 0;   ///< standard isobaric surface : not used   
 
  static const int ww       = 41;  ///< Weather at obs.time
  static const int W1       = 42;  ///< Weather 1  before obs.time
  static const int W2       = 43;  ///< Weather 2  before obs.time
  
  static const int wawa     = 49;  ///< Auto-Weather at obs.time
  static const int Wa1      = 47;  ///< Auto-weather 1  before obs.time
  static const int Wa2      = 48;  ///< Auto-weather 2  before obs.time

  static const int Nh       = 14;  ///< Amount of low/medium clouds
  static const int Cl       = 23;  ///< Type (low clouds)
  static const int Cm       = 24;  ///< Type (medium clouds)
  static const int Ch       = 22;  ///< Type (high clouds)
  static const int UUU      = 262; ///< Relative humidity

  /// SECTION 222 ---------

  static const int ds       = 403; ///< Direction of the ship  deg 
  static const int vs       = 404; ///< Velocity of the ship   m/s 
  static const int TwTwTw   = 242; ///< Water Temperature
  static const int TbTbTb   = 0;   ///< Wet bulb Temperature       Not in Norway
  static const int PwaPwa   = 154; ///< Wave period
  static const int HwaHwa   = 134; ///< Wave height 0.5 m       
  static const int HwaHwaHwa= 134; ///< Wave height 0.1 m         
  static const int PwPw     = 151; ///< Wave period
  static const int HwHw     = 131; ///< Wave height 0.5 m         
  static const int dw1dw1   = 65;
  static const int dw2dw2   = 66;
  static const int Pw1Pw1   = 152;
  static const int Hw1Hw1   = 132;
  static const int Pw2Pw2   = 153;
  static const int Hw2Hw2   = 133;
  static const int Is       = 11;
  static const int EsEs     = 101;
  static const int Rs       = 17;
  static const int Ci       = 4;
  static const int Si       = 20;
  static const int Bi       = 2;
  static const int Di       = 6;
  static const int Zi       = 21;
 

  /// SECTION 333 ---------

  static const int TxTxTx   = 216; ///< Max.temp. (day)
  static const int TnTnTn   = 214; ///< minimum temp. (night)
  static const int E        = 7;   ///< soil condition   
  static const int jjj      = 0;   ///< nat.dev.par. (not in Norway?)  
  static const int E_       = 7;   ///< soil condition in snow E'
  static const int sss      = 112; ///< Snow depth in cm

  static const int Ns1      = 25;  ///< Cloud amount  lvl 1
  static const int Ns2      = 26;  ///< Cloud amount  lvl 2
  static const int Ns3      = 27;  ///< Cloud amount  lvl 3
  static const int Ns4      = 28;  ///< Cloud amount  lvl 4

  static const int C1       = 305; ///< Cloud type    lvl 1
  static const int C2       = 306; ///< Cloud type    lvl 2
  static const int C3       = 307; ///< Cloud type    lvl 3
  static const int C4       = 308; ///< Cloud type    lvl 4
  
  static const int hshs1    = 301; ///< Cloud height  lvl 1
  static const int hshs2    = 302; ///< Cloud height  lvl 2
  static const int hshs3    = 303; ///< Cloud height  lvl 3
  static const int hshs4    = 304; ///< Cloud height  lvl 4

  static const int ff_911   = 83;  ///< Max gust                     
  static const int EEE      = 103; ///< evaporation or evapotransp.
  static const int ie       = 0;   ///< part of the above            
  static const int SSS      = 122; ///< daily hours of sunshine
  static const int FF24     = 0;   ///< 24 hour net radiation        
  
 
  /// SECTION 444 ---------
  
  /// (_ means ') > N_ = N' etc.    
  /// The whole section is not used in Norway

  static const int N_       = 0;
  static const int C_       = 0;
  static const int H_H_     = 0;
  static const int Ct       = 0;

  /// SECTION 555 ---------                 

  static const int S        = 19;  ///< Sea
  static const int tz       = 13;  ///< time of last max middle wind 
  static const int fxfx     = 86;  ///< Maks. mid. wind since last obs
  static const int Tx_Tx_   = 216; ///< Max temp. (night)             
  static const int Tn_Tn_   = 214; ///< Min temp. (day)               
  static const int TgTgTg   = 224; ///< Min temp. (grass kl.07)       
  static const int Rt       = 0;   ///< Precipitation (1/10)  

  /// wdwdwd

  static const int X1wd     = 44;  ///< Weather amend. since last obs
  static const int X2wd     = 45;  ///< Weather amend. since last obs
  static const int X3wd     = 46;  ///< Weather amend. since last obs

  static const int RR24     = 110; ///< RR24


  /// EXTRA --------------

  /// Parameters in this section are not part of the
  /// SYNOP directly, but are generated by the decoder
  /// based on observations  ....
  
  static const int SD        = 18; ///< Snow cover extracted from
                                   ///< Soil condition (E_)
  


};

#endif










