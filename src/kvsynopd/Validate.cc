/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: ValidData.cc,v 1.5.2.4 2007/09/27 09:02:23 paule Exp $                                                       

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
#include <milog/milog.h>
#include "Validate.h"

using namespace kvalobs;
using namespace kvQCFlagTypes;

//Bxrge Moe
//2005.02.28
//Endring av grenseverdi filter for temeperatur. 
//Vi slipper ikke gjennom "sv�rt mistenkelige" temperaturer.
//Dvs.observert verdi h�yere enn h�yeste testverdi eller lavere enn 
//laveste testverdi.




kvdatacheck::
Validate::
Validate()
{
	validate = &Validate::validDataNoCheck;
}

kvdatacheck::
Validate::
Validate( HowToValidate howToValidate )
{
	switch( howToValidate ) {
	case UseOnlyControlInfo:
		validate = &Validate::validDataUseOnlyControlInfo;
		break;
	case UseOnlyUseInfo:
		validate = &Validate::validDataUseOnlyUseInfo;
		break;
	case CombineControlAndUseInfo:
		validate = &Validate::validDataCombineControlAndUseInfo;
		break;
	case NoCheck:
		validate = &Validate::validDataNoCheck;
		break;
	default:
		validate = &Validate::validDataNoCheck;
	}
}

bool
kvdatacheck::
Validate::
operator()( const Data &data )
{
	return (this->*validate)( data );
}


/**
 * We use the controlinfo and useinfo in data to deceide if we can use
 * this parameter. We use the parameter:
 *  1) It is not  checked
 *  2) It has passed the check
 *  3) It has not passed the check, but it is impossible to deciede if there
 *     is a problem with this parameter or another parameter it is
 *     checked againt.
 * 
 * For useinfo.
 * We dont use the data if:
 * 1) useingo(1) Have a observation period that differ to mutch from normert.
 */
bool
kvdatacheck::
Validate::
validDataUseOnlyControlInfo( const Data &data )
{
  kvUseInfo     uinfo=data.useinfo();
  kvControlInfo info=data.controlinfo();
  
  if(!check_useinfo1(uinfo, data.paramID())){
    LOGINFO("REJECTED useinfo(1): stationid: " << data.stationID() 
	    << " obstime: " << data.obstime()
	    << " paramid: " << data.paramID() 
	    << " original: " << data.original());
    return false;
  }
    

  if(!check_fmis(info, data.paramID())){
    LOGINFO("REJECTED (fmis): stationid: " << data.stationID() 
	    << " obstime: " << data.obstime()
	    << " paramid: " << data.paramID() 
	    << " original: " << data.original());
    return false;
  }


  if(!check_fr(info, data.paramID())){
    LOGINFO("REJECTED (fr): stationid: " << data.stationID() << " obstime: " 
	    << data.obstime() << " paramid: " << data.paramID() 
	    << " original: " << data.original());
    return false;
  }

  if(!check_fcc(info, data.paramID())){
    LOGINFO("REJECTED (fcc): stationid: " << data.stationID() << " obstime: " 
	    << data.obstime() << " paramid: " << data.paramID() << 
	    " original: " << data.original());
    return false;
  }

  if(!check_fcp(info, data.paramID())){
    LOGINFO("REJECTED (fcp): stationid: " << data.stationID() 
	    << " obstime: " << data.obstime()
	    << " paramid: " << data.paramID() 
	    << " original: " << data.original());
    return false;
  }

  if(!check_fs(info, data.paramID())){
    LOGINFO("REJECTED (fs): stationid: " << data.stationID() 
	    << " obstime: " << data.obstime()
	    << " paramid: " << data.paramID() 
	    << " original: " << data.original());
    return false;
  }

  if(!check_fnum(info, data.paramID())){
    LOGINFO("REJECTED (fnum): stationid: " << data.stationID() 
	    << " obstime: " << data.obstime()
	    << " paramid: " << data.paramID() 
	    << " original: " << data.original());
    return false;
  }

  if(!check_fpos(info, data.paramID())){
    LOGINFO("REJECTED (fpos): stationid: " << data.stationID() 
	    << " obstime: " << data.obstime()
	    << " paramid: " << data.paramID() 
	    << " original: " << data.original());
    return false;
  }

  
  return true;
}

bool
kvdatacheck::
Validate::
validDataUseOnlyUseInfo( const Data &data )
{
  kvUseInfo     uinfo=data.useinfo();

  return true;
}

