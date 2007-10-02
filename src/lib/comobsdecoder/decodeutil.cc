/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: decodeutil.cc,v 1.1.2.2 2007/09/27 09:02:24 paule Exp $                                                       

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
#include <string.h>
#include <decodeutility/decodeutility.h>
#include "decodeutil.h"


using namespace std;


bool
kvalobs::decodeutil::
doWdWdWd(kvalobs::decodeutil::DecodedDataElem &data, 
	 const std::string &wdwdwd )
{
  string buf;
  int    i;

  if(wdwdwd.empty())
    return true;

  if(wdwdwd.length()==4){
    i=1;
  }else if(wdwdwd.length()==3){
    //Vi aksepterer dette siden det virker som om
    //_RtWdWdWd mangler Rt når ir=3 eller 4.
    i=0;
  }else{
    return false;
  }
  
  
  buf=wdwdwd.substr(i,1);
  
  if(buf[0]!='/' && buf[0]!='x' && buf[0]!='X')
    data.addData("X1WD", buf);

  i++;
  buf=wdwdwd.substr(i,1);
  
  if(buf[0]!='/' && buf[0]!='x' && buf[0]!='X')
    data.addData("X2WD", buf);

  i++;
  buf=wdwdwd.substr(i,1);
  
  if(buf[0]!='/' && buf[0]!='x' && buf[0]!='X')
    data.addData("X3WD", buf);

  return true;

}

bool
kvalobs::decodeutil::
doKvEsss(kvalobs::decodeutil::DecodedDataElem &data, const std::string &Esss)
{
  string buf;

  if(Esss.length()!=4){
    data.addData("EM", "-1");
    return true;
  }

  buf=Esss.substr(0,1);
  
  if(buf[0]!='/' && buf[0]!='x' && buf[0]!='X')
    data.addData("EM", buf);
  
  buf=Esss.substr(1, 3);

  for(string::size_type i=0; i<buf.length(); i++)
    if(buf[i]=='/' && buf[i]=='x' && buf[i]=='X')
      return true;;
  
  if(buf=="997")
    data.addData("SA", "0");
  else if(buf=="998")
    data.addData("SA", "-1");
  else if(buf=="999")
    data.addData("SA", "-3");
  else{
    data.addData("SA", buf);
  }

  return true;
}


bool
kvalobs::decodeutil::
doKvNhClCmCh(kvalobs::decodeutil::DecodedDataElem &data, 
	     const std::string &NhClCmCh)
{
  string buf;

  if(NhClCmCh.length()!=4)
    return false;

  buf=NhClCmCh.substr(0,1);
  
  if(buf[0]!='/' && buf[0]!='x' && buf[0]!='X')
    data.addData("NH", buf);
  
  buf=NhClCmCh.substr(1,1);
  
  if(buf[0]!='/' && buf[0]!='x' && buf[0]!='X')
    data.addData("CL", buf);
  
  buf=NhClCmCh.substr(2,1);
  
  if(buf[0]!='/' && buf[0]!='x' && buf[0]!='X')
    data.addData("CM", buf);
  
  buf=NhClCmCh.substr(3,1);
  
  if(buf[0]!='/' && buf[0]!='x' && buf[0]!='X')
    data.addData("CH", buf);

  return true;
}

bool 
kvalobs::decodeutil::
dohVV(kvalobs::decodeutil::DecodedDataElem &data, 
      const std::string &hVV)
{
  string h;
  string vv;

  if(hVV.length()!=3)
    return false;

  h=decodeutility::HL(hVV.substr(0, 1));
  vv=decodeutility::VV(hVV.substr(1, 2));
    
  if(!h.empty())
    data.addData("HL", h);
  
  if(!vv.empty())
    data.addData("VV", vv);

  return true;
}

bool 
kvalobs::decodeutil::
doNsChshs(kvalobs::decodeutil::DecodedDataElem &data, 
	  const std::string &NsChshs)
{
  string buf;

  if(NsChshs.length()!=4)
    return false;

  buf=NsChshs.substr(0, 1);

  if(isdigit(buf[0]))
    data.addData("NS1", buf);

  buf=NsChshs.substr(1, 1);
  
  if(isdigit(buf[0]))
    data.addData("CC1", buf);

  buf=NsChshs.substr(2, 2);

  if(isdigit(buf[0]) && isdigit(buf[1]))
    data.addData("HS1", buf);

  return true;

}

bool 
kvalobs::decodeutil::
dowwW1W2(kvalobs::decodeutil::DecodedDataElem &data, 
	 const std::string &wwW1W2)
{
  if(wwW1W2.length()!=4)
    return false;

  string buf;

  buf=wwW1W2.substr(0, 2);

  if(isdigit(buf[0]) && isdigit(buf[1]))
    data.addData("WW", buf);

  buf=wwW1W2.substr(2, 1);

  if(isdigit(buf[0]))
    data.addData("W1", buf);


  buf=wwW1W2.substr(3, 1);

  if(isdigit(buf[0]))
    data.addData("W2", buf);
  
}

void
kvalobs::decodeutil::
doVT(kvalobs::decodeutil::DecodedDataElem &data, 
     const std::string &vt,
     const std::string &vt1, 
     const std::string &vt2)
{
  
  if(vt.length()!=4)
    return;

  string v1=vt.substr(0, 2);
  string v2=vt.substr(2, 2);
  

  if(!vt1.empty() && v1!="00" && 
     isdigit(v1[0]) && isdigit(v1[1]))
    data.addData(vt1, v1);
  
  if(!vt2.empty() && v2!="00" &&
     isdigit(v2[0]) && isdigit(v2[1]))
    data.addData(vt2, v2);

}
