/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: synop.h,v 1.1.6.1 2007/09/27 09:02:26 paule Exp $                                                       

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
#ifndef _synop_h
#define _synop_h

///  Synop class. Developed for KVALOBS by
///  Juergen Schulze / DNMI/ PU 12/2001
///  The Class builds on a lexer (synopLexer.lex)
///  and is able to read and understand plain
///  text synops.
///  Usage: synop S = lexSynop(string); 
///  check  synopLexer.h


#include <string>
#include <iostream>
#include <map>
#include <synopElements.h>

using namespace std;


class synop {
private:
  /// decoding parameters 

  int group;                     /// which group? 1xxxx 2xxxx etc.
  int element;                   /// where are we? while parsing
  int clCounter;                 /// which cloud group (333) 1-4

  syn::windunit wunit;           /// Kt or m/s
  syn::synopType type;           /// Ship, land etc.          
  syn::synopTokenType tokenType; /// to trick plain text sev. word tokens
  syn::synopSection section;     /// actual section while decoding
  int sjj;                       /// to check supp follow group
  bool extra333;                 /// to trick (80000 (0....) ...)
  string unrecognised;           /// error pool 

  string raw;                    /// original synop   
 
  /// SYNOP MEMBERS

 
  string callSign;               /// literal SHIP/MOBIL or call sign
  string icetext;                /// ICE   plain text
  string icingtext;              /// ICING plain text
  
  map<string,float> obs;         /// all data storage 


public:
  synop() : element(0), group(-1), type(syn::SYNOP), 
	    section(syn::S000), tokenType(syn::NORMALTOKEN), 
	    sjj(0), extra333(false), clCounter(-1) {}

  
  bool hasUnrecognised() { return !unrecognised.empty();}

  /// Output

  string Unrecognised() const { return unrecognised;}
  string Raw() const {return raw;}
  string index();               /// all elements with name
  bool value(const string,float&);
  bool value(const string,int&);

  friend ostream& operator<<(ostream&, const synop&);
 
  /// The external interface ends HERE!!!!!================================ 

  /// Parser functions (step1: external interface)
  /// accessed by synopLexer.lex
  /// don't try anything else! The parser expects clean
  /// strings which are provided by lex. A string 345G23
  /// will not chrash but will not give any reliable result!!!!
  /// The lexer has memory control over char *token

  void sortToken(bool,const char *);             /// set ([0-9]|\/){5}
  void setType(const char *);                    /// set AAXX/BBXX/OOXX
  void fetchSection(const char *);               /// set (111|222dv|...|555)
  void setText(const char *);                    /// set rest (error/call...)

 
  /// The famous ICE plain text tokens
  
  void setIceToken(const char *);                     /// ICE ([0-9]|\/){5}
  void setIceText(syn::synopTokenType,const char *);  /// ICE|ICING plain text

private:

  void addError(string,string);                  /// error:type/token


  /// SYNOP PARSER FUNCTIONS ( step 2: internal)


  int readToken(const char*, int pos=0, int len=0);  
                                                 /// parts of a token 
                                                 /// to be read in an 
                                                 /// integer
  void setToken(const string, const char*, 
		int pos=0, int len=0);           /// set it into map

  int  sign(const char,const string s=syn::sn);  /// sn/sw/ss: +/- (0/1) 
  void scale(float&,float s=1.0);                /// 333=>33.3 etc. 

  int  checkCountry();                           /// II
  void checkOrder(const char*);                  /// group order error?
  void setTemperature(const char*,const string,
		      const string s=syn::sn);   /// common temp func.
                                                 /// returns +-xx.x
  void setPressure(const char*,const string);    /// common press func.
                                                 /// 0xxx = 10xx.x 
                                                 /// 9xxx = 9xx.x
  void setWind(const string, const char*, 
	       int pos=0, int len=0);            /// set wind and
                                                 /// transform kt 2 m/s
                                                 /// if necessary
 
  /// SECTION 0
  void setYYGGiw(const char*);
  void sort000Token(const char *);
  void sortSynop000Token(const char *);
  void sortShipMobil000Token( const char *);

  /// SECTION 1
  void sort111Token(const char *);
   
  /// SECTION 2
  void sort222Token(const char *);
  void setShipDirectionAndVelocity(const char *);

  /// SECTION 3
  void sort333Token(const char *); 
  void setSupplementaryJJ(const char*);   /// to read the 
                                          /// j1234 56789 token
  void setExtra333(const char*);          /// to read 80000 (0....) ...
  void setSupplementarySP(const char*);   /// to read all SpSpspsp
  void setExtraClouds(const char*);       /// to read sev. cloud groups

  /// SECTION 4
  void sort444Token(const char *);
  
  /// SECTION 5
  void sort555Token(const char *);

};
  

#endif




