/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: synop.cc,v 1.16.2.7 2007/09/27 09:02:18 paule Exp $                                                       

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
#include "synop.h"
#include <math.h>
#include <sstream>
#include <cstring>
#include <cstdlib>

using namespace syn;
using namespace std;

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

int synop::typeID()
{
  if(type==SHIP)
    return  shipTypeID;
  return   kvalobsTypeID;
}

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
      return (*token == '/' || *token == 'X' || *token == 'x' ? undef : (*token -zero));
    else if ( a >= strlen(token))
      return undef;
    else
      return (*(token+a) == '/' || *(token+a) == 'X' || *(token+a) == 'x' ? undef : (*(token+a) -zero));
  }
  else {
    if(b+a >  strlen(token))
      return undef;
  }

  char buf[6];
  buf[0]='\0';
  strncpy(buf,token+a,b);

  // strncpy does NOT 0-terminate the string!!!!!

  buf[b]='\0';


  if(strchr(buf,'/'))
    return undef;
 
  if(strchr(buf,'X'))
    return undef;
 
 if(strchr(buf,'x'))
    return undef;
 
 
  return atoi(buf);

}
void synop::setBuffer(const int par,float val)
{
  obs[par] = val;
}

void synop::setToken(const int par, const char* token, int a, int b)
{
  float val= readToken(token,a,b);
  setBuffer(par,val);
}

void synop::setStaticToken(int& par, const char* token, int a, int b)
{
  par= readToken(token,a,b);
}


void synop::setScaledToken(const int par, const char* token,float scl, int a, int b)
{
  float val= readToken(token,a,b);
  scale(val,scl);
  setBuffer(par,val);
}


// special case for klima-DB

void synop::setSnowToken(const int& par,const char* token, int a, int b)
{
  float val= readToken(token,a,b);
  int   ival = int(val);
  if(val == undef)
    val = -1;
  
  if ( ival == 997 )  val =  0;
  if ( ival == 998 )  val =  0;
  if ( ival == 999 )  val = -3;
    
  setBuffer(par,val);  
}

void synop::setScaledSubstituteToken(const int par, const char* token,float scl,
				     int fsub, int tosub, int a, int b)
{

  float val= readToken(token,a,b);
  if(int(val)==fsub) 
    val = float(tosub);
  else
    scale(val,scl);
  setBuffer(par,val);
}



bool synop::hasobs(int par)
{
  return bool(obs.count(par));
}


bool synop::value(int par,float& o)
{
  if(!obs.count(par))
    return false;
  o = obs[par];
  return true;
}

bool synop::value(int par,int& o)
{
  if(!obs.count(par))
    return false;
  o = int(obs[par]);
  return true;
}

void synop::scale(float &buf,float s)
{
  if(buf > undef)
    buf*=s;
}
 
int synop::sign(const char s,const signtype stype)
{
  int res = s-zero;
  
  if(stype == sn )
    return ( res ? -1 : 1);

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
	int II=pinfo.IIiii ? pinfo.IIiii  / 1000 : 0;
	
	if( II != 0 )
		return II;
	
	if( pinfo.callSign.length()>=3 ) {
		//Check if the callsign of the ship is for norway.
		//callsign for norway start with 3 character in the
		//following range JWA-JXZ, LAA-LNZ or 3YA-3YZ
		string cs = pinfo.callSign.substr( 0, 3 );
		
		if( (cs>="JWA" && cs<="JXZ") ||
			 (cs>="LAA" && cs<="LNZ") ||
			 (cs>="3YA" && cs<="3YZ")    )
			return 1;
	}
		
	return 0;	
}


