/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: SynopData.cc,v 1.17.2.7 2007/09/27 09:02:23 paule Exp $                                                       

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
#include <float.h>
#include <sstream>
#include <decodeutility/decodeutility.h>
#include "SynopData.h"

using namespace decodeutility;

SynopData::SynopData():
    tempNaa(FLT_MAX),
    tempMid(FLT_MAX), 
    tempMin(FLT_MAX),
    tempMax(FLT_MAX),
    fuktNaa(FLT_MAX),
    fuktMid(FLT_MAX),
    vindHastNaa(FLT_MAX),
    vindHastMid(FLT_MAX),
    vindHastGust(FLT_MAX),
    vindHastMax(FLT_MAX),
    FX_3(FLT_MAX),
    vindRetnNaa(FLT_MAX),
    vindRetnMid(FLT_MAX),
    vindRetnGust(FLT_MAX),
    vindRetnMax(FLT_MAX),
    DX_3(FLT_MAX),
    nedboerTot(FLT_MAX), 
    nedboer1Time(FLT_MAX),
    nedboer2Time(FLT_MAX),
    nedboer3Time(FLT_MAX),  
    nedboer6Time(FLT_MAX),
    nedboer9Time(FLT_MAX),
    nedboer12Time(FLT_MAX),
    nedboer15Time(FLT_MAX),
    nedboer18Time(FLT_MAX),
    nedboer24Time(FLT_MAX),
    nedboerJa(FLT_MAX),
    trykkQFENaa(FLT_MAX),
    trykkQFEMid(FLT_MAX),
    trykkQFEMin(FLT_MAX),
    trykkQFEMax(FLT_MAX),
    trykkQNHNaa(FLT_MAX),
    trykkQFFNaa(FLT_MAX),
    trykkTendens(FLT_MAX), 
    TAN_12(FLT_MAX),
    TAX_12(FLT_MAX),
    TW(FLT_MAX),
    TWM(FLT_MAX),
    TWN(FLT_MAX),
    TWX(FLT_MAX),
    TGN(FLT_MAX),
    TGN_12(FLT_MAX),
    FG(FLT_MAX),
    FX(FLT_MAX),
    WAWA(FLT_MAX),
    HLN(FLT_MAX),
    nedboerInd_verInd("//"),
    hoeyde_sikt("///"),
    verGenerelt("////"),
    skyer("////"),
    verTillegg("////"),
    skyerEkstra1("////"),
    skyerEkstra2("////"),
    skyerEkstra3("////"),
    skyerEkstra4("////"),
    snoeMark("////")
{
}



