/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: synop.cc,v 1.1.6.1 2007/09/27 09:02:26 paule Exp $                                                       

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
#include <synop.h>
#include <math.h>
#include <sstream>

using namespace syn;

static const int zero  = int('0');
static const int undef = -99999;
static const int norway= 1;

static const string errorOrder     = "Order: ";
static const string errorToken     = "Token: ";
static const string errorType      = "Type: ";
static const string errorCallsign  = "Call: ";
static const string errorDouble    = "Double: ";
static const string errorSlash     = "Slash: ";
static const string errorUnknown   = "Unknown: ";

// General functions ------------------------------


void synop::addError(string errType, string errToken)
{
  if(!unrecognised.empty())
    unrecognised+=" | ";
  unrecognised+=errType + errToken;

}



// Parser functions -----------------------------------

// make a int out of any given token

int synop::readToken(const char *token, int a, int b)
{
  
  if(!b) {
    if(!a)
      return (*token == '/' ? undef : (*token -zero));
    else if ( a >= strlen(token))
      return undef;
    else
      return (*(token+a) == '/' ? undef : (*(token+a) -zero));
  }
  else {
    if(b+a > strlen(token) )
      return undef;
  }

  char buf[6];

  strncpy(buf,token+a,b);
  
  if(strchr(buf,'/'))
    return undef;
  
  return atoi(buf);

}

void synop::setToken(const string par, const char* token, int a, int b)
{
  float tmp = readToken(token,a,b);

  obs[par] = tmp;

}


void synop::scale(float &buf,float s)
{
  if(buf > undef)
    buf*=s;
}
 
int synop::sign(const char s,const string stype)
{
  int res = s-zero;
  
  if(stype == sn )
    return ( res ? -1 : 1);

  obs[stype] = res;

  if(res < 2 )
    return ( res ? -1 : 1);


  if(stype == sw ) {
    switch(res) {
    case 5:
      return  1;
    case 6:
      return -1;
    }
    return 1;
  }
  
  return (res%2 ? -1 : 1);

}

int synop::checkCountry()
{
  if(obs.count(IIiii))
    return int(obs[IIiii]) / 1000;
  return 0;
}

void synop::checkOrder(const char* token) {
  
  int g = readToken(token);
  if (g < group)
    addError(errorOrder,token);
  if (g == group)
    if( section == S333 ) {
      if(group != 5 && group != 8 && group != 9)
	addError(errorDouble,token);
    }
    else
      addError(errorDouble,token);

  group = g;  
}

/// common read functions

void synop::setTemperature(const char* token,const string par,
			   const string signt)
{
  float tmp = readToken(token,1,3);
  
  if(tmp > undef ) 
    tmp*=sign(*token,signt);
 
  scale(tmp,0.1);
  
  
  obs[par] = tmp;

}

void synop::setWind(const string par, const char* token, int a, int b)
{
  float tff = readToken(token,a,b);

  if (wunit == KT )
    tff*= 0.5144; // knots 2 m/s ;

  obs[par] = tff;
  
}

void synop::setPressure(const char* token,const string par)
{
  float tmp = readToken(token,0,4);
  
  scale(tmp,0.1);
  
  if(tmp < 900 )
    tmp+= 1000;
  
  obs[par] = tmp;

}


/// 1st step parser functions

void synop::setType(const char *token)
{
  tokenType = NORMALTOKEN;
  raw+=token;

  if (*token == 'A')
    type = SYNOP;
  else 
    type = ((*token == 'B' ) ? SHIP : MOBIL);
}


void synop::fetchSection(const char *token)
{
  tokenType = NORMALTOKEN;

  int sec = readToken(token);
  
  if(sec > 1 )
    raw+=token;

  element=0;
  group = -1;

  synopSection s = section;

  switch( sec ) {
  case 1:
    section = S111;
    break;
  case 2:
    section = S222;
    setShipDirectionAndVelocity(token);
    break;
  case 3:
    section = S333;
    break;
  case 4:
    section = S444;
    break;
  case 5:
    section = S555;
    break;
  }
  
  if(section < s) addError(errorOrder,token);

}


