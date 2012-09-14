/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: decodeutility.cc,v 1.9.2.8 2007/09/27 09:02:27 paule Exp $                                                       

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
#include <math.h>
#include <stdlib.h>
#include <string>
#include <stdio.h>
#include "decodeutility.h"


using namespace std;

std::string
decodeutility::
VV( const std::string &spVal_ )
{
  //else if(spDef[i].id=="VV"){
  int iVV;
  char cVV[20];
  string val;
  
  if(spVal_.empty())
    return "";
  
  iVV=atoi(spVal_.c_str());
  
  if(iVV==0){
    val="0";
  }else if(iVV>0 && iVV<=50){
    sprintf(cVV, "%d", iVV*100);
    val=cVV;
  }else if(iVV>50 && iVV<56) {
     sprintf(cVV, "%d", -iVV);
     val=cVV;
  }else if(iVV>=56 && iVV<=80){
    sprintf(cVV, "%d", (iVV-50)*1000);
    val=cVV;
  }else if(iVV>80 && iVV<89){
    sprintf(cVV, "%d", (((iVV-81)*5)+35)*1000);
    val=cVV;
  }else if(iVV == 89 ){
    val="75000";
  }else{
    val="";
  }
  
  return val;
}

std::string
decodeutility::
VVKode(float m)
{
  char cVV[20];
  int  im;

  im=static_cast<int>(round(m));
  
  if( im < 0 )
     return string("");
  
  if(im==0){
    return string("00");
  }

  if(im>0 && im<=5000){
    im=im/100;
    sprintf(cVV, "%02d", im);
    return cVV;
  }

  if(im<6000){
    return "50";
  }
    
    
  if(im>=6000 && im<=30000){
    im=im/1000+50;
    sprintf(cVV, "%02d", im);
    return cVV;
  }

  if(im<35000)
    return "80";

  if(im<40000)
    return "81";

  
  if(im<45000)
    return "82";

  if(im<50000)
    return "83";

  if(im<55000)
    return "84";

  if(im<60000)
    return "85";

  if(im<65000)
    return "86";

  if(im<70000)
    return "87";

  if(im==70000)
    return "88";

  return "89";
}

std::string
decodeutility::
HL(const std::string &spVal_)
{
  string val;

  if(spVal_.empty() || spVal_.length()!=1 )
    return "";
  
  switch(spVal_[0]){
  case '0': val="0"; break;
  case '1': val="50"; break;
  case '2': val="100"; break;
  case '3': val="200"; break;
  case '4': val="300"; break;
  case '5': val="600"; break;
  case '6': val="1000"; break;
  case '7': val="1500"; break;
  case '8': val="2000"; break;
  case '9': val="2500"; break;
  case 'X': 
  case 'x': val="-3"; break;
  default:
    val="";
  }
  return val;
}

char     
decodeutility::
HLKode(float m)
{
  int  im;
    
  im=static_cast<int>(round(m));

  if(im<0)
    return 'X';

  if(im>=0 && im<50)
    return '0';
  else if(im>=50 && im<100)
    return '1';
  else if(im>=100 && im<200)
    return '2';
  else if(im>=200 && im<300)
    return '3';
  else if(im>=300 && im<600)
    return '4';
  else if(im>=600 && im<1000)
    return '5';
  else if(im>=1000 && im<1500)
    return '6';
  else if(im>=1500 && im<2000)
    return '7';
  else if(im>=2000 && im<2500)
    return '8';
  else  //im>=2500
    return '9';
}
  

int 
decodeutility::
V456(int val)
{
  switch( val){
  case 12: return 1;
  case 11: return 2;
  case 10: return 3;
  case 16: return 4;
  case 15: return 5;
  case 14: return 7;
  case 13: return 8;
  case 17: return 10;
  case 18: return 12;
  case 19: return 17;
  case 20: return 20;
  case 21: return 29;
    
  default:
    return -1;
  }
}