SynopData::SynopData(const SynopData &p):
    time_(p.time_), 
    tempNaa(p.tempNaa),
    tempMid(p.tempMid), 
    tempMin(p.tempMin),
    tempMax(p.tempMax),
    fuktNaa(p.fuktNaa),
    fuktMid(p.fuktMid),
    vindHastNaa(p.vindHastNaa),
    vindHastMid(p.vindHastMid),
    vindHastGust(p.vindHastGust),
    vindHastMax(p.vindHastMax),
    FX_3(p.FX_3),
    vindRetnNaa(p.vindRetnNaa),
    vindRetnMid(p.vindRetnMid),
    vindRetnGust(p.vindRetnGust),
    vindRetnMax(p.vindRetnMax),
    DX_3(p.DX_3),
    nedboerTot(p.nedboerTot), 
    nedboer1Time(p.nedboer1Time),  
    nedboer2Time(p.nedboer2Time),
    nedboer3Time(p.nedboer3Time),
    nedboer6Time(p.nedboer6Time),
    nedboer9Time(p.nedboer9Time),
    nedboer12Time(p.nedboer12Time),
    nedboer15Time(p.nedboer15Time),
    nedboer18Time(p.nedboer18Time),
    nedboer24Time(p.nedboer24Time),
    nedboerJa(p.nedboerJa),
    trykkQFENaa(p.trykkQFENaa),   
    trykkQFEMid(p.trykkQFEMid),
    trykkQFEMin(p.trykkQFEMin),
    trykkQFEMax(p.trykkQFEMax),
    trykkQNHNaa(p.trykkQNHNaa),
    trykkQFFNaa(p.trykkQFFNaa),
    trykkTendens(p.trykkTendens),
    TAN_12(p.TAN_12),
    TAX_12(p.TAX_12),
    TW(p.TW),
    TWM(p.TWM),
    TWN(p.TWN),
    TWX(p.TWX),
    TGN(p.TGN),
    TGN_12(p.TGN_12),
    FG(p.FG),
    FX(p.FX),
    WAWA(p.WAWA),
    HLN(p.HLN),
    nedboerInd_verInd(p.nedboerInd_verInd),
    hoeyde_sikt(p.hoeyde_sikt),
    skydekke(p.skydekke),
    //    nedboermengde(p.nedboermengde),
    verGenerelt(p.verGenerelt),
    skyer(p.skyer),
    verTillegg(p.verTillegg),
    skyerEkstra1(p.skyerEkstra1),
    skyerEkstra2(p.skyerEkstra2),
    skyerEkstra3(p.skyerEkstra3),
    skyerEkstra4(p.skyerEkstra4),
    sjoeTemp(p.sjoeTemp),
    sjoegang(p.sjoegang),
    snoeMark(p.snoeMark),
    AA(p.AA),
    ITZ(p.ITZ),
    IIR(p.IIR),
    ITR(p.ITR)
  
{
}

SynopData&
SynopData::operator=(const SynopData &p)
{
    if(this==&p)
	return *this;

    time_            =p.time_;
    tempNaa          =p.tempNaa;
    tempMid          =p.tempMid; 
    tempMin          =p.tempMin;
    tempMax          =p.tempMax;
    fuktNaa          =p.fuktNaa;
    fuktMid          =p.fuktMid;
    vindHastNaa      =p.vindHastNaa;
    vindHastMid      =p.vindHastMid;
    vindHastGust     =p.vindHastGust;
    vindHastMax      =p.vindHastMax;
    FX_3             =p.FX_3;
    vindRetnNaa      =p.vindRetnNaa;
    vindRetnMid      =p.vindRetnMid;
    vindRetnGust     =p.vindRetnGust;
    vindRetnMax      =p.vindRetnMax;
    DX_3             =p.DX_3;
    nedboerTot       =p.nedboerTot; 
    nedboer1Time     =p.nedboer1Time;
    nedboer2Time     =p.nedboer2Time;
    nedboer3Time     =p.nedboer3Time;
    nedboer6Time     =p.nedboer6Time;
    nedboer9Time     =p.nedboer9Time;
    nedboer12Time    =p.nedboer12Time;
    nedboer15Time    =p.nedboer15Time;
    nedboer18Time    =p.nedboer18Time;
    nedboer24Time    =p.nedboer24Time;
    nedboerJa        =p.nedboerJa;
    trykkQFENaa      =p.trykkQFENaa;   
    trykkQFEMid      =p.trykkQFEMid;
    trykkQFEMin      =p.trykkQFEMin;
    trykkQFEMax      =p.trykkQFEMax;
    trykkQNHNaa      =p.trykkQNHNaa;
    trykkQFFNaa      =p.trykkQFFNaa;
    trykkTendens     =p.trykkTendens;
    TAN_12           =p.TAN_12;
    TAX_12           =p.TAX_12;
    TW               =p.TW;
    TWM              =p.TWM;
    TWN              =p.TWN;
    TWX              =p.TWX;
    TGN              =p.TGN;
    TGN_12           =p.TGN_12;
    FG               =p.FG;
    FX               =p.FX;
    WAWA             =p.WAWA;
    HLN              =p.HLN;
    nedboerInd_verInd=p.nedboerInd_verInd;
    hoeyde_sikt      =p.hoeyde_sikt;
    skydekke         =p.skydekke;
    //    nedboermengde    =p.nedboermengde;
    verGenerelt      =p.verGenerelt;
    skyer            =p.skyer;
    verTillegg       =p.verTillegg;
    skyerEkstra1     =p.skyerEkstra1;
    skyerEkstra2     =p.skyerEkstra2;
    skyerEkstra3     =p.skyerEkstra3;
    skyerEkstra4     =p.skyerEkstra4;
    sjoeTemp         =p.sjoeTemp;
    sjoegang         =p.sjoegang;
    snoeMark         =p.snoeMark;
    AA               =p.AA;
    ITZ              =p.ITZ;
    IIR              =p.IIR;
    ITR              =p.ITR;
    return *this;
}