/// usually used for unrecognised tokens

void synop::setText(const char *token)
{
  raw+=token;
  raw+=" ";
  

  if(section == S555 && checkCountry() != norway)
    return;


  if(tokenType == ICETEXTTOKEN) 
    icetext+=string(" ")+token;
  else if (tokenType == ICINGTEXTTOKEN)
    icingtext+=string(" ")+token;
  else {
    if ( section == S000 && element == 1 && type != SYNOP){
      element++;
      callSign = token;
      if(strlen(token) > 9 || strlen(token) < 3 )
	addError(errorCallsign,token);
    }
    else
      addError(errorToken,token); 
  } 
}


// sort in tokens ......

void synop::sortToken(bool lastToken,const char *token)
{
  raw+=token;
  element++;

  tokenType = NORMALTOKEN;

  if(readToken(token) == undef && section != S111 ){
    addError(errorSlash,token);
    return;
  }
  
  switch(section) {
  case S000:
    sort000Token(token);
    break;
  case S111:
    sort111Token(token); 
    break;
  case S222:
    sort222Token(token);
    break;
  case S333:
    sort333Token(token);
    break;
  case S444:
    sort444Token(token);
    break;
  case S555:
    sort555Token(token);
  }
}
// The famous ICE plain text tokens

void synop::setIceToken(const char *token)
{
  raw+=token;
  tokenType = NORMALTOKEN;

  setToken(Ci,token);
  setToken(Si,token+1);
  setToken(Bi,token+2);
  setToken(Di,token+3);
  setToken(Zi,token+4);

}

void synop::setIceText(synopTokenType stt, const char *token)
{
  raw+=token;
  tokenType = stt;
  if(tokenType == ICETEXTTOKEN)
    icetext=token;
  else
    icingtext=token;

}



// PARSER: Section specific sorting

// SECTION 0 ----------------------------------


void synop::setYYGGiw(const char* token)
{
  setToken(YY,token,0,2);
  setToken(GG,token,2,2);
  setToken(iw,token,4,0);

  int tmp = int(obs[iw]);
 
  if( tmp < 0 || tmp > 4 )
    addError("windunit: ",token);
  else 
    wunit = ( tmp < 3 ? MS : KT );  

}

// Section varies between synoptype;


void synop::sort000Token(const char *token)
{
  if(type == SYNOP)
    sortSynop000Token(token);
  else
    sortShipMobil000Token(token);
}

void synop::sortSynop000Token(const char *token)
{
  switch(element) {
  case 1:
    setYYGGiw(token);
    break;
  case 2:
    setToken(IIiii,token,0,5);
    fetchSection("111");
    break;
  }
}

void synop::sortShipMobil000Token(const char *token)
{
  switch(element) {
  case 1:
    setToken(A1BwNb,token,0,5);
    break;
  case 2:
    setYYGGiw(token);
    break;
  case 3:
    setToken(LaLaLa,token,2,3);
    break;
  case 4:
    setToken(Qc,token);
    setToken(LoLoLoLo,token,1,4);
    if(type == SHIP )
      fetchSection("111");
    break;
  case 5:
    setToken(MMM,token,0,3);
    setToken(UlaUlo,token,3,2);
    break;
  case 6:
    setToken(h0h0h0h0,token,0,4);
    setToken(im,token,4);
    fetchSection("111");
    break;
  }
}


// SECTION 1 ----------------------------------

// first 2 groups er defined (iRixhVV / Nddff)
// after that the groups 0...9 starts 