void synop::squareDetection()
{
  if(hasobs(LoLoLoLo))
    if(pinfo.Qc > 4 )
      obs[LoLoLoLo]*=(-1);

  if(hasobs(LaLaLa))
    if(pinfo.Qc == 5 || pinfo.Qc == 3 )
      obs[LaLaLa]=(-1);  
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

void synop::setTemperature(const char* token,const int par,
			   const signtype signt)
{
  float tmp = readToken(token,1,3);
  
  if(tmp > undef ) 
    tmp*=sign(*token,signt);
 
  scale(tmp,0.1);
  
  setBuffer(par,tmp);
}

void synop::setWind(const int par, const char* token, int a, int b)
{
  if (wunit==NOWINDUNIT ) {
    addError("windunit: ",token);
    return;
  }
 
  float tff = readToken(token,a,b);
  
  if (wunit == KT )
    tff*= 0.5144; // knots 2 m/s ;
  
  setBuffer(par,tff);
}

void synop::setPressure(const char* token,const int par)
{
  float tmp = readToken(token,0,4);
  
  scale(tmp,0.1);
  
  if(tmp < 500 )
    tmp+= 1000;
  
  setBuffer(par,tmp);
}



void synop::setPrecipitation(const char* token)
{
  int tr_ =   readToken(token,4); // rrr interval indicator
  setBuffer(tr,float(tr_));


  int par=0; 

  switch ( tr_ ) {
  case 1:
    par = 108; // RR_06
    break;
  case 2:
    par = 109; // RR_12
    break;
  case 3:
    par = 109; // (18 not def. -> RR_12)
    break;
  case 4:
    par = 110; // RR_24
    break;
  case 5:
    par = 106; // RR_01
    break;
  case 6:
    par = 106; // (2  not def. -> RR_01)
    break;
  case 7:
    par = 107; // RR_03 
    break;
  case 8:
    par = 108; // (9  not def. -> RR_06)
    break;
  case 9:
    par = 109; // (15 not def. -> RR_12)
    break;
  }


  float RRR = readToken(token,1,3);

  int ir_ = 0;
  
  if(value(ir,ir_))
    if (ir_ == 3 )   // ir=3    -> no precip = -1 in klima-DB (old)
      RRR = -1;
 
  if (int(RRR) == 0) // RRR=000 -> no precip = -1 in klima-DB (new)
    RRR = -1;

  if (RRR >= 990 ) {
    RRR-=990;
    scale(RRR,0.1);
  }
  
  setBuffer(par,RRR);
}

// adding decimals of precipitation 
// ( 555 RT )
// postprocess 

void synop::setPrecipitationRt(const char* token)
{
 
  float fRt = readToken(token,1);
  scale(fRt,0.1);

  setBuffer(Rt,fRt);

  if(fRt > 0.4 )
    fRt-=1.0;

  addPrecipitation(106, fRt);// RR_01
  addPrecipitation(107, fRt);// RR_03 
  addPrecipitation(108, fRt);// RR_06
  addPrecipitation(109, fRt);// RR_12
  addPrecipitation(110, fRt);// RR_24
}

void synop::addPrecipitation(int par,float fRt)
{
  float newRRR;

  if(!value(par,newRRR))  return;
  if(newRRR < 0)          return;
  newRRR += fRt;
  if(newRRR < 0)          return;
  
  setBuffer(par,newRRR);
  
}


void synop::setHorizontalVisibillity(const char* token)
{
  int   ivv = readToken(token,3,2);  // VV in code
  float vv  = undef;                 // VV in m

  if      (             ivv <= 50 ) vv =  ivv        * 100;  // 0  - 5  km 
  else if ( ivv > 50 && ivv <= 55 ) vv =  undef;             // not used 
  else if ( ivv > 55 && ivv <= 80 ) vv = (ivv - 50 ) * 1000; // 5  - 30 km
  else if ( ivv > 80 && ivv <  90 ) vv = (ivv - 74 ) * 5000; // 30 - 70 km
  else {
    
    // special cases

    if      (ivv == 90)  vv = 0;
    else if (ivv == 91)  vv = 50;
    else if (ivv == 92)  vv = 200;
    else if (ivv == 93)  vv = 500;
    else if (ivv == 94)  vv = 1000;
    else if (ivv == 95)  vv = 2000;
    else if (ivv == 96)  vv = 4000;
    else if (ivv == 97)  vv = 10000;
    else if (ivv == 98)  vv = 20000;
    else if (ivv == 99)  vv = 50000;
  }      

   setBuffer(VV,vv);
}



void synop::setCloudBaseHeight(const char* token)
{
  int   ih = readToken(token,2,0);
    
  if      (ih == 0  ) setBuffer(h,  0    );
  else if (ih == 1  ) setBuffer(h,  50   );
  else if (ih == 2  ) setBuffer(h,  100  );
  else if (ih == 3  ) setBuffer(h,  200  );
  else if (ih == 4  ) setBuffer(h,  300  );
  else if (ih == 5  ) setBuffer(h,  600  );
  else if (ih == 6  ) setBuffer(h,  1000 );
  else if (ih == 7  ) setBuffer(h,  1500 );
  else if (ih == 8  ) setBuffer(h,  2000 );
  else if (ih == 9  ) setBuffer(h,  2500 );
  else                setBuffer(h, -3    ); // undef == -3 for klima-DB
  
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
    
  section = S000;
  element = 0;

}

void synop::fetchSection(const char *token)
{
  tokenType      = NORMALTOKEN;
  int sec        = readToken(token);
  synopSection s = section;

  if( sec==2 ) {
    if( section == S000 ) {
      sortToken(token);
      // to avoid mistakes at certain dates. 
      // For instance 22 at 21 oclock -> 22211 section 2
      // the group 222xx can result in here ....
      return;
    }

    if( section == S111 ) {
      if(element < 2 ) {
	sortToken(token);
	// to avoid mistakes at certain values. 
	// For instance N=2 DD=22 -> 222xx in section 1
	// the group 222xx can result in here ....
	return;
      }
    }
  }



  if(sec > 1 )
    raw+=token;

  element =  0;
  group   = -1;

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
  case 9:
    section = S999;
    break;
  }
  
  if(section < s) 
    addError(errorOrder,token);
}