SynopData::~SynopData()
{
}

void 
SynopData::cleanUpSlash()
{
  if(verGenerelt=="////")
    verGenerelt.erase();


  if(skyer=="////")
    skyer.erase();

  if(verTillegg=="////")
    verTillegg.erase();

  if(skyerEkstra1=="////")
    skyerEkstra1.erase();

  if(skyerEkstra2=="////")
    skyerEkstra2.erase();

  if(skyerEkstra3=="////")
    skyerEkstra3.erase();

  if(skyerEkstra4=="////")
    skyerEkstra4.erase();
  
  if(snoeMark=="////")
    snoeMark.erase();
}


bool
SynopData::setData(const int  &param, 
		   const std::string &data_)
{
    float       fData;
    int         im;
    char        ch;
    std::string s;
    char        buf[256];

    if(data_.empty())
      return true;

    if(sscanf(data_.c_str(),"%f", &fData)!=1){
      fData=FLT_MAX;
      return false;
    }

    im=static_cast<int>(round(fData));
    sprintf(buf, "%d", im);

    switch(param){
    case 211: tempNaa=fData;       break; //TA
    case 212: tempMid=fData;       break; //TAM
    case 213: tempMin=fData;       break; //TAN
    case 215: tempMax=fData;       break; //TAX
    case 262: fuktNaa=fData;       break; //UU
    case 263: fuktMid=fData;       break; //UM
    case  81: vindHastNaa=fData;   break; //FF
    case  85: vindHastMid=fData;   break; //FM
    case  90: vindHastGust=fData;  break; //FG_1
    case  93: FX_3=fData;          break; //FX_3
    case  87: vindHastMax=fData;   break; //FX_1
    case  61: vindRetnNaa=fData;   break; //DD
    case  64: vindRetnMid=fData;   break; //DM
    case  63: vindRetnGust=fData;  break; //DG
    case  67: vindRetnMax=fData;   break; //DX
    case 104: nedboerTot=fData;    break; //RA
    case 106: nedboer1Time=fData;  break; //RR_1
    case 107: nedboer3Time=fData;  break; //RR_3
    case 108: nedboer6Time=fData;  break; //RR_6
    case 109: nedboer12Time=fData; break; //RR_12
    case 110: nedboer24Time=fData; break; //RR_24
    case 119: nedboer2Time=fData;  break; //RR_2
    case 120: nedboer9Time=fData;  break; //RR_9
    case 123: nedboerJa=fData;     break; //RT_1
    case 125: nedboer15Time=fData; break; //RR_15
    case 126: nedboer18Time=fData; break; //RR_18
    case 173: trykkQFENaa=fData;   break; //PO
    case 174: trykkQFEMid=fData;   break; //POM
    case 175: trykkQFEMin=fData;   break; //PON
    case 176: trykkQFEMax=fData;   break; //POX
    case 172: trykkQNHNaa=fData;   break; //PH
    case 178: trykkQFFNaa=fData;   break; //PR
    case 177: trykkTendens=fData;  break; //PP
    case 214: TAN_12=fData;        break; //TAN_12
    case 216: TAX_12=fData;        break; //TAX_12
    case 242: TW=fData;            break; //TW
    case 244: TWN=fData;           break; //TWN
    case 243: TWM=fData;           break; //TWM
    case 245: TWX=fData;           break; //TWX
    case 223: TGN=fData;           break; //TGN
    case 224: TGN_12=fData;        break; //TGN_12
    case  83: FG=fData;            break; //FG
    case  86: FX=fData;            break; //FX  
    case  56: HLN=fData;           break; //HLN
    case  49: WAWA=fData;          break;
    case   9:                             //IR, _irix
              IIR=buf;
	      nedboerInd_verInd.replace(0, 1, buf);
	      break;
    case  10:                             //IX, _irix
 	      nedboerInd_verInd.replace(1, 1, buf);
	      break;
    case  55:                               //HL, _hVV
	      ch=HLKode(fData);
	      hoeyde_sikt[0]=ch;
	      break;
    case 273:                              //VV, _hVV
	      s=VVKode(fData);
	      hoeyde_sikt.replace(1, 2, s);
	      break;
    case 15:                                //NN, _N
	      skydekke=buf;
	      break;
    case 12:                                //ITR, _RRRtr
	      ITR=buf;
	      break;
    case 41:                                //WW, _wwW1W2
              sprintf(buf, "%02d", im);
              verGenerelt.replace(0, 2, buf);
	      break;
    case 42:                                //W1, _wwW1W2
	      verGenerelt.replace(2, 1, buf);
	      break;
    case 43:                                //W2, _wwW1W2
	      verGenerelt.replace(3, 1, buf);
	      break;
    case 14:                                //NH, _NhClCmCh")
	      skyer.replace(0, 1, buf);
	      break;
    case 23:                                //CL, _NhClCmCh")
	      skyer.replace(1, 1, buf);
	      break;
    case 24:                                //CM, _NhClCmCh")
	      skyer.replace(2, 1, buf);
	      break;
    case 22:                                //CH, _NhClCmCh")
	      skyer.replace(3, 1, buf);
	      break;
    case 44:                                //X1WD"_RtWdWdWd")
	      verTillegg.replace(1, 1, buf);
	      break;
    case 45:                                //X2WD"_RtWdWdWd")
	      verTillegg.replace(2, 1, buf);
	      break;
    case 46:                                //X3WD"_RtWdWdWd")
	      verTillegg.replace(3, 1, buf);
	      break;
    case 25:                                //NS1, "_1NsChshs")
              skyerEkstra1.replace(0, 1, buf);
              break;
    case 26:                                //NS2, "_2NsChshs")
              skyerEkstra2.replace(0, 1, buf);
              break;
    case 27:                                //NS3, "_3NsChshs")
              skyerEkstra3.replace(0, 1, buf);
              break;
    case 28:                                //NS4, "_4NsChshs")
              skyerEkstra4.replace(0, 1, buf);
              break;
    case 305:                                //CC1, "_1NsChshs")
              skyerEkstra1.replace(1, 1, buf);
              break;
    case 306:                                //CC2, "_2NsChshs")
              skyerEkstra2.replace(1, 1, buf);
              break;
    case 307:                                //CC3, "_3NsChshs")
              skyerEkstra3.replace(1, 1, buf);
              break;
    case 308:                                //CC4, "_4NsChshs")
              skyerEkstra4.replace(1, 1, buf);
              break;
    case 301:                                //HS1, "_1NsChshs")
              sprintf(buf, "%02d", im);
              skyerEkstra1.replace(2, 2, buf);
              break;
    case 302:                                //HS2, "_2NsChshs")
              sprintf(buf, "%02d", im);
              skyerEkstra2.replace(2, 2, buf);
              break;
    case 303:                                //HS3, "_3NsChshs")
              sprintf(buf, "%02d", im);
              skyerEkstra3.replace(2, 2, buf);
              break;
    case 304:                                //HS4, "_4NsChshs")
              sprintf(buf, "%02d", im);
              skyerEkstra4.replace(2, 2, buf);
              break;
	      
    case  19:                                 //_S
              sjoegang=buf;
	      break;
    case   7:        
              if(fData>=0)//EM, _Esss
		snoeMark.replace(0, 1, buf);
	      break;
    case 112:                                 //SA, _Esss
         if(im==-2){ // 2005.06.21, Bxrge, Endret fra -1.
		       snoeMark.replace(1, 3, "998");
	      }else if(im==-3){
		       snoeMark.replace(1, 3, "999");
	      }else if(im==998 || im==997 || im==999){
            //Bxrge Moe
		      //2005.01.20
		      //Denne er ikke i bruk og kan fjernes
            sprintf(buf,"%03d", im);
            snoeMark.replace(1, 3, buf);
         }else if(fData>=0  && fData<0.5){
            snoeMark.replace(1, 3, "997");
	      }else if(fData>=0.5 && fData<997.0){
	         im=static_cast<int>(fabs(floor(static_cast<double>(fData)+0.5)));
	         sprintf(buf,"%03d", im);
	         snoeMark.replace(1, 3, buf);
	      }
	     break;
    case  13:                                 //ITZ, "_tz")
	      ITZ=buf;
	      break;
	      
    case   1:                                 //AA, _aa
              AA=buf;
	      break;
    default:
      return false;
    }
    
    return true;
}