void synop::sort111Token(const char *token)
{
 
  int tst;
  
  if ( element > 2 ) {
    checkOrder(token);
    
    switch(group){
    case 0:
      setWind(ff,token,2,3);     // ff > 99 
      break;
    case 1:
      setTemperature(token+1,TTT); 
      break;
    case 2:
      if(*(token+1) != '9')
	setTemperature(token+1,TdTdTd);
      else
	setToken(UUU,token,2,3);
      break;
    case 3:
      setPressure(token+1,P0P0P0P0);
      break;
    case 4:
      tst = readToken(token+1);
      if(tst == 0 || tst == 9 )
	setPressure(token+1,PPPP);
      else {
	setToken(a3,token,1);
	setToken(hhh,token,2,3);
      }
      break;
    case 5:
      setToken(a,token+1);
      setToken(ppp,token,2,3);
      break;
    case 6:
      setToken(RRR,token,1,3);
      setToken(tr,token,4);
      break;
    case 7:
      setToken(ww,token,1,2);
      setToken(W1,token,3);
      setToken(W2,token,4);
      break;
    case 8:
      setToken(Nh,token,1);
      setToken(Cl,token,2);
      setToken(Cm,token,3);
      setToken(Ch,token,4);
      break;
    case 9:
      setToken(GGgg,token,1,4);      
      break;
    case undef:
      addError(errorSlash,token);
      break;
    }
  }
  else {
    switch(element) {
    case 1:
      setToken(ir,token);
      setToken(ix,token,1,0);
      setToken(h ,token,2,0);
      setToken(VV,token,3,2);
      break;
    case 2:
      setToken(N ,token);
      setToken(dd,token,1,2);
      scale(obs[dd],10);
      setWind(ff,token,3,2);
      break;
    }
  }

}


// SECTION 2 ----------------------------------

void synop::sort222Token(const char *token)
{ 
  checkOrder(token);

  switch(group){
  case 0:
    setTemperature(token+1,TwTwTw,ss);
    break;
  case 1: 
    setToken(PwaPwa,token,1,2);
    setToken(HwaHwa,token,3,2);
    scale(obs[HwaHwa],0.5);
    break;
  case 2:
    setToken(PwPw,token,1,2);
    setToken(HwHw,token,3,2);
    break;
  case 3:
    setToken(dw1dw1,token,1,2);
    setToken(dw2dw2,token,3,2);
    break;
  case 4:
    setToken(Pw1Pw1,token,1,2);
    setToken(Hw1Hw1,token,3,2);
    break;
  case 5:  
    setToken(Pw2Pw2,token,1,2);
    setToken(Hw2Hw2,token,3,2);
    break;
  case 6:
    setToken(Is,token+1);
    setToken(EsEs,token,2,2);
    setToken(Rs,token+4);
    break;
  case 7:
    setToken(HwaHwaHwa,token,2,3);
    scale(obs[HwaHwaHwa],0.1);
    break;
  case 8:
    setTemperature(token+1,TbTbTb,sw);
    break;
  }

}

void synop::setShipDirectionAndVelocity(const char *token)
{
  setToken(ds,token,3);
  setToken(vs,token,4);

  if(type != SHIP ) 
    if(obs[vs] > undef )  
      addError(errorType,token);
}

// SECTION 3 ----------------------------------

void synop::setSupplementaryJJ(const char* token)
{
  int j1   = readToken(token+1);
  int j2   = readToken(token+2);
  int j234 = readToken(token,2,3);
 
  if(!sjj) {
    if ( j1 < 3 ) {
      setToken(EEE,token,1,3);
      setToken(ie, token,4,1);
    }
    else if ( j1 == 5 ) {
      if ( j2 < 3 ) {
	setToken(SSS,token,2,3);
      }
      else {
	switch(j234) {
	case 407:
	  sjj = j234;
	  break;
	case 408:
	  sjj = j234;
	  break;
	case 507:
	  sjj = j234;
	  break;
	case 508:
	  sjj = j234;
	  break;
	}
      }
    }
  }
  else {
    switch(sjj) {
    case 507:
      setToken(FF24,token,0,5);
      break;
    }
    sjj = 0;
  }

}
void synop::setExtra333(const char* token)
{
  /// this function does nothing at this
  /// point and is just added for future expansion
  /// the 80000 (0....) (1....) group is not used
  /// in norway now 

  return;
}