/// usually used for unrecognised tokens

void synop::setText(const char *token)
{
  raw+=token;
  raw+=" ";
  
  int tokensize =  strlen(token);
  
  if(section == S555 && checkCountry() != norway)
    return;


  if(tokenType == ICETEXTTOKEN) 
    icetext+=string(" ")+token;
  else if (tokenType == ICINGTEXTTOKEN)
    icingtext+=string(" ")+token;
  else {
    if ( section == S000 && element == 0 && type != SYNOP){
      element++;
      pinfo.callSign = "";
      for(int i=0; i< tokensize;i++)
	pinfo.callSign+= toupper(token[i]);
      if(tokensize > 9 || tokensize < 3 )
	addError(errorCallsign,token);
    }
    else {
      addError(errorToken,token); 
    }
  } 
}


// sort tokens into groups ......

void synop::sortToken(const char *token, bool trash)
{
  raw+=token;
  

  // If this is trash we have allready achiewed what we want:
  // 1) Add the trash to the raw
  // 2) And we increment elements if we are in section S111
  
  if ( trash ) {
     if ( section == S111 )
        element++;
        
     return;
  } 

  element++;
   
  tokenType = NORMALTOKEN;

  if( section == S999 )
    return; /// german praxis

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
// The ICE plain text tokens

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
  
  setStaticToken(pinfo.YY,token,0,2);
  setStaticToken(pinfo.GG,token,2,2);
  int iw = readToken(token,4,0);;
 
  if( iw < 0 || iw > 4 )
    wunit = NOWINDUNIT;
  else 
    wunit = ( iw < 3 ? MS : KT );  

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
    setStaticToken(pinfo.IIiii,token,0,5);
    fetchSection("111");
    break;
  }
}

void synop::sortShipMobil000Token(const char *token)
{
  switch(element) {
  case 1:
    setStaticToken(pinfo.A1BwNb,token,0,5);
    break;
  case 2:
    setYYGGiw(token);
    break;
  case 3:
    setScaledToken(LaLaLa,token,0.1,2,3);
    break;
  case 4:
    setStaticToken(pinfo.Qc,token);
    setScaledToken(LoLoLoLo,token,0.1,1,4);
    squareDetection(); // use Qc on LoLoLoLo and LaLaLa
    if(type == SHIP )
      fetchSection("111");
    break;
  case 5:
    setStaticToken(pinfo.MMM,token,0,3);
    setStaticToken(pinfo.UlaUlo,token,3,2);
    break;
  case 6:
    setStaticToken(pinfo.h0h0h0h0,token,0,4);
    setStaticToken(pinfo.im,token,4);
    fetchSection("111");
    break;
  }
}