SynopDataList::SynopDataList()
{
} 

SynopDataList::SynopDataList(const SynopDataList &d)
{
  dataList=d.dataList;
}


SynopDataList::~SynopDataList()
{
}  
  
//SynopDataList& 
//SynopDataList::operator=(const SynopDataList &rhs)
//{
//  if(this!=&rhs){
//    dataList=rhs.dataList;
//  }
//}




  /**
   * \exception 
   */
const SynopDataList::SynopDataProxy 
SynopDataList::operator[](const miutil::miTime &t)const
{
  //std::cerr << "const [miTime] operator\n";
  
  return SynopDataProxy(const_cast<SynopDataList*>(this), t);
}

SynopDataList::SynopDataProxy 
SynopDataList::operator[](const miutil::miTime &t)
{
  //std::cerr << "[miTime] operator\n";
  
  return SynopDataProxy(this, t);
}

/**
 * \exception  
 */
const SynopData& 
SynopDataList::operator[](const int index)const
{
  CISynopDataList it=dataList.begin();

  //std::cerr << "const [int] operator\n";

  for(int i=0; it!=dataList.end() && i<index; i++,it++);

  if(it==dataList.end()){
    std::ostringstream ost;
    ost << "Index <" << index << "> out of range [0," << dataList.size()
	<< ">!";
    throw std::out_of_range(ost.str());
  }
    
  return *it;
}