void synop::setSupplementarySP(const char* token)
{
  // For now, this function just adds the max gust
  // from spsp ... to be expanded

  int Sp = readToken(token,0,2);

  switch(Sp) {
  case 11:
    setWind(ff_911,token,2,2);
    break;
  }

}

void synop::setExtraClouds(const char* token)
{
  ostringstream ost;

  if(clCounter++ > 3 ){
    addError("Clouds",token);
    return;
  }
  if(clCounter)
    ost << clCounter;
   
  setToken(Ns+ost.str()  ,token,1);
  setToken(C+ost.str()   ,token,2);
  setToken(hshs+ost.str(),token,3,2);

}

void synop::sort333Token(const char *token)
{
  if(extra333) {
    setExtra333(token);
    return;
  }
  
  if(sjj) { 
    setSupplementaryJJ(token);
    return;
  }
    
  checkOrder(token);

  switch(group){
  case 0:
    break;
  case 1:
    setTemperature(token+1,TxTxTx);
    break;
  case 2:
    setTemperature(token+1,TnTnTn);
    break;
  case 3:
    setToken(E,token,1);
    setToken(jjj,token,2,3);
    break;
  case 4:
    setToken(E_,token,1);
    setToken(sss,token,2,3);
    break;
  case 5:
    setSupplementaryJJ(token);
    break;
  case 6: 
    if(!obs.count(tr)) {
      setToken(RRR,token,1,3);
      setToken(tr,token,4);
    } /// just if this does not exist in S111
    break;
  case 7:
    setToken(RR24,token,1,4);
    scale(obs[RR24],0.1);
    break;
  case 8:
    if(!readToken(token,1,4))
      extra333 = true;
    else {
      setExtraClouds(token);
    }
    break;
  case 9:
    setSupplementarySP(token+1);
    break;
  }
}


// SECTION 4 ----------------------------------

/// 444 just contains one group N'C'H'H'Ct

void synop::sort444Token(const char *token)
{
  setToken(N_,token);
  setToken(C_,token,1);
  setToken(H_H_,token,2,2);
  setToken(Ct,token,4);
}

// SECTION 5 ----------------------------------

// This Group contains nationally developed groups 
// and is tuned for norwegian conditions
// at this time there are just 4 possible groups
// in Norway (to be expanded..) 

void synop::sort555Token(const char *token)
{
  if(checkCountry() != norway)
    return;
  cout << "Country: " << checkCountry() << endl;

   checkOrder(token);
   switch(group){
   case 0:
     setToken(S,token,1);
     setToken(tz,token,2);
     setWind(fxfx,token,3,2);
     break;
   case 1:
     setTemperature(token+1,Tx_Tx_);
     break;
   case 2:
     setTemperature(token+1,Tn_Tn_);
     break;
   case 3:
     setTemperature(token+1,TgTgTg);
     break;
   case 4:
     setToken(Rt,token,1);
     setToken(wdwdwd,token,2,3);
     break;
   default:
     addError(errorUnknown,token);
     break;
   }
   

}

// OUTPUT -------------------------------------- 

ostream& operator<<(ostream& out, const synop& rhs){
  out << rhs.raw;
  
 return out;
};

string synop::index()
{
  map<string,float>::iterator itr= obs.begin();
  
  ostringstream tmp;
  
  for(;itr!=obs.end(); itr++) 
    tmp << itr->first << ":\t" << itr->second << endl;

  return tmp.str();
}
  
bool synop::value(const string par,float& val)
{
  if(obs.count(par)) {
    val = obs[par];
    return true;
  }
  
  val = undef;
  return false;
}

bool synop::value(const string par,int& val)
{
  if(obs.count(par)) {
    val = int(obs[par]);
    return true;
  }
  
  val = undef;
  return false;
}