// SECTION 1 ----------------------------------

// The first 2 groups are defined (iRixhVV / Nddff)
// after that the grouping is given by the first 
// token 0...9

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
      if(*(token+1) != '9') {
	setTemperature(token+1,TdTdTd);
	computeUUU();
      }
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
      setScaledToken(ppp,token,0.1,2,3);
      break;
    case 6:
      setPrecipitation(token);
      break;
    case 7:
      if(hasobs(ix))
	if(int(obs[ix])==7) {
	  setToken(wawa,token,1,2);
	  setToken(Wa1,token,3,0); 
	  setToken(Wa2,token,4,0);
	  break;
	}
      setToken(ww,token,1,2);
      setToken(W1,token,3,0); 
      setToken(W2,token,4,0);
      break;
    case 8:
      setToken(Nh,token,1);
      setToken(Cl,token,2);
      setToken(Cm,token,3);
      setToken(Ch,token,4);
      break;
    case 9:
      setStaticToken(pinfo.GGgg,token,1,4);      
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
      setCloudBaseHeight(token);
      setHorizontalVisibillity(token);
      break;
    case 2:
      setToken(N ,token);
      setScaledSubstituteToken(dd,token,10,99,-3,1,2); // 99 == -3 for klima-DB
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
    setScaledToken(HwaHwa,token,0.5,3,2);
    break;
  case 2:
    setToken(PwPw,token,1,2);
    setScaledToken(HwHw,token,0.5,3,2);
    break;
  case 3:
    setScaledToken(dw1dw1, token, 10, 1, 2);
    setScaledToken(dw2dw2, token, 10, 3, 2);
    break;
  case 4:
    setToken(Pw1Pw1,token,1,2);
    setScaledToken(Hw1Hw1,token,0.5,3,2);
    break;
  case 5:  
    setToken(Pw2Pw2,token,1,2);
    setScaledToken(Hw2Hw2,token,0.5,3,2);
    break;
  case 6:
    setToken(Is,token+1);
    setToken(EsEs,token,2,2);
    setToken(Rs,token+4);
    break;
  case 7:
    setScaledToken(HwaHwaHwa,token,0.1,2,3);
    break;
  case 8:
    setTemperature(token+1,TbTbTb,sw);
    break;
  }

}

void synop::setShipDirectionAndVelocity(const char *token)
{


  if(type == SHIP ) {
    setToken(ds,token,3);
    setToken(vs,token,4);
  }
}

// SECTION 3 ----------------------------------

void synop::setSupplementaryJJ(const char* token)
{
  int j0   = readToken(token);
  int j1   = readToken(token+1);
  int j2   = readToken(token+2);
  int j234 = readToken(token,2,3);
 
  if(!sjj) {
    if ( j1 < 3 ) {
      setToken(EEE,token,1,3);
      setToken(ie, token,4,1);
    }
    else if ( j1 == 5 ) {
      if ( j2 < 3 )
	setToken(SSS,token,2,3);
      sjj = j234;
    }
  }
  else { /// second group found:
    switch(sjj) {
    case 507:
      setToken(FF24,token,0,5);
      break;
    }

    /// third group etc. found. Up to group 6 is anything possible
    /// this may be a bug from the WMO because of the crash with
    /// group 6RRRtR which is now ignored in case og j5j6...
    /// this is NOT used in Norway

    sjj=1;		  
    if(j0 > 5 ) 
      sjj = 0;
  }

}
void synop::setExtra333(const char* token)
{
  /// This function does nothing at this
  /// point and is only added for future expansion
  /// the 80000 (0....) (1....) group is not used
  /// in norway now 

  return;
}

