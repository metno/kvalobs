/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvMetar.cc,v 1.3.2.1 2007/09/27 09:02:37 paule Exp $                                                       

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
#include "kvMetar.h"
#include <iostream>

extern "C" {
#include <metarParser.h>
#include <wwTabInit.h>
}

using namespace miutil;
using namespace std;

/// metar * m is a complex pointer containing struct system.
/// allways use metarKill / metarCopy etc....
/// or you will be out of luck....

kvMetar::kvMetar(const kvMetar& rhs)
{ 
  copyMembers(rhs);
}

void kvMetar::wwTabInit(const miutil::miString& wwfile)
{
  wwtabInit(wwfile.cStr());
}


void kvMetar::safe_create() 
{
  /// trick to force c++ templates to use the constructor
  /// instead of the copy constructor (like for instance vector does)
  /// (goes bad with metar* )
  if(unsafe != 42 ){
    unsafe= 42;
    m     = NULL;
  }
}

kvMetar::~kvMetar() 
{
  clear();
}

void kvMetar::copyMembers(const kvMetar& rhs)
{
  if(unsafe)
    safe_create();
  
  unrecognised = rhs.unrecognised;
  raw          = rhs.raw;
  obs          = rhs.obs;
  txt          = rhs.txt;

  metarCopy(&m,rhs.m);
}

void kvMetar::clear()
{
  unrecognised="";
  raw="";
  obs.clear();
  txt.clear();
  metarKill(&m);
}

miutil::miString kvMetar::ICAOID() 
{
  if(!m) 
    return "";
  
  return miString(m->header->icaoID);
}

miutil::miTime kvMetar::createObsTime(miutil::miTime r)
{
  if(!m) 
    return r;
  
  int i = m->header->issued;
  int d =  i        / 10000;
  int h = (i%10000) / 100 ;
  int m =  i%100;
  
  return miTime(r.year(),r.month(),d,h,m,0);
}

bool kvMetar::decode(const miutil::miString& r)
{
  clear();
  raw=r;

  char *em=NULL,*es=NULL;
  m=parseMetar(raw.cStr(),&es,&em);

  free(em);
  free(es);
  
  if(m)
    setMetarTokens();
  else
    setUnrecognised();
  
  return bool(m);
}

void kvMetar::setToken(const int par,float value,int lvl)
{
  kmet::obsbuf o;
  o.val = value;
  o.lvl = lvl;
  o.par = par;
  obs.push_back(o); 
};

void kvMetar::setToken(const int par,miString value) 
{
  kmet::txtbuf t;
  t.val = value;
  t.par = par;
  txt.push_back(t);
}

void kvMetar::setUnrecognised()
{
  int ep=metarErrorAt(); 
  
  if(ep < 1 || ep > raw.size()) 
    unrecognised = "Logical error";
  else 
    unrecognised ="Error after:" + raw.substr(0,ep);
}


/// SPECIAL CASES


const int kvMetar::dir2int(const metarDirection md)
{
  switch(md){
  case NODIR:
    return 0; 
  case North:
    return 0;
  case NorthE:
    return 45;
  case East:
    return 90;
  case SouthE:
    return 135;
  case South:
    return 180;
  case SouthW:
    return 225;
  case West:
    return 270;
  case NorthW:
    return 315;
  }
  return 0;
}

const int kvMetar::amount2int(const metarCloudAmount amount)
{
  switch(amount) {
  case VV:
    return -2;
  case  NSC:
    return -1;
  case SKC:
    return 0;
  case  FEW:
    return 2;
  case  SCT:
    return 4;
  case  BKN:
    return 6;
  case  OVC:
    return 8;
  }
  return 0;
}


void kvMetar::setWind(const int par,float value, metarWindUnit unit)
{
  float scale = 1;
  if(unit==KT)
    scale=0.5144; /// Knots 2 m/s
  if(unit==KMH)
    scale=0.277;  /// Kmh 2 m/s
  setToken(par,value*scale);
}

///// METARTOKENS ........

void kvMetar::setMetarTokens()
{
  ///  cerr << metar2str(m) << endl;
  setToken(kmet::MESS,raw);

  if (m->obs) {

    /// VISIBILITY

    if(m->obs->metVisib) {

      if( m->obs->metVisib->CAVOK ) {
	setToken(kmet::WWCAVOK,1);
      } else {
	setToken(kmet::VV,    m->obs->metVisib->vvvv);

	if(m->obs->metVisib->Dv)
	  setToken(kmet::DVV, dir2int(m->obs->metVisib->Dv)  );

	if(m->obs->metVisib->vxvx)
	  setToken(kmet::VVX, m->obs->metVisib->vxvx);

	if(m->obs->metVisib->Dvx)
	  setToken(kmet::DVX, dir2int(m->obs->metVisib->Dvx) );
      }
    }

    /// TEMPERATURE

    if(m->obs->metTemp ) {
      setToken(kmet::TA,    m->obs->metTemp->tt);
      setToken(kmet::TD,    m->obs->metTemp->td);
    }

    /// WEATHER

    if(m->obs->metWeather){
      miString ww_;
      for (int i = 0; i < m->obs->metWeather->size; i++ ){
	ww_+= m->obs->metWeather->ptr[i];
	setToken(kmet::WWB,ww_);
      }
    }

    /// WIND

    if(m->obs->metWind ) {
      /// directions ....
      if(m->obs->metWind->vrb)
	setToken(kmet::DD,     999); /// variable wind
      else
	setToken(kmet::DD,     m->obs->metWind->dd);

      if(m->obs->metWind->dn)
	setToken(kmet::DVRBDN, m->obs->metWind->dn);
      if(m->obs->metWind->dx)
	setToken(kmet::DVRBDX, m->obs->metWind->dx);

      /// speed ....
      if(m->obs->metWind->ff)
	setWind(kmet::FF,    m->obs->metWind->ff,  m->obs->metWind->unit);
      if(m->obs->metWind->gff)
	setWind(kmet::FG_10, m->obs->metWind->gff, m->obs->metWind->unit);
      
    }
    
    /// pressure

    if(m->obs->metQNH) 
      if(m->obs->metQNH->unit) {
	float scale = (m->obs->metQNH->unit == A ? 0.3385 : 1); /// inch/hpa
	setToken(kmet::PH, float(m->obs->metQNH->pp) * scale );
      }

    /// clouds and VV(HL) in m

    if(m->obs->metCloud) {
      int scale = 30;
      
      int NS = kmet::NS1;
      int HS = kmet::HS1;
      int CC = kmet::CC1;

      for(int i=0; i<m->obs->metCloud->size; i++ ) {

	if ( m->obs->metCloud->ptr[i].amount == NSC ) {
	  continue;
	}
	else if (m->obs->metCloud->ptr[i].amount == VV) {
	  setToken(kmet::HL, m->obs->metCloud->ptr[i].base * scale );
	} 
	else {
	  int base   = m->obs->metCloud->ptr[i].base * scale;
	  int amount = amount2int(m->obs->metCloud->ptr[i].amount );

	  setToken(NS, amount);	  
	  setToken(HS, base   );
	  
	  if(m->obs->metCloud->ptr[i].cbType) {
	    int cb = ( m->obs->metCloud->ptr[i].cbType == CB ? 9 : 8 );
	    setToken(CC, cb );
	  }
	  
	  NS++;
	  HS++;
	  CC++;
	}
      }
    }
  }
}