bool
kvdatacheck::
Validate::
validDataCombineControlAndUseInfo( const Data &data )
{
  kvUseInfo     uinfo=data.useinfo();
  kvControlInfo info=data.controlinfo();

  return true;
}


bool
kvdatacheck::
Validate::
validDataNoCheck( const Data &data )
{
  return true;
}


bool 
kvdatacheck::
Validate::
check_fr(kvalobs::kvControlInfo &f, int paramid)
{
  int  flag;
  
  flag=flag2int(f.cflag(f_fr));

  if(flag<0){
    LOGERROR("Invalid flag value <" << f.cflag(f_fr) << ">!");
    return true;
  }

  if(flag>5){
    return false;
  }

  //Bxrge Moe
  //2005.02.28
  //
  //Vi slipper ikke gjennom "sv�rt mistenkelige" temperaturer.
  //Dvs.observert verdi h�yere enn h�yeste testverdi eller lavere enn 
  //laveste testverdi.
 #if 0
  if(paramid==211 ||   //TA
     paramid==213 ||   //TAN
     paramid==215 ||   //TAX
     paramid==216 ||   //TAX_12
     paramid==214){    //TAN_12
    if(flag>3)
      return false;
  }
#endif
  
  return true;
}

bool 
kvdatacheck::
Validate::
check_fcc(kvalobs::kvControlInfo &f, int paramid)
{
  int  flag=flag2int(f.cflag(f_fcc));

  if(flag<0){
    LOGERROR("Invalid flag value <" << f.cflag(f_fcc) << ">!");
    return true;
  }
  
  if(flag>7){
    return false;
  }
  
  return true;
}

bool 
kvdatacheck::
Validate::
check_fcp(kvalobs::kvControlInfo &f, int paramid)
{
  int  flag=flag2int(f.cflag(f_fcp));

  if(flag<0){
    LOGERROR("Invalid flag value <" << f.cflag(f_fcp) << ">!");
    return true;
  }
  
  if(flag>7){
    return false;
  }
  
  return true;
}

bool 
kvdatacheck::
Validate::
check_fs(kvalobs::kvControlInfo &f, int paramid)
{
  int  flag=flag2int(f.cflag(f_fs));

  if(flag<0){
    LOGERROR("Invalid flag value <" << f.cflag(f_fs) << ">!");
    return true;
  }
  
  if(flag>5){
    return false;
  }
  
  return true;
}

bool 
kvdatacheck::
Validate::
check_fnum(kvalobs::kvControlInfo &f, int paramid)
{
  int  flag=flag2int(f.cflag(f_fnum));

  if(flag<0){
    LOGERROR("Invalid flag value <" << f.cflag(f_fnum) << ">!");
    return true;
  }
  
  if(flag>5){
    return false;
  }
  
  return true;
}

bool 
kvdatacheck::
Validate::
check_fpos(kvalobs::kvControlInfo &f, int paramid)
{
  int  flag=flag2int(f.cflag(f_fpos));

  if(flag<0){
    LOGERROR("Invalid flag value <" << f.cflag(f_fpos) << ">!");
    return true;
  }
  
  if(flag>3){
    return false;
  }
  
  return true;
}

bool 
kvdatacheck::
Validate::
check_fmis(kvalobs::kvControlInfo &f, int paramid)
{
  int  flag=flag2int(f.cflag(f_fmis));

  if(flag<0){
    LOGERROR("Invalid flag value <" << f.cflag(f_fmis) << ">!");
    return true;
  }
  
  if(flag==0){
    return true;
  }
  
  return false;
}


int
kvdatacheck::
Validate::
flag2int(char c)
{
  switch(c){
  case '0': return 0;
  case '1': return 1;
  case '2': return 2;
  case '3': return 3;
  case '4': return 4;
  case '5': return 5;
  case '6': return 6;
  case '7': return 7;
  case '8': return 8;
  case '9': return 9;
  case 'a':
  case 'A': return 10;
  case 'b':
  case 'B': return 11;
  case 'c':
  case 'C': return 12;
  case 'd':
  case 'D': return 13;
  case 'e':
  case 'E': return 14;
  case 'f':
  case 'F': return 15;
  default:
    return -1;
  }
}




bool 
kvdatacheck::
Validate::
check_useinfo1(kvalobs::kvUseInfo  &f,int paramid)
{
  int ui=flag2int(f.cflag(1)); 

  if(ui<0){
    LOGERROR("Invalid flag value (useinfo(1)) <" << f.cflag(1) << ">!");
    return true;
  }

  if(ui!=9 && ui>1)
    return false;
  
  return true;
}