void synop::setSupplementarySP(const char* token)
{
  // This function only adds the max gust
  // from spsp ... 

  int Sp = readToken(token,0,2);

  switch(Sp) {
  case 11:
    setWind(ff_911,token,2,2);
    break;
  }

}

void synop::setExtraClouds(const char* token)
{
  if(clCounter++ > 3 ){
    addError("Clouds",token);
    return;
  }
   
  setToken( Ns1  +clCounter, token, 1   );
  setToken( C1   +clCounter, token, 2   );
  setToken( hshs1+clCounter, token, 3, 2);

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
    {
    	//Climadb quirk
    	int EValue;;

    	if( value( E, EValue ) ) {
    		if( EValue>=0 && EValue<=9 )
    			setBuffer( sss, -1 );
    	}
    }
    break;
  case 4:
    setToken(E_,token,1);
    setSnowToken(sss,token,2,3);
    setSnowCoverBySoilCondition();   /// extra SD by E_
    break;
  case 5:
    setSupplementaryJJ(token);
    break;
  case 6: 
    if(!hasobs(tr)) {
      setPrecipitation(token);
    } /// just if this does not exist in S111
    break;
  case 7: {
		  int tmp=readToken( token, 1, 4 );

		  if( tmp == 9999 )
			  setBuffer( RR24, 0 );
		  else if( tmp == 0 )
			  setBuffer( RR24, -1 );
		  else
			  setScaledToken(RR24,token,0.1,1,4);
	  }
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

/// 444 contains only one group N'C'H'H'Ct

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
// at this time there are only four groups defined
// in Norway ...

void synop::sort555Token(const char *token)
{
  if(checkCountry() != norway)
    return;

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
    
     setPrecipitationRt(token);
     setToken(X1wd,token,2,0);
     setToken(X2wd,token,3,0);
     setToken(X3wd,token,4,0);
     break;
   default:
     addError(errorUnknown,token);
     break;
   }
   

}

// OUTPUT -------------------------------------- 


string synop::index() 
{
  map<int,float>::iterator itr= obs.begin();
  
  ostringstream ost;
  
  for(;itr!=obs.end(); itr++) 
    ost << itr->first << ":\t" << itr->second  << endl;

  return ost.str();
}
  

/// EXTRA ---------------------------------------


/// for klima-DB / kvalobs compability
void synop::setSnowCoverBySoilCondition()
{
  int   EM;

  if(!value(E_,EM))
    return;

  if      (EM == 0  ) setBuffer(SD, 0 );
  else if (EM == 1  ) setBuffer(SD, 1 );
  else if (EM == 2  ) setBuffer(SD, 3 );
  else if (EM == 3  ) setBuffer(SD, 4 );
  else if (EM == 4  ) setBuffer(SD, 4 );
  else if (EM == 5  ) setBuffer(SD, 1 );
  else if (EM == 6  ) setBuffer(SD, 3 );
  else if (EM == 7  ) setBuffer(SD, 4 );
  else if (EM == 8  ) setBuffer(SD, 4 );
  else if (EM == 9  ) setBuffer(SD, 4 );

}


/// computing UUU (relativ humidity )
/// in case of raporting TT/TD instead of UUU
/// for klima testing purpose ...


void synop::computeUUU()
{
  if(!hasobs(TdTdTd) || !hasobs(TTT) )
    return;

  float tt_ =  obs[TTT];
  float td_ =  obs[TdTdTd];
  float uu  =  100.0 * svp( td_) / svp( tt_ );
  
  int iuu=static_cast<int>(rint(uu));
  uu=iuu;

  setBuffer(UUU, (uu>100? 100:uu ) );
}

/// Sverdrups equation to compute vapour pressure E
/// empirical equation with different sets over neg/pos.

float synop::svp(double tt)
{
  if(tt>0.0)
    return 6.10780*exp( 17.08085*tt / ( 243.175+tt));
  else
    return 6.10780*exp( 17.84363*tt / ( 245.425+tt));
}