SynopData&
SynopDataList::operator[](const int index)
{
   ISynopDataList it=dataList.begin();

   //std::cerr << "const [int] operator\n";

  for(int i=0; it!=dataList.end() && i<index; i++,it++);

  if(it==dataList.end()){
    std::ostringstream ost;
    ost << "Index <" << index << "> out of range [0," << dataList.size()
	<< ">!";
    throw std::out_of_range(ost.str());
  }
    
  return *it;
}

int 
SynopDataList::nContinuesTimes()const
{
  CISynopDataList it=dataList.begin();
  miutil::miTime  prevT;
  miutil::miTime  testT;
  int             n;
  
  if(it==dataList.end())
    return 0;
  
  n=1;
  prevT=it->time();
  it++;

  for(;it!=dataList.end(); it++){
    testT=it->time();
    testT.addHour(1);

    if(testT!=prevT){
      break;
    }else{
      prevT=it->time();
      n++;
    }
  }

  return n;
}

  
bool      
SynopDataList::insert(const miutil::miTime &timeIndex,
		      const SynopData            &sd,
		      bool                 replace)
{
  ISynopDataList it=dataList.begin();

  for(;it!=dataList.end(); it++){
    if(it->time()<=timeIndex)
      break;
  }
  
  if(it==dataList.end()){
    dataList.push_back(sd);
    it=dataList.end();
    it--;
  }else if(it->time()==timeIndex){
    if(!replace)
      return false;
    *it=sd;
  }else{
    it=dataList.insert(it, sd);
  }
  
  it->time_=timeIndex;

  return true;
}




ISynopDataList
SynopDataList::find(const miutil::miTime &from)
{
  ISynopDataList it=dataList.begin();

  if(from.undef())
    return dataList.end();

  for(;it!=dataList.end(); it++){
    if(it->time()<=from)
      return it;
  }

  return dataList.end();
}


CISynopDataList 
SynopDataList::find(const miutil::miTime &from)const
{
  CISynopDataList it=dataList.begin();

  if(from.undef())
    return dataList.end();

  for(;it!=dataList.end(); it++){
    if(it->time()<=from)
      return it;
  }

  return dataList.end();
}


SynopDataList::SynopDataProxy& 
SynopDataList::SynopDataProxy::operator=(const SynopData &rhs) //lvalue use
{
  //std::cout << "SynopData: lvalue ...\n";
  ISynopDataList it=sdl->dataList.begin();

  for(;it!=sdl->dataList.end(); it++){
    if(it->time()<=timeIndex)
      break;
  }

  if(it==sdl->dataList.end()){
    sdl->dataList.push_back(rhs);
    it=sdl->dataList.end();
    it--;
  }else if(it->time()==timeIndex)
    *it=rhs;
  else{
    it=sdl->dataList.insert(it, rhs);
  }

  sdl->setTime(it, timeIndex);
  
  return *this;
}

SynopDataList::SynopDataProxy::operator SynopData()const //rvalue use
{
  //std::cerr << "SynopData: rvalue ...\n";
  ISynopDataList it=sdl->dataList.begin();

  for(;it!=sdl->dataList.end(); it++){
    if(it->time()<=timeIndex)
      break;
  }
  
  if(it->time()!=timeIndex){
    std::ostringstream ost;
    ost << "NO SynopData at <" << timeIndex << ">!";
    throw std::out_of_range(ost.str());
  }
  
  return *it;
}


std::ostream& 
operator<<(std::ostream& ost, const SynopData& sd)
{
  using namespace std;
  if(sd.time_.undef())
    ost << "obsTime                    : " << "(UNDEFINED)" <<  endl;
  else
    ost << "obsTime                    : " << sd.time_ <<  endl;
  
  ost << "tempNaa                (TA): " << sd.tempNaa           << endl 
      << "tempMid               (TAM): " << sd.tempMid           << endl
      << "tempMin               (TAN): " << sd.tempMin           << endl
      << "tempMax               (TAX): " << sd.tempMax           << endl
      << "tempMin       (12t)(TAN_12): " << sd.TAN_12            << endl
      << "tempMax       (12t)(TAX_12): " << sd.TAX_12            << endl
      << "fuktNaa                (UU): " << sd.fuktNaa           << endl
      << "fuktMid                (UM): " << sd.fuktMid           << endl
      << "vindHastNaa            (FF): " << sd.vindHastNaa       << endl
      << "vindHastMid            (FM): " << sd.vindHastMid       << endl
      << "vindHastGust         (FG_1): " << sd.vindHastGust      << endl
      << "FG (Since last obs.)       : " << sd.FG                << endl
      << "vindHastMax          (FX_1): " << sd.vindHastMax       << endl
      << "FX_3                       : " << sd.FX_3              << endl
      << "FX  (Since last obs.)      : " << sd.FX                << endl
      << "vindRetnNaa            (DD): " << sd.vindRetnNaa       << endl
      << "vindRetnMid            (DM): " << sd.vindRetnMid       << endl
      << "vindRetnGust           (DG): " << sd.vindRetnGust      << endl
      << "vindRetnMax            (DX): " << sd.vindRetnMax       << endl
      << "DX_3                       : " << sd.DX_3              << endl
      << "nedboerTot             (RA): " << sd.nedboerTot        << endl
      << "nedboer1Time           (RR): " << sd.nedboer1Time      << endl
      << "nedboer2Time         (RR_2): " << sd.nedboer2Time      << endl
      << "nedboer3Time         (RR_3): " << sd.nedboer3Time      << endl
      << "nedboer6Time         (RR_6): " << sd.nedboer6Time      << endl
      << "bedboer9Time         (RR_9): " << sd.nedboer9Time      << endl
      << "nedboer12Time       (RR_12): " << sd.nedboer12Time     << endl
      << "nedboer15Time       (RR_15): " << sd.nedboer15Time     << endl
      << "nedboer18Time       (RR_18): " << sd.nedboer18Time     << endl
      << "nedboer24Time       (RR_24): " << sd.nedboer24Time     << endl
      << "Nedb�r indikator      (IIR): " << sd.IIR               << endl
      << "Nedb�r periode        (ITR): " << sd.ITR               << endl
      << "sj�temperatur          (TW): " << sd.TW                << endl
      << "TWN                   (TWN): " << sd.TWN               << endl
      << "TWM                   (TWM): " << sd.TWM               << endl 
      << "TWX                   (TWX): " << sd.TWX               << endl
      << "nedboerJa (min)      (RT_1): " << sd.nedboerJa         << endl
      << "trykkQFENaa            (PO): " << sd.trykkQFENaa       << endl
      << "trykkQFEMid           (POM): " << sd.trykkQFEMid       << endl
      << "trykkQFEMin           (PON): " << sd.trykkQFEMin       << endl
      << "trykkQFEMax           (POX): " << sd.trykkQFEMax       << endl
      << "trykkQNHNaa            (PH): " << sd.trykkQNHNaa       << endl
      << "trykkQFFNaa            (PR): " << sd.trykkQFFNaa       << endl
      << "trykkTendens           (PP): " << sd.trykkTendens      << endl
      << "trykkKarakter          (AA): " << sd.AA                << endl
      << "nedboerInd_verInd   (_irix): " << sd.nedboerInd_verInd << endl
      << "hoeyde_sikt          (_hVV): " << sd.hoeyde_sikt       << endl
      << "HLN                        : " << sd.HLN               << endl
      << "skydekke               (_N): " << sd.skydekke          << endl
    //      << "nedboermengde      (_RRRtr): " << sd.nedboermengde     << endl
      << "verGenerelt       (_wwW1W2): " << sd.verGenerelt       << endl
      << "WAWA   (ww automatisk målt): " << sd.WAWA              << endl
      << "skyer           (_NhClCmCh): " << sd.skyer             << endl
      << "verTillegg      (_RtWdWdWd): " << sd.verTillegg        << endl
      << "skyerEkstra1    (_1NsChshs): " << sd.skyerEkstra1      << endl
      << "skyerEkstra2    (_2NsChshs): " << sd.skyerEkstra2      << endl
      << "skyerEkstra3    (_3NsChshs): " << sd.skyerEkstra3      << endl
      << "skyerEkstra4    (_4NsChshs): " << sd.skyerEkstra4      << endl
      << "sjoeTemp        (_snTwTwTw): " << sd.sjoeTemp          << endl
      << "gressTemp             (TGN): " << sd.TGN               << endl
      << "gressTemp_12       (TGN_12): " << sd.TGN_12            << endl
      << "sjoegang               (_S): " << sd.sjoegang          << endl
      << "snoeMark            (_Esss): " << sd.snoeMark          << endl
      << "Naar intraff FX       (ITZ): " << sd.ITZ               << endl;
  
  return ost;
}


std::ostream& 
operator<<(std::ostream& ost,
	   const SynopDataList& sd)
{
  CISynopDataList it=sd.begin();

  for(;it!=sd.end(); it++){
    ost << *it << std::endl 
	      << "-----------------------------------" << std::endl;
  }
  
  return ost;
}
