/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: synop.cc,v 1.34.2.20 2007/09/27 09:02:23 paule Exp $                                                       

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
#include <sstream>
#include <algorithm>
#include <math.h>
#include <float.h>
#include <milog/milog.h>
#include <decodeutility/decodeutility.h>
#include "synop.h"

/*CHANGES
 *
 * 2005.11.24
 * Bxrge Moe
 * Lagt inn stÃ¸tte for FX_3 i Max_Vind_Max_Kode'en. Dette for Ã¥
 * stÃ¸tte ARGOS bÃ¸yen pÃ¥ Svalbard.
 * 
 * 2003.05.28 Bxrge Moe
 * Endret kodingen av nedbor slik at all nedbï¿½r mellom terminene 
 * (0,3,6,9,12,15,18 og 21) kodes med tR=5, nedbor siste time.
 *
 * 2003.05.26 Bxrge Moe
 * Endret prioritering av nedbï¿½r. Automatisk observert nedbï¿½r skal
 * gï¿½ foran manuell observert nedbï¿½r. (Dette i overensstemmelse med 
 * Knut Bjï¿½rheim (Roar Skï¿½lind))
 * I tillegg er all logikk med nedbï¿½rsbereegning lagt inn i en
 * egen funksjon doNedboerKode.
 *
 * 2003.03.12 Bxrge Moe
 * Endring av beregningen av nedï¿½r. Bruker kun RA (nedboerTot) dersom
 * ikke manuell nedbï¿½r er gitt. Endring initiert av Obs. Div (Ragnar Brekkan)
 * Nedbï¿½ren beregnes i nedborFromRA.
 * 
 * 2004.08.30 Bxrge Moe
 * Nedbï¿½r vil kun legges til SYNOP dersom 'precipitation' parameteren
 * er gitt i konfigurasjonsfilen. Denne endringen er gjort for ï¿½
 * kode etter ny WMO standard hvor nedbï¿½r altid skal vï¿½re med hvis stasjonen
 * er satt opp til ï¿½ vï¿½re en nedbï¿½rstasjon, Ir alltid 1. Hvis stasjonen ikke er
 * en nedbï¿½r stasjon skal Ir settes 4.
 *
 * 2004.08.31 Bxrge Moe
 * Ny koding for RRRtr, i henhold til ny WMO sandard.
 * 
 * 2006.03.30 Bxrge Moe
 * Retter feil i kodingen av gruppe 7 (RR24) i seksjon 333.
 *
 * 2006.05.23 Bxrge Moe
 * -Lagt til stÃ¸tte for automatiske mÃ¥lte verdier for ww (WaWa) i gruppe 7 i 
 *  seksjon 1.
 * -Lagt til stÃ¸tte for automatisk mÃ¥lt h (HLN), laveste skybase.
 *
 * 2006-07-12 Bxrge Moe
 * -NÃ¥r duggpunktet beregnes marginalt hÃ¸yere, 0.5 grader C, enn  
 *  lufttemperaturen sÃ¥ settes duggpunket lik lufttemperaturen.
 * -Ryddet opp i tÃ¥pelig og ulogisk kode.
 * 
 * 2007-12-19 Bxrge
 * - Endret koding av gruppe 4 E'sss.
 * 
 * 2008-01-15 Bxrge
 * - Endret kodingen av vind fra knop to m/s.
 * 
 * 2008-01-16 Bxrge
 * - Lagt til støtte for autmatisk målt VV (Vmor).
 * 
 * 2008-09-24 Bxrge
 * - Rettet avrundingsfeil i Gust, max vind og E'sss.
 * 2009-02-26 Bxrge
 * - #1241. Rettet feil i generereing av nedbør for en 1 fra RR_1. 
 */

using namespace std;

#define FEQ(f1, f2, d) (((f1)-(f2))<(d)?true:false)

bool
Synop::
doSynop(int           synopno,
	const std::string &utsteder,
	int               listenummer,
	std::string       &synop,
	StationInfoPtr    info,
	SynopDataList     &synopData,
	bool              create_CCA_template)
{
    using std::string;
    
    int    ir;
    int    ix;
    char   tmp[512];
    string buf;
    string dato_tid;
    string synopStr;
    string tempStr;
    string tidsKode;
    string luftTempKode;
    string RRRtr;
    string nedboerKodeSect3;
    string nedboerKodeSect1;
    string duggTempKode;
    string minMaxKode;
    string maxMinKode;
    string naaVindKode;
    string maxVindGustKode;
    string maxVindMaxKode;
    string trykkQFFKode;
    string trykkQFEKode;
    string tendensKode;
    string skydekkeKode;
    string hoyde_siktKode;
    string verGenereltKode;
    string verTilleggKode;
    string skyerKode;
    string snoeMarkKode;
    string skyerEkstraKode1;
    string skyerEkstraKode2;
    string skyerEkstraKode3;
    string skyerEkstraKode4;
    string sjoeTempKode;
    string gressTemp;
    bool   verGenerelt;
    string rr24Kode;
    int    ITR;
    string tmpUtsteder(utsteder);
    SynopData sisteTid;
    int       nTimeStr=0;
    
    milog::LogContext context("synop");
    
    verGenerelt    = false;
     
    errorMsg.erase();

    if(synopData.size()==0){
      errorMsg="No data!";
      return false;
    }

    nTimeStr=synopData.nContinuesTimes();
    
    sisteTid=*synopData.begin();
    ISynopDataList it=synopData.end();
    it--;

	 {
     		precipitationParam=NoPrecipitation;
      	StationInfo::TStringList precip=info->precipitation();
     
      	if(precip.size()>0){
			string rr=*precip.begin();
	
			if(rr=="RA"){
	 			 precipitationParam=PrecipitationRA;
			}else if(rr=="RR_1" || rr=="RR"){
	  			precipitationParam=PrecipitationRR;
			}else if(rr=="RR_3" || rr=="RR_6" 
		  			 || rr=="RR_12" || rr=="RR_24"){
	  			precipitationParam=PrecipitationRR_N;
			}else if( rr=="RRRtr"){
	  			precipitationParam=PrecipitationRRR;
			}else if( rr=="RR_01"){
	  			//NOT implemented (Not needed) yet
			}
      	}
    }

    if(precipitationParam==NoPrecipitation)
      	ir=4;
    else
      	ir=1;

  
    LOGDEBUG("nTimeStr (cont): " << nTimeStr << endl <<
	    	 "Tot times:       " << synopData.size() << endl <<
	     	 "sisteTid:        " << sisteTid.time() << endl <<
	    	 "fï¿½rsteTid:       " << it->time() << endl <<
	    	 "nedbor12t:       " << sisteTid.nedboer12Time << endl <<
	    	 //	     "nedbormen:       " << sisteTid.nedboermengde << endl <<
		     "verTillegg:      " << sisteTid.verTillegg << endl <<
		     "nedboerTot (RA): " << sisteTid.nedboerTot);
    
  
    sprintf(tmp,"%02d%02d",sisteTid.time().day(), sisteTid.time().hour() );
    dato_tid=tmp;
    
    Sjekk_Gruppe(4,verTilleggKode,sisteTid.verTillegg);
    
    if(ir==1){
      	doNedboerKode(RRRtr,
				      verTilleggKode,
		   			  rr24Kode,
		   			  ITR,
		    		  synopData);
    }

    LOGDEBUG("Etter doNedboerKode:"
	    	 << "\n   RRRtr:            " << RRRtr
	    	 << "\n   verTilleggKode:   " << verTilleggKode
	    	 << "\n   stasjonHarNedbï¿½r: " << (ir==1?"true":"false")
	    	 << "\n   ir:               " << ir
	    	 << "\n   ITR:              " << ITR);

	 // 7wwW1W2
    verGenerelt=doVerGenerelt(verGenereltKode, ix, sisteTid);
       
    /* Lagar tidskoda SM|SI|SN */
    Tid_Kode(tidsKode, sisteTid.time().hour());
    
    /* Lufttemperatur i synop */
    Temp_Kode(luftTempKode, sisteTid.tempNaa);

    /* Reknar ut duggtemp. vha. lufttemp. og fuktigheit */
    Dugg_Kode(duggTempKode, sisteTid.tempNaa, sisteTid.fuktNaa);
 
    /* Reknar ut nattens min.temp ELLER dagens max.temp  */
    Min_Max_Kode(minMaxKode, synopData);

    /* Reknar ut nattens max ELLER dagens min */
    Max_Min_Kode(maxMinKode, synopData);

    if(sisteTid.vindRetnNaa!=FLT_MAX && sisteTid.vindHastNaa!=FLT_MAX){
      	Naa_Vind_Kode(naaVindKode, 
			   	      sisteTid.vindRetnNaa,
		  			  sisteTid.vindHastNaa);
    }else{
      	naaVindKode="////";
    }
    
    /* Reknar ut max. vindgust kode sidan forrige hovud obs. (12t int.) */
    Max_Vind_Gust_Kode(maxVindGustKode, synopData);

    /* Reknar ut max. vindkast kode sidan forrige hovud obs. (6t int.) */
    Max_Vind_Max_Kode(maxVindMaxKode, synopData);

      //Regner ut gressTempereaturen
    GressTempKode(gressTemp, synopData);

    Trykk_Kode(4,trykkQFFKode, sisteTid.trykkQFFNaa);
    Trykk_Kode(3,trykkQFEKode, sisteTid.trykkQFENaa);
    
    if(!sisteTid.AA.empty() && sisteTid.trykkTendens!=FLT_MAX ){
      	Tendens_Kode(tendensKode, sisteTid);
    }else if(nTimeStr>=4){
		Tendens_Kode(tendensKode,
				     sisteTid.time().hour(),
		    		 synopData[3].trykkQFENaa,
		    		 synopData[2].trykkQFENaa,
		    		 synopData[1].trykkQFENaa,
		    		 sisteTid.trykkQFENaa);
    }

  
    Skydekke_Kode(skydekkeKode, sisteTid.skydekke);
    Hoyde_Sikt_Kode(hoyde_siktKode, sisteTid);
    //SjekkEsss(snoeMarkKode, sisteTid.snoeMark);
    doEsss( snoeMarkKode, sisteTid );
    Sjekk_Gruppe(8, skyerKode, sisteTid.skyer);
    Sjekk_Gruppe(8, skyerEkstraKode1, sisteTid.skyerEkstra1);
    Sjekk_Gruppe(8, skyerEkstraKode2, sisteTid.skyerEkstra2);
    Sjekk_Gruppe(8, skyerEkstraKode3, sisteTid.skyerEkstra3);
    Sjekk_Gruppe(8, skyerEkstraKode4, sisteTid.skyerEkstra4);
    Sjekk_Gruppe(0, sjoeTempKode, sisteTid.sjoeTemp);
    
    synop="\r\r\nZCZC\r\r\n";
    synop+=tidsKode;
    synop+="NO";
    sprintf(tmp,"%02d ", listenummer);
    synop+=tmp;
    
    while(tmpUtsteder.length()<4)
		tmpUtsteder.insert(0," ");

    tmpUtsteder+=" ";
    synop+=tmpUtsteder;
    synop+=dato_tid;
    synop.append("00");

    if(create_CCA_template)
      	synop+=" CCCXXX";

    if(ir==1){
      	if(!(sisteTid.time().hour()%6) && ITR>=1 && ITR<=4)
			nedboerKodeSect1=RRRtr;
      	else if(RRRtr.find("////")!=string::npos)
			ir=4;
      	else{
			ir=2;
			nedboerKodeSect3=RRRtr;
      	}
    }

  
    /**
     * Changed the wind unit from knop to m/s
     * 
     * AAXX DDhhW 
     * 
     * Where DD is the day.
     *       hh is the termin (hour).
     *       W  wind unit, 
     *          W = 4 - knop
     *          W = 1 - m/s
     * 
     * IW is defined in synop.h
     */
    sprintf(tmp,"\r\nAAXX %s%1d", dato_tid.c_str(), IW);
    synop+=tmp;
      
    sprintf(tmp, "\r\n%05d %1d%1d", synopno, ir, ix);
    synop+=tmp;
    synop+=hoyde_siktKode;
    synop+=string(" ")+skydekkeKode+naaVindKode;
    synop+=" 1";
    synop+=luftTempKode+duggTempKode;

    synop+= trykkQFEKode+trykkQFFKode+tendensKode;
    
    if(ir==1 && !nedboerKodeSect1.empty())
      	synopStr+=nedboerKodeSect1;
    
    if(verGenerelt)
		synopStr+=verGenereltKode;
    
    if(skyerKode.length()>0 && 
       skydekkeKode[0]!='0' && 
       skydekkeKode[0]!='9')
		synopStr+=skyerKode;
    
    
    //Seksjon 222
    //Kyststajoner som fï¿½r beskjed
    if(sisteTid.time().hour()==12){
      	if(SjoeTempKode(sjoeTempKode, sisteTid)){
			synopStr+=" 222// ";
			synopStr+=sjoeTempKode;
      	}
    }
    
    
    /**
     * COMMENT:
     * komentert ut rr24Kode intil vi bestemmer oss 
     * for ï¿½ ha den med.
     */
    if(!minMaxKode.empty()       ||
       !nedboerKodeSect3.empty() ||
       !maxVindGustKode.empty()  ||
       !snoeMarkKode.empty()     ||
       !rr24Kode.empty()         ||
       !skyerEkstraKode1.empty() ||
       !skyerEkstraKode2.empty() ||
       !skyerEkstraKode3.empty() ||
       !skyerEkstraKode4.empty()){
      	/* Gruppe 333 */
      	synopStr+=" 333 ";
      	synopStr+=minMaxKode;
      	synopStr+=snoeMarkKode;
      	synopStr+=nedboerKodeSect3;
      	synopStr+=rr24Kode;
      	synopStr+=skyerEkstraKode1;
      	synopStr+=skyerEkstraKode2;
      	synopStr+=skyerEkstraKode3;
      	synopStr+=skyerEkstraKode4;
      	synopStr+=maxVindGustKode;
    }
    
    if(!maxVindMaxKode.empty() || 
       !maxMinKode.empty()     || 
       !verTilleggKode.empty() ||
       !gressTemp.empty()){
      	/* Gruppe 555 */
      	synopStr+=" 555 ";
      	synopStr+=maxVindMaxKode;
      	synopStr+=maxMinKode;
      	synopStr+=gressTemp;
      	synopStr+=verTilleggKode;
    }

    // Avslutting av data-streng 
    synopStr+="=";
   
    // Datastrengen kan ikke vere lenger enn 69 karakterar 
    SplittStreng(synopStr, 69);
    
    synop+=synopStr;
    synop+="\r\n\r\r\n\n\n\n\n\n\n\nNNNN\r\n";
    
    return true;
} /* LagSynop */






/*
** Returnerar  ein streng ; anten "SM","SI",el. "SN"
*/
void 
Synop::Tid_Kode(std::string &kode, int time)
{
   switch(time){
   case 0:
   case 6:
   case 12:
   case 18:
     	kode="SM";
      	break;
   case 3:
   case 9:
   case 15:
   case 21:
      	kode="SI";
      	break;
   default:
      	kode="SN";
      	break;
   }
   return;
} /* LagTidKode */





/*****
 * Bxrge Moe, 20.12.96
 * Bugfix:
 *   Avrundingsfeil. 'vindretningen' ble feil avrundet.
 *   Dette kan tyde paa en bug i matterutinen (math.h) 'rint'. 
 *   0.5 ble avrundet til 0, og ikke 1 som forventet. Denne feilen gjelder 
 *   bare for argument minde enn 1. For argument > 1, ble resultatet korrekt.
 * 
 * Jeg har ogsaa lagt inn en test om vindhastigheten er stoerre enn 98 m/s.
 * Hvis det er tilfellet antas det feil i maalingen. Det samme gjoer vi 
 * dersom vindretningen er stoerre enn 360 grader. Dette kan virke 
 * unoedvendig siden dette ogsaa blir testet og skal vaere korrigert paa 
 * stasjonen (Grensefeil). Men det kan forekomme grensefeil i dataene 
 * fra stasjonene. 
 *
 * Bxrge Moe, 26.06.97
 * Lagt inn korrigering for maaleunoeyaktighet i minimumsverdiene for
 * FF og DD.
 *     FF < FF(min) -> FF=0.0
 *     DD < DD(min) -> DD=360
 *
 * FF(min) og DD(min) er satt til
 *    FF(min)=0.1 m/s    og    DD(min)=1 gr
 *
 * Dette i hennhold til anbefalinger fra instrument avdelingen. 
 * Ved Ragnar Breakkan.
 *
 * Foelgende grenser er definert for automatstasjoner:
 *  vindhastighet: [0,98]  (m/s)
 *  vindretning:   [0,360] (grader)
 *
 * Synopkode gruppe 3 har foelgende form:
 *   Nddff 
 *   Hvor:
 *     N - Samlet skydekke. Angis ikke for automatstasjoner.
 *         (angis som / ). For hybridstasjoner kan denne verdien
 *         tastes inn av opperatoer ("skytitter").
 *    dd - Vindretningen. Angis i nermeste 10'de grad. 
 *         Gyldige verdier [00,36]. Verdien 00 gis for vindstille.
 *    ff - Vindhastighet i knop. Gyldige verdier [0,99]. Hvis vindhastigheten
 *         er mindre enn 1 knop settes dd=00. slik at synopen blir /0000.
 *         Hvis vindhastigheten er stoerre enn 99 knop. Faar vi en ekstra
 *         gruppe umiddelbart etter. Synopen blir da /dd99 00fff, fff er 
 *         vinden i knop.
 *
 * Parameterene 'retn' og 'hast' til funksjonen er gitt i grader og m/s.
 * 'hast' maa omregnes til knop foer den settes i synop koden.
 *  
 ******/

void 
Synop::Naa_Vind_Kode(std::string &kode, float retn, float hast)
{  
  	char tmp[30];
  	
  	kode="////";
  
  	if( hast < 0.1 )
    	hast = 0.0;
  
  	if( retn < 1 )
    	retn = 360;
  
  	if( fabs(retn) > 360){ /* Grensefeil */
    	if(hast > 98 ){     /* Grensefeil */
    		kode="////";
    	}else{
    		hast *=KNOPFAKTOR;  
    		hast = floor( (double) hast+0.5);
      
    		if(hast<1.0)
    			kode="0000";
    		else if(hast >= 99.0){
    			sprintf(tmp,"//99 00%03.0f",hast);
				kode=tmp;
    		}else{
				sprintf(tmp,"//%02.0f",hast);
				kode=tmp;
    		}
    	}
  	}else{
    	if(retn>=5)
    		retn = floor( (double)(retn/10)+0.5 );
    	else   
    		retn = 36;
    
    	if(fabs(hast) > 98 ){     /* Grensefeil */
    		sprintf(tmp,"%02.0f//",retn);
    		kode=tmp;
    	}else{
    		hast *= KNOPFAKTOR;
    		hast = floor( (double) hast+0.5);
      
    		if(hast<1.0)
				kode="0000";
    		else if(hast>=99.0){
				sprintf(tmp,"%02.0f99 00%03.0f",retn,hast);
				kode=tmp;
    		}else{
				sprintf(tmp,"%02.0f%02.0f",retn,hast);
				kode=tmp;
    		}
    	}
  	}
}



/*
** Lagar temperaturkode pï¿½ format SnTTT der Sn er forteikn
*/
void 
Synop::Temp_Kode(std::string &kode, float temp)
{
 	char stmp[30];
  
  	kode="////";
  
  	/* Ugyldige verdiar */
  	if(temp==FLT_MAX)
    	return;
  
  
  	if((temp>=0.0)&&(temp<200.0)){
    	/* Positiv verdi (0 som prefix)*/
    	sprintf(stmp,"0%03.0f",fabs(rint((double)temp*10)));
  	}else if((temp<0.0)&&(temp>-200.0)){
    	/* Negativ verdi (1 som prefix)*/
    	sprintf(stmp,"1%03.0f",fabs(rint((double)temp*10)));
  	}else{
    	return;
  	}

  	kode=stmp;
} /* Temp_Kode */


/*
 * Regner ut duggtemperaturen vha. formel
 * 
 * Bxrge Moe,  30.10.97
 * Pga. av maaleusikkerhet ved maaling av relativfuktighet kan maale
 * verdiene vaere stoerre enn 100%. Klimaavdelingen har bestemt foelgende
 * haandtering av verdier for relativfuktighet stoerre enn 100%:
 *
 * Hvis sensorverdien for relativ fuktighet er element i <100,104],
 * settes dugpunkt temperaturen lik maalt temperatur. Hvis verdien
 * for relativfuktighet er stoerre enn 104. Krysses verdien for
 * duggpunkt teperaturen ut (2////)
 *
 * Bxrge Moe, 1999.2.17
 * -Buggfix for feil i behandlingen av fuktighet stï¿½rre enn 100%.
 * 
 * Bxrge Moe, 2006-07-12
 * -NÃ¥r duggpunktet beregnes marginalt hÃ¸yere, 0.5 grader C, enn 
 *  lufttemperaturen sÃ¥ settes duggpunket lik lufttemperaturen.
 * -Ryddet opp i tÃ¥pelig og ulogisk kode.
 *  
 */
void 
Synop::Dugg_Kode(std::string &kode, float temp, float fukt)
{
	char stmp[30];
   int index;
   float CK[9] = {
   	6.10714,
	  	22.44294,
		272.440,
		6.10780,
		17.84362,
		245.425,
		6.10780,
		17.08085,
		234.175};
	float SVP;
   float VP;
   float Q1;
   float td;
   std::string str;

   index = 0;

   if(fukt==FLT_MAX || temp==FLT_MAX){
     	kode=" 2////";
     	return;
   }

   
   LOGDEBUG("duggtemp: UU=" << fukt << "  TA=" << temp);

  	if(fukt>100.0){
   	LOGDEBUG("duggtemp: fukt(" << fukt << ")>100");
     
		if(fukt<=104){
	    	Temp_Kode(str, temp);
	    	sprintf(stmp, " 2%s", str.c_str());
	    	kode=stmp;
		}else{
	  		kode=" 2////";
		}
		return;
	}

   if(temp>0.0)
     	index = 6;
    
	/* Saturation vapor pressure */
   SVP =  CK[index]*exp(CK[index+1]*temp/(CK[index+2]+temp));
       
   /* Actual vapor pressure */
   VP = fukt*SVP/100;
    
   /* Dewpoint temperature */
   Q1 = log(VP/CK[index]);
   td = CK[index+2]*Q1/(CK[index+1]-Q1);

   LOGDEBUG("duggtemp: " << endl     <<
	  	 	 	"-- SVP=" << SVP << endl <<
	  	 	 	"--  VP=" <<  VP << endl <<
	  	 	 	"--  Q1=" <<  Q1 << endl <<
	  	 	 	"--  TD=" <<  td);

 	if(td>temp){
   	if(td<(temp+0.5)){
   		td=temp; 
   	}else{
     		kode=" 2////";
     		return;
   	}
   }

   Temp_Kode(str,td);
   kode=" 2";
   kode+=str;
} /* Dugg_Kode */

/* 25.11.97
 * Bxrge Moe
 *
 * Regner ut nattens minimumstemperatur hvis klokken er 06,
 * eller dagens maksimumstemperatur hvis klokken er 18.
 *
 * Rutinen er endret slik at vi ikke trenger 24 timer tilbake 
 * i tid med data.
 */
void 
Synop::Min_Max_Kode(std::string &kode, SynopDataList &sd)
{
    int         nTimeStr=sd.nContinuesTimes();
    float       min;
    float       max;
    std::string str;

    min =  200.0;
    max = -200.0;

    if(nTimeStr<12){
      	switch(sd[0].time().hour()){
      	case 6:
			if(sd[0].TAN_12==FLT_MAX)
	  			return;
	
			Temp_Kode(str, sd[0].TAN_12);
			kode=" 2";
			kode+=str;
			return; 
	
      	case 18:
			if(sd[0].TAX_12==FLT_MAX)
	  			return;
	
			Temp_Kode(str, sd[0].TAX_12);
			kode=" 1";
			kode+=str;
			break;
	
      	default:
			return;
      	}
      
      	return;
    }


    switch(sd[0].time().hour()){
    case 6: /* Nattens minimumstemp. */
		for(int i=0; i<12; i++){
	  		if(sd[i].tempMin==FLT_MAX)
	    		return;
	  
	  		if(sd[i].tempMin<min)
	     		min=sd[i].tempMin;
		}
	
		if(sd[0].tempNaa<min)
	  		min=sd[0].tempNaa;
	
		/* Ved feil paa data vil min ha defaultverdien 200 */
		if(min>199.0)
	    	return;
	
		/* Temperatur kode paa format TTT */         
		Temp_Kode(str, min);
		kode=" 2";
		kode+=str;
		return;
	
    case 18: /* Dagens maksimumstemp. */
		for(int i=0; i<12; i++){
	  		if(sd[i].tempMax==FLT_MAX)
	    		return;
	  
	  		if(sd[i].tempMax>max)
	    		max=sd[i].tempMax;
		}

		if(sd[0].tempNaa>max)
	    	max = sd[0].tempNaa;         

		/* Ved feil paa data vil max ha defaultverdien -200 */
		if(max<-199.0)
	    	return;
	
		Temp_Kode(str, max);
		kode=" 1";
		kode+=str;
		return;
	
    	default:
      		return;
    }
} /* Min_Max_Kode */


/* 25.11.97
 * Bxrge Moe
 *
 * Regner ut nattens maksimumstemperatur hvis klokken er 06,
 * eller dagens minimumstemperatur hvis klokken er 18.
 *
 * Rutinen er endret slik at vi ikke trenger 24 timer tilbake 
 * i tid med data.
 *
 * nTimeStr holder antall timer med data vi har.
 *
 */
void 
Synop::Max_Min_Kode(std::string &kode, SynopDataList &sd)
{
    int nTimeStr=sd.nContinuesTimes();
    float min;
    float max;
    std::string str;

    min =  200.0;
    max = -200.0;

    kode.erase();

    if(nTimeStr<12){
      	switch(sd[0].time().hour()){
      	case 6:
			if(sd[0].TAX_12==FLT_MAX)
	  			return;
	
			Temp_Kode(str, sd[0].TAX_12);
			kode=" 1";
			kode+=str;
			break; 
	
      	case 18:
			if(sd[0].TAN_12==FLT_MAX)
	  			return;
	
			Temp_Kode(str,sd[0].TAN_12);
			kode=" 2";
			kode+=str;
			break; 
	
      	default:
			return;
      }
      
      return;
    }
    
    switch(sd[0].time().hour()){
    case 6:
      	for(int i=0; i<12; i++){
			if(sd[i].tempMax==FLT_MAX)
	  			return;
	
		if(sd[i].tempMax>max)
	  		max = sd[i].tempMax;
      	}
      
      	if(sd[0].tempNaa>max)
			max = sd[0].tempNaa;         
      
      	if(max<-199.0)
			return;
            
      	Temp_Kode(str,max);
      	kode=" 1";
      	kode+=str;
      	return;
      
	case 18:
      	for(int i=0; i<12; i++){
			if(sd[i].tempMin==FLT_MAX)
	  			return;
	
			if(sd[i].tempMin<min)
	  			min=sd[i].tempMin;
      	}
      
      	if(sd[0].tempNaa<min)
			   min=sd[0].tempNaa;         
        
      	if(min>199.0)
			return;
            
      	Temp_Kode(str,min);
      	kode=" 2";
     	kode+=str;
      	return;
      
    default:
    	return;
    }
} /* Max_Min_Kode */


/* 16.01.98
 * Bxrge Moe
 *
 * Beregning av Maksimalt vindkast (FG), maksimalvind (FX) 
 * siden forrige hovedobservasjon og vinden ved synoptidspunkt (FF).
 * ( FG - Seksjon 333 911, FX - Seksjon 555 0 og FF - alle stasjoner Nddff )
 * -------------------------------------------------------------------------
 * 
 * Noen definisjoner.
 *    Automatstasjonene leverer parameterene ff, fx og fg hver time.
 *    Tilsvarende verdier for synoptidspunktene er FF, FX og FG.
 *
 *    hvor
 *       ff - 10 minutters middel. Beregnet 10 minutter foer hver hele time.
 *       fx - er hoeyeste glidende 10 minutters middelverdi i en 69 minutters
 *            periode. ( (tt-2):51-tt:00, tt angir timen for beregningen.)
 *       fg - er hoeyeste glidende 3 sekunders middelverdi i loepet av en
 *            60 minutters periode. ((tt-1):00-tt:00, tt - timen for beregn.)
 *
 *    Synoptidene er 0, 3, 6, 9, 12, 15, 18, og 21. Hvor hovedtidene er
 *    0, 6, 12 og 18. Mellomtidene er 3, 9, 15 og 21.
 *
 *    La XX representere hovedtidene, og xx mellomtidene. 
 *
 * Beregningen av FF, FX og FG er gitt som foelger.
 *
 * 1) FF er enten ff(XX) eller ff(xx).
 * 2) FX(XX) er hoeyeste fx av timeverdiene XX, XX-1, XX-2, XX-3, XX-4, XX-5 og
 *    FF(XX-6) (NB! ff(XX-6), ikke fx(XX-6)).
 * 3) FX(xx) er hoeyeste fx av timeverdiene xx, xx-1, xx-2 og FF(xx-3).
 *    (NB! ff(xx-6), ikke fx(xx-6)) 
 * 4) FG(XX) er hoeyeste fg av timeverdiene XX, XX-1, XX-2, XX-3, XX-4 og XX-5.
 * 5) Dersom FG<FX, settes FG=FX.
 *
 * Koden for FG er gitt i rutinen 'Max_Vind_Gust_Kode' og koden for
 * FX er gitt i rutinen 'Max_Vind_Max_Kode'.
 */

/* 16.01.98
 * Bxrge Moe
 *
 * Synopgruppe: 333 911ff  (FG)
 *
 * Beregner maksimalt vindkast siden forrige hovedsynoptid.
 *
 * nTimeStr er en global variabel som holder antall timer med data vi har.
 * nTimeStr settes  i rutinen LagTabell.
 *
 * 'tab' er en array paa 24 element. nTimeStr elementer er gyldig. For aa
 * kunne beregne FG trenger vi 6 timestrenger.
 *
 * 13.03.98 
 * Bxrge Moe
 * Lagt til stoette for manuell intasting av av FG. Den intastede verdien
 * ligger i variabelen _fg (DATASTRUCTTYPE1._fg) og er gitt i knop. Hvis
 * _fg ikke er gitt er lengden av _fg lik 0. (_fg er deklarert som en char _fg[20]).
 *
 * Hvis det er nok time verdier til aa beregne FG, har disse
 * prioritet forran den manuelle hvis den finnes.
 */

/* 18.11.98
 * Bxrge Moe
 *
 * Rettet bugg for generering av Gust for 'pio'.
 */ 
void 
Synop::Max_Vind_Gust_Kode(std::string &kode, SynopDataList &sd)
{
    int   nTimeStr=sd.nContinuesTimes();
    float fMax;
    std::string::iterator it;
    char  stmp[30];

    fMax=-1.0;
    kode.erase();
    
    if((sd[0].time().hour())%6 != 0)
		return;

    if(nTimeStr<6){
     	if(sd[0].FG==FLT_MAX || sd[0].FG<0)
     		return;

      fMax=sd[0].FG;
      fMax *= KNOPFAKTOR;
      fMax = floor((double) fMax + 0.5);
      
      if(fMax>=99.0 && fMax<=176){
      	sprintf(stmp, " 91199 00%03.0f", fMax);
      	kode=stmp;
      }else if(fMax<99.0){
      	sprintf(stmp, " 911%02.0f", fMax);
      	kode=stmp;
      }

      return;
    }

    

    for(int i=0; i<6; i++){
      if(sd[i].vindHastGust==FLT_MAX)
			return;
      
      if(sd[i].vindHastGust>fMax)
			fMax=sd[i].vindHastGust;
    }

    if(fMax<0)
      return;
    
    fMax *= KNOPFAKTOR;
    fMax = floor((double) fMax+0.5);
    
    if(fMax>=99.0)
     	sprintf(stmp, " 91199 00%03.0f", fMax);
    else
     	sprintf(stmp, " 911%02.0f", fMax);

    kode=stmp;
}


/* 16.01.98
 * Bxrge Moe
 *
 * Synopgruppe 555 0STzFxFx
 *
 * Regner ut maksimal middelvind siden forrige hovedtermin,
 * dvs. kl. 0, 6, 12  eller 18, og hvor mange timer siden det inntraff. 
 *
 * nTimeStr angir antall timer med kontinuerlig data vi har.
 *
 *
 * 13.03.98 
 * Bxrge Moe
 *
 * Lagt til stoette for observert TzFXFX. De intastede verdien
 * ligger i variabelene ITZ og FX. FX er gitt i 
 * m/s.  
 *
 * Hvis det er nok time verdier til aa beregne tzFXFX, har disse
 * prioritet forran den observerte hvis den finnes.
 *
 * 2005.11.24
 * Bxrge Moe
 * Lagt inn stÃ¸tte for FX_3.
 */
void 
Synop::Max_Vind_Max_Kode(std::string &kode, SynopDataList &sd)
{
   int   nTimeStr=sd.nContinuesTimes();
   int   nNeedTimes;
   float fMax;
   int   iMax;
   int   iNaaMax;
   std::string  sjoegangKode;
   int   iMaxIndex;
   int   iTidsAngiv;
   int   i;
   char  cTid;
   std::string sff;
   char stmp[30];

   kode.erase();
   nNeedTimes=3;
   fMax=-1.0;

   if((sd[0].time().hour())%3 != 0)
      return;
       
   if((sd[0].time().hour())%6 == 0)
      nNeedTimes=6;
    
   if(sd[0].sjoegang.length()!=1)
      sjoegangKode="/";
   else
      sjoegangKode=sd[0].sjoegang;
    
   if(nTimeStr < nNeedTimes){
      fMax=FLT_MAX;
      cTid='/';
    	
      if(sd[0].FX!=FLT_MAX && sd[0].FX>=0){
         fMax=sd[0].FX;
      }else if(sd[0].FX_3!=FLT_MAX && sd[0].FX_3>=0){
         fMax=sd[0].FX_3;
    		
         if((sd[0].time().hour()%6)==0){ //Hovedtermin!
            miutil::miTime prevTime=sd[0].time();
            prevTime.addHour(-3);
            CISynopDataList it=sd.find(prevTime);
    			
            if(it!=sd.end() && it->time()==prevTime &&
               it->FX_3!=FLT_MAX && it->FX_3>=0){
    			 
               if(it->FX_3>fMax)
                  fMax=it->FX_3;
            }else{
               fMax=FLT_MAX;
            }
         }
      }
    			   
      if(fMax==FLT_MAX){
         if(sjoegangKode!="/") {
            kode=" 0";
            kode+=sjoegangKode;
            kode+="///";
         } 
	
         return;
      }

      fMax*=KNOPFAKTOR;
      
      if(sd[0].ITZ.length()>0){
         if(isdigit(sd[0].ITZ[0]))
            cTid=sd[0].ITZ[0];
      }   

      //Guard against rounding error.
      fMax = floor( (double) fMax+0.5);
      
      if(fMax>=0 && fMax<176){
         if(fMax>=99)
            sprintf(stmp, "99 00%03.0f", fMax);
         else
            sprintf(stmp, "%02.0f", fMax);

         sff=stmp;
      }else{
         sff="///";
      }

      kode=sjoegangKode;
      kode+=cTid;
      kode+=sff;
      kode.insert(0, " 0");
      
      return;
   }
    
   i=nNeedTimes-1;
    
   if(sd[i].vindHastMax==FLT_MAX ){
      kode=" 0";
      kode+=sjoegangKode;
      kode+="///";
      return;
   }
    
   fMax=sd[i].vindHastMax;
   iMaxIndex=i;
   i--;
    
   while(i>=0){
      if(sd[i].vindHastMax==FLT_MAX){
         kode=" 0";
         kode+=sjoegangKode;
         kode+="///";
         return;
      }
      
      if(sd[i].vindHastMax>=fMax){
         fMax=sd[i].vindHastMax;
         iMaxIndex=i;
      }

      i--;
   }
   
   if(fMax<0){
      kode=" 0";
      kode+=sjoegangKode;
      kode+="///";
      return;
   }

   if(iMaxIndex<3)
      iTidsAngiv=iMaxIndex+1;
   else if(iMaxIndex<6)
      iTidsAngiv=4;
   else if(iMaxIndex<9)
      iTidsAngiv=5;
   else if(iMaxIndex<12)
      iTidsAngiv=6;
   else{
      kode=" 0";
      kode+=sjoegangKode;
      kode+="///";
      return;
   }
   
   iMax = (int)floor((double) fMax*KNOPFAKTOR+0.5);
   iNaaMax = (int) floor((double) sd[0].vindHastNaa*KNOPFAKTOR );
 //  iMax= (int)rint(fMax*KNOPFAKTOR);
 //  iNaaMax=(int)rint( sd[0].vindHastNaa*KNOPFAKTOR);
    
   if(iTidsAngiv==1){
      if(iMax==iNaaMax)
         iTidsAngiv=0;
   }
    
   kode=" 0";
   kode+=sjoegangKode;

   if(iMax>=99)
      sprintf(stmp, "%1d99 00%03d", iTidsAngiv, iMax);
   else
      sprintf(stmp, "%1d%02d", iTidsAngiv, iMax);

   kode+=stmp;
}

/*
 * 22.01.98
 * Bxrge Moe
 *
 * Rettet bugg i koden for generering av synopkoden for trykk.
 * Har ogsaa lagt til test for aa se om trykkverdien er 'mulig'.
 * trykk maa vaere element i [800,1100]. (jfr. klimaavdelingen)
 */
void 
Synop::Trykk_Kode(int prefix, std::string &kode, float trykk)
{
   	double dTrykk;
  	char   stmp[30];

   	kode.erase();

   	if(trykk==FLT_MAX)
     	return;

   	if(trykk<800.0 || trykk>1100.0)
   		return;
        
   	dTrykk=trykk*10;
   	dTrykk=rint(dTrykk);
   
   	if(dTrykk>=10000.0)
    	dTrykk = dTrykk -10000.0;
   
   	sprintf(stmp," %1d%04.0f",prefix, dTrykk);
   	kode=stmp;

} /* Trykk_Kode */


/*Bxrge Moe
 *27.01.98
 *
 *Rettet en liten bugg i koden for tedens angivning.
 *
 */
void 
Synop::Tendens_Kode(std::string &kode, 
		  int time, 
		  float trykk1, 
		  float trykk2, 
		  float trykk3, 
		  float trykk4)
{
   	int a;
   	float dP1;
   	float dP3;
   	float PD3;
   	float t1;
   	float lim;
   	char stmp[30];

   	a      = 4;
   	dP1    = trykk2 - trykk1;
   	dP3    = trykk4 - trykk3;
   	PD3    = trykk4 - trykk1;
   	t1     =    dP3 - dP1;
   	lim    = 0.01;

   	kode="";

          /* Ved ulovlege verdiar */   
   	if(trykk1==FLT_MAX ||
       trykk2==FLT_MAX ||
       trykk3==FLT_MAX ||
       trykk4==FLT_MAX){
     /* Bxrge Moe
      * 5.8.2002
      * 
      * Endret return 'kode' til en tom streng nï¿½r verdiene
      * er ugyldig.
      */
      //kode=" 5////";
   		kode="";
      	return;
   	}

   	if(PD3 > lim){
     	if(t1 < (-1*lim)){
       		if(dP3 < (-1*lim)) /*27.01.98 Bxrge Moe*/
				a = 0;
       		else
	 			a = 1;
     	}else{
       		if(t1 > lim)
	 			a = 3;
       		else
	 			a = 2;
     	}
   	}else if(PD3 < (-1*lim)){
     	if(t1 > lim){ /*27.01.98 Bxrge Moe*/
       		if(dP3 > lim)
	 			a = 5;
       		else
	 			a = 6;
     	}else if(t1 < (-1*lim))
       		a = 8;
     	else
       		a = 7;
   	}else if((PD3<=lim) && (PD3>=(-1*lim))){
     	if(t1 > lim)
       		a = 5;
     	else if(t1<(-1*lim))
       		a = 0;
     	else
       		a = 4;
   	}else
    	a = 4;
   
   	sprintf(stmp," 5%1d%03.0f",a,fabs(rint((double)(PD3*10))));
   
   	kode=stmp;
} /* Tendens_Kode */

/* 13.03.98
 * Bxrge Moe
 *
 * Prosedyren lager tedenskode fra manuelt intastede verdier. Kan ogsaa
 * endres til aa bruke verdier for trykktendens og karakteristikk beregnet
 * paa stasjonsnivaa.
 *
 * Trykktendensen er git med parameteren DATASTRUCTTYPE1.trykkTendens og
 * trykkarakteristikk er gitt med DATASTRUCTTYPE1._aa
 */
void 
Synop::Tendens_Kode(std::string &kode, const SynopData &data)
{
  	char karakter;
  	char stmp[30];
  
  	if(data.AA.length()==0)
    	karakter='/';
  	else
    	karakter=data.AA[0];
  
  
  	if(data.trykkTendens==FLT_MAX){
    	if(karakter=='/'){
      		kode="";
      		return;
    	}
    
    	kode=" 5";
    	kode+=karakter;
    	kode+="///";
    	return;
  	}

  	sprintf(stmp, " 5%c%03.0f", karakter, fabs(rint((double)(data.trykkTendens*10))));
  
  	kode=stmp;
  
  	if(kode.length()!=6){
    	if(karakter=='/'){
      		kode="";
      		return;
    	}
    
    	kode=" 5";
    	kode+=karakter;
    	kode+="///";
  	}
}





/*
**
*/
void 
Synop::Skydekke_Kode(std::string &kode, const std::string &str)
{
   	if(str.length()!= 1)
      	kode="/";
   	else if(isdigit(str[0]) || str[0] == '/')
      	kode=str;
   	else
      	kode="/";

} /* Skydekke_Kode */

/*
**
*/
void 
Synop::Hoyde_Sikt_Kode(std::string &kode, const SynopData &data)
{
   float Vmor=data.Vmor;
   float VV=data.VV;
   float HLN=data.HLN;
   float HL=data.HL;

   kode="///";
   
	if( VV == FLT_MAX && Vmor!=FLT_MAX )
	   VV = Vmor;
	
	if( HL == FLT_MAX && HLN != FLT_MAX )
	   HL = HLN;
	   
	if( HL != FLT_MAX ) {
	   char ch=decodeutility::HLKode( HL );
	   kode[0]=ch;
	}
	
	if( VV != FLT_MAX ) {
	   string s=decodeutility::VVKode( VV );
	   
	   if( ! s.empty() )
	      kode.replace(1, 2, s);
	}
}


/*
**
*/
int 
Synop::ix_Kode(const std::string &str)
{
  	if(str.length()!=2 || !(isdigit(str[1])))
    	return 6;
  
  	return atoi(&(str.c_str())[1]);

} /* ix_Kode */


/**
 * Funksjonen lager koden 7wwW1W2 i seksjon 1, samt setter ix.
 * Manuelt observert ww gÃ¥r foran automatisk generert ww.
 * 
 * Den manuelt observerte ww ligger i datasettet som verGenerelt og
 * den automatiske ligger som WAWA.
 *  
 */
bool 
Synop::doVerGenerelt(std::string &kode, int &ix, const SynopData &data)
{
	bool verGenerelt;
	
	kode.erase();
	
	ix = ix_Kode(data.nedboerInd_verInd);
   verGenerelt = Sjekk_Gruppe(7, kode, data.verGenerelt);
    
   if(verGenerelt){
   	ix=1; 
   }else if(data.WAWA!=FLT_MAX){
   	char buf[10];
   	int i=static_cast<int>(round(data.WAWA));
   	
   	if(i>=0 && i<100){
    		sprintf(buf, " 7%02d//", i);
    		kode=buf;
    		ix=7;
    		verGenerelt=true;
   	}else{
   		ix=6;
   	}
   }
   
   if(ix==1 && !verGenerelt)
   	ix=6;
   
   return verGenerelt;
}	
 

/**
 * Bï¿½rge Moe
 * 1999.06.10
 *
 * Endringer av tolkning av Esss gitt av klimaavdelingen.
 * En Esss kode pï¿½ formen /000 skal ikke tas med i synopen
 * men den angir en lovlig verdi for koden. I fï¿½lge synop 
 * kodingen (spec) er ikke sss=0 en lovlig verdi, men er tillatt
 * slik at man kan sende Esss hele ï¿½ret, ogsï¿½ nï¿½r det ikke er
 * snï¿½.
 */
bool 
Synop::SjekkEsss(std::string &kode, const std::string &str)
{
  	std::string::const_iterator it;
  
  	if(str.length()!=4 || str=="////" || str=="/000"){
    	kode="";
    	return false;
  	}else{
    	it=str.begin();
    
    	while(it!=str.end()){
     		if(!(isdigit(*it) || *it == '/')){
				kode="";
				return false;
     		}
     		it++;
    	}
    
    	kode=" 4";
    	kode+=str;
  	}
  
  	return true;
}


/**
 * Coding of E'sss.
 * 
 * The coding of E'sss is dependent on EM and SA.
 * 
 * EM -> E
 * SA -> sss
 * 
 * Når en værstasjon sender 998 vil de enten også sende E'= 1. (Hvis de har
 * utelatt E' må vi dekode E' til 1 siden 998 er en såpass bevisst handling.)
 *
 * Altså, det er E' som bestemmer om SA=-1 er flekkvis snø. I koding av synop
 * må en altså for alle typeid bruke kombinasjonen av SA og E' eller SA og SD 
 * for å kunne angi 998 i synop.
 * SA=-1 når snødybde raporteres som "blank", utelatt (gruppe) eller "0" (Ingen
 * snø)
 * SA=-1 når snødybde raporteres som 998             (flekkvis snø)
 * SA=0  når snødybde raporteres som 997             (mindre enn 0.5 cm snø)
 * SA=-3 når snødybde raporteres som 999             (måling umulig)
 * EM=-1 når EM raporteres som "blank" eller utelatt (gruppe) 
 * EM=0  er is-lag
 * EM= 1 - 9 er andel snødekke og type
 *
 * Synop enkoding fra Kvalobs
 * Når det skal lages synop fra kvalobs så må det kanskje ut fra dette til en 
 * justering av dagens enkoder slik at koding av SA og EM blir riktig? (For 302
 * må kun SA benyttes i synop - ikke SD.) 
 * 
 */

void 
Synop::
doEsss( std::string &kode, const SynopData &data )
{
   kode.erase();
   
   char buf[16];
   string em;
   string sa;
   int    iSA;
   
   if( data.SA == FLT_MAX )
   	iSA = INT_MAX;
   else
   	iSA = (int) floor((double) data.SA + 0.5 );
   
   if( data.EM == FLT_MAX && iSA == INT_MAX )
      return;
   
   if( data.EM == FLT_MAX || data.EM < 0 || data.EM > 10 )
      em = "/";
   else {
      sprintf( buf, "%01.0f", data.EM );
      em = buf;
   }
   
   if( iSA == INT_MAX  || iSA < -3 || iSA > 996 )
      sa = "///";
   else if( iSA == -1 ) {
      if( em =="/" )
         sa = "///";
      else
         sa = "998";
   }else if( iSA == 0 )
      sa = "997";
   else if( iSA == -3 )
      sa = "999";
   else {
      sprintf( buf, "%03d", iSA );
      sa = buf;
   }

      
   //Creates the code 4E'sss
   kode = " 4" + em + sa;
}


/*
** Sjekkar kode for grupper med 4 teikn;
** maa innehalde anten tal eller '/'
*/
bool
Synop::Sjekk_Gruppe(int grpNr, std::string &kode, const std::string &str)
{
  	std::string::const_iterator it;
  	char tmp[30];
  
  	//std::cerr <<  "Sjekk_Gruppe (in) grpnr(" << grpNr << "): " << str << "\n";  

  	if(str.length()!=4 || str=="////"){
    	kode="";
    	//std::cerr <<  "Sjekk_Gruppe (err1): \n";  
    	return false;
  	}else{
    	it=str.begin();
    	while(it!=str.end()){
      
     		if(!isdigit(*it) && *it != '/'){
				kode="";
				//std::cerr <<  "Sjekk_Gruppe (err2): \n";  
				return false;
     		}
      		it++;
    	}
  
    	sprintf(tmp," %1d%4s",grpNr,str.c_str());
    	kode=tmp;
  	}
  
  	//std::cerr <<  "Sjekk_Gruppe (ut): " << kode << "\n";  
  	return true;

} /* Sjekk_Gruppe */

/*
** Sjekkar den manuelt inntasta koda "ir"
*/
int 
Synop::Vis_ir_Kode(const std::string &str)
{
   	std::string s;
   	std::string::iterator it;

   	s=str;
   	it=s.begin();
   
   	if(it==s.end())
    	return 4;
   
   	if(s.size()!=2 || !(isdigit(*it)))
      	return 4;

   	s.erase(1);

   	return atoi(s.c_str());

} /* Vis_ir_Kode */

bool 
Synop::SjoeTempKode(std::string &kode, const SynopData &data)
{
  	kode="";
  
  	if(data.time().hour()!=12)
    	return false;
  
  	if(data.sjoeTemp.length()>0){
    	if(data.sjoeTemp=="65535"){
      		kode="0////";
      		return false;
    	}
    
    	kode="0";
    	kode+=data.sjoeTemp;
    	return true;
  	}
  
  	if(data.TW==FLT_MAX){
    	kode="0////";
    	return false;
  	}
  
  	Temp_Kode(kode, data.TW);
  
  	if(kode=="0////")
    	return false;
  
  	kode.insert(0, "0");
  
  	return true;
}

/**
 * GressTemperaturen angis klokken 06, og angir minimums
 * temperaturen de siste 12 timene. 
 * For automatiske mï¿½lteverdier trenger vi 12 timestrenger,
 * parameteren har navnet TGN.
 *
 * For manuelt mï¿½lt verdi har parametern navnet TGN_12 og
 * vi trenger bare en timestreng.
 */
void 
Synop::GressTempKode(std::string &kode, SynopDataList &sd)
{
  	int nTimeStr=sd.nContinuesTimes();
  	float min;
  	int i;
  	std::string str;
  
  	min=FLT_MAX;
  
  	kode.erase();
  
  	if(sd[0].time().hour()!=6)
    	return;
  
  	if(nTimeStr<13){
    	if(sd[0].TGN_12==FLT_MAX)
      		return;
    
    	Temp_Kode(kode, sd[0].TGN_12);
    	kode.insert(0, " 3");
    
    	return;
  	}
  
  	i=0;
  
  	while(i<12){
    	if(sd[i].TGN!=FLT_MAX && sd[i].TGN<min)
      		min=sd[i].TGN;
    
    	i++;
  	}
  
  	if(min==FLT_MAX)
    	return;
  
  	Temp_Kode(kode, min);
  	kode.insert(0, " 3");
  
  	return;
}


/*
 * Funksjonen lagar nedboerkoda for nedboermengde i perioden.
 *
 * Lagt til ir for ï¿½ styre kodingen av RRRtr.
 * 
 * ir==1, det har falt mï¿½lbar nedbï¿½r.
 * ir==3, tï¿½rt, det har ikke falt nedbï¿½r, kode ==> 6000tr 
 * ir==4, 
 */

void
Synop::Nedboer_Kode(std::string &kode,  //RRRtr
		    		std::string &verTilleggKode, //555 ... 4RtWdWdWd
		    		std::string &rr24Kode,       //333 ... 7RR24
		    		float totalNedboer, 
		    		float fRR24,
		    		int time,
		    		int &tr,
		    		int ir)
{
  	double dummy;
  	float  nedboerTiDel=0.0;
  	char   stmp[30];

  	kode.erase();
  	rr24Kode.erase();

  	if(time==6){
    	//Skal vi kode 24 (7RR24) timers nedbï¿½r i 333 seksjonen 
    	if(fRR24!=FLT_MAX){
      		if(fRR24<0){
				rr24Kode=" 70000";
      		}else{
				fRR24*=10;

				if(fRR24>9999.5){
	  				rr24Kode=" 7////";
				}else{
	  				sprintf(stmp," 7%04.0f",fabs(floor((double)fRR24+0.5)));
	  				rr24Kode=stmp;
				}
      		}
    	}else{
      		rr24Kode=" 7////";
    	}
  	}
	  
  	if(tr<0 || tr>9){
    	if((time==0)||(time==12))
      		tr=1;
    	else if((time==6)||(time==18))      
      		tr=2;
    	else
      		tr=5;
  	}
  
  	if(verTilleggKode.length()!=6)
    	verTilleggKode=" 4////";

  	if(ir==1){
    
    	//Ugyldig verdi
    	if(totalNedboer==FLT_MAX || totalNedboer<=-2.0){
      		kode=" 6////";
      		return;
    	}else if(totalNedboer<0.0){ //Tï¿½rt
      		sprintf(stmp, " 6000%1d", tr);
      		kode=stmp;
      		verTilleggKode[2]='0';
      		return;
    	}else if(totalNedboer<1.0){
      		sprintf(stmp," 699%1.0f%1d",fabs(floor((double)totalNedboer*10+0.5)),tr);
    	}else if((totalNedboer>=1.0)&&(totalNedboer<989.0)){
      		sprintf(stmp," 6%03.0f%1d",fabs(floor((double)totalNedboer+0.5)), tr);
    	}else{ // totalNedboer>=989.0
      		sprintf(stmp," 6989%1d", tr);
    	}

    	kode=stmp;
    	nedboerTiDel = 10*modf((double)totalNedboer, &dummy);
    
    	//verTilleggKode er pï¿½ formen:
    	// ' 4RtWdWdWd', det er Rt vi skal sette. Rt har indeks=2.
    	//  012 3 4 5 , indekser.
    	sprintf(stmp, "%1.0f", nedboerTiDel);
    	verTilleggKode[2]=stmp[0];
  	}else if(ir==3){
    	sprintf(stmp, " 6000%1d", tr);
    	kode=stmp;
    	verTilleggKode[2]='0';
  	}else if(ir==4){
    	kode=" 6////";
  	}else{
    	LOGWARN("Nedboer (6RRRtr): Unknown ir <" << ir << ">!");
    	kode=" 6////";
  	}
} /* Nedboer_Kode */



void
Synop::doNedboerKode(std::string &nedboerKode,
		     		 std::string &verTilleggKode,
		     		 std::string &rr24Kode,
		     		 int         &tr,
		     		 SynopDataList &sd)
{
  	SynopData     sisteTid;
  	ostringstream ost;
  	int           ir;
  	float         nedboerTotal=0.0;
  	float         fRR24=FLT_MAX;

  	tr=-1;
  	rr24Kode.erase();
  
  	sisteTid = *sd.begin();

  	ost << "doNedboerKode: sisteTid: " << sisteTid.time() << endl;;

  	if(precipitationParam==PrecipitationRA){
    	ir = nedborFromRA(nedboerTotal, fRR24, tr, sd);
    	ost << "PrecipitationParam: RA    (Automatisk)" << endl
			<< "Nedbï¿½r bereggning Ir:" << ir  << endl 
			<< "   RR_24: " << fRR24          << endl
			<< "  nedbï¿½r: " << nedboerTotal   << endl 
			<< "      tr: " << tr             << endl;
  	}else if(precipitationParam==PrecipitationRR){
    	ir = nedborFromRR(nedboerTotal, fRR24, tr, sd);
    	ost << "EPrecipitationParam: RR    (Automatisk)" << endl
			<< "Nedbï¿½r bereggning Ir:" << ir 
			<< "  nedbï¿½r: " << nedboerTotal << endl;
  	}else if(precipitationParam==PrecipitationRR_N){
    	ost << "EPrecipitationParam: RR_N, hvor N=1,3,6,12,24" << endl;
    	ir = nedborFromRR_N(nedboerTotal, fRR24, tr ,sd);
    	//tr=4;
    	ost << "Nedbï¿½r bereggning Ir:" << ir 
			<< "  nedbï¿½r: " << nedboerTotal << endl;
  	}else if(precipitationParam==PrecipitationRRR){
    	ost << "PrecipitationParam: RRR  (Manuell)" << endl;
    	ir=nedborFromRRRtr(nedboerTotal, fRR24, tr, sd);
  	}else{
    	ost << "PrecipitationParam: UNKNOWN" << endl;
    	ir=4;
  	}

  	LOGDEBUG(ost.str());

  	Nedboer_Kode(nedboerKode, verTilleggKode, rr24Kode, nedboerTotal, fRR24, 
			     sisteTid.time().hour(), tr, ir);
}



/**
 * 2003.03.12 Bxrge Moe
 *
 * nedborFromRA, beregner nedbï¿½ren fra RA (Akumulert nedbï¿½r).
 * Nedbï¿½ren beregnes ved ï¿½ ta differansen mellom  RA fra synoptidspunktet og
 * RA 12 (evt 24) timer tilbake. Dette betyr at nedbï¿½ren bergnes pï¿½ fï¿½lgende
 * mï¿½te for synoptidspunktene 06, 12, 18 og 24. Bruke notasjonen RA(t) for ï¿½
 * angi bï¿½tteinholdet ved timen t. Eks RA(12) er bï¿½tteinnholdet kl 12.
 * 
 * synoptidspunkt kl 00 og 12,  6 timers nedbï¿½r:
 *    nedbï¿½r= RA(t)-RA(t-6)
 *
 * synoptidspunkt kl 6 og 18,  12 timers nedbï¿½r:
 *    nedbï¿½r=RA(t)-RA(t-12)
 *
 * Nedbï¿½ren raporteres bare dersom den er over en gitt grense. For
 * ï¿½yeblikket er den hardjodet til 0.15. Dette kan endres dersom
 * obsdiv finner det nï¿½dvendig.
 *
 * Forutsetninger:
 *   dataList innholder en kontinuerlig rekke med timesverdier med
 *   en differanse pï¿½ 1 time.
 *
 * \param nedbor verdi som settes av funksjonen.
 * \param times  Time vi skal beregne nedbï¿½ren for.
 * \return ir
 */
int
Synop::nedborFromRA(float &nedbor, float &fRR24, int &tr, SynopDataList &sd)
{
  	const float limit=0.2;
  	const float bucketFlush=-10.0;
  	int   nTimes;
  	miutil::miTime t=sd.begin()->time();
  	miutil::miTime t2;
  	SynopData d1;
  	SynopData d2;
  	ISynopDataList it;

  	int   time=t.hour();

  	nedbor=FLT_MAX;
  	fRR24=FLT_MAX;

  	if(time==6 || time==18){
    	tr=2;
    	nTimes=12;
  	}else if(time==0 || time==12){
    	tr=1;
    	nTimes=6;
  	}else{
    	tr=5;
    	nTimes=1;
  	}

  	d1=*sd.begin();

  	if(time==6){
    	//Vi lager en RR_24 verdi til bruk i seksjonen 333 7RR_24
    	t2=t;
    	t2.addHour(-24);
    
    	it=sd.find(t2);

    	if(it!=sd.end() && it->time()==t2){
      		d2=*it;
      
      		if(d1.nedboerTot!=FLT_MAX && d2.nedboerTot!=FLT_MAX){
				fRR24=d1.nedboerTot-d2.nedboerTot;
	
				if(fRR24>bucketFlush){
	  				if(fRR24<=limit)
	    				fRR24=0.0;  //Tï¿½rt
					}else{
	  					//Bï¿½tta er tï¿½mt
	  					fRR24=FLT_MAX;
					}
      		}
    	}
  	}

  	t2=t;
  	t2.addHour(-1*nTimes);  

  	it=sd.find(t2);

  	if(it==sd.end())
    	return 4;

  	d2=*it;

  	if(d2.time()!=t2)
    	return 4;


  	if(d1.nedboerTot==FLT_MAX || d2.nedboerTot==FLT_MAX)
    	return 4;

  	nedbor=d1.nedboerTot-d2.nedboerTot;

  	LOGDEBUG("synopTidspunkt:          " << d1.time() << endl
			 << "nedbor=" <<  nedbor << endl
	   		 << " RR_24=" <<  fRR24 << endl
	   		 << "    RA=" <<  d1.nedboerTot << endl 
  	   		 << "synopTidspunkt-" << nTimes << " timer : " << d2.time() << endl 
	   		 << "    RA=" <<  d2.nedboerTot << endl);

  
  	if(nedbor>bucketFlush){
    	if(nedbor<=limit){
      		nedbor=-1.0;
      		return 3;
    	}
  	}else{ //Bï¿½tta er tï¿½mt
    	nedbor=FLT_MAX;
    	return 4;
  	}

  	return 1;
}

int
Synop::RR_24_from_RR_N(SynopDataList &sd, float &fRR24)
{
  	return 4;
}


/*
 * synoptidspunkt kl 00 og 12,  6 timers nedbï¿½r: RR_6
 * synoptidspunkt kl 6 og 18,  12 timers nedbï¿½r: RR_12
 * 
 * Sï¿½ker i nedbï¿½rparametrene RR_N, hvor N=1, 3, 6, 12, 24
 * Sï¿½ker nedbï¿½ren fra den fï¿½rste som er gitt, sï¿½ker fra 24, 12, .. ,1
 */
int  
Synop::nedborFromRR_N(float &nedbor,
		      		  float &fRR24,
		      		  int &tr,
		      		  SynopDataList &sd)
{
  	int t=sd.begin()->time().hour();

  	nedbor=FLT_MAX;
  	fRR24=FLT_MAX;

  	if(t==6 && sd[0].nedboer24Time!=FLT_MAX)
    	fRR24=sd[0].nedboer24Time;

  	if((t==6 || t==18) && sd[0].nedboer12Time!=FLT_MAX){
    	nedbor=sd[0].nedboer12Time;
    	tr=2;
  	}else if((t==12 || t==0) && sd[0].nedboer6Time!=FLT_MAX){
    	nedbor=sd[0].nedboer6Time;
    	tr=1;
  	}

  	if(nedbor==FLT_MAX){
    	if(sd[0].nedboer24Time!=FLT_MAX){
      		nedbor=sd[0].nedboer24Time;
      		tr=4;
    	}else if(sd[0].nedboer18Time!=FLT_MAX){
      		nedbor=sd[0].nedboer18Time;
      		tr=3;
    	}else if(sd[0].nedboer15Time!=FLT_MAX){
      		nedbor=sd[0].nedboer15Time;
      		tr=9;
    	}else  if(sd[0].nedboer12Time!=FLT_MAX){
      		nedbor=sd[0].nedboer12Time;
      		tr=2;
    	}else if(sd[0].nedboer9Time!=FLT_MAX){
      		nedbor=sd[0].nedboer9Time;
      		tr=8;
    	}else if(sd[0].nedboer6Time!=FLT_MAX){
      		nedbor=sd[0].nedboer6Time;
      		tr=1;
    	}else if(sd[0].nedboer3Time!=FLT_MAX){
      		nedbor=sd[0].nedboer3Time;
      		tr=7; 
    	}else if(sd[0].nedboer2Time!=FLT_MAX){
      		nedbor=sd[0].nedboer2Time;
      		tr=6;
    	}else if(sd[0].nedboer1Time!=FLT_MAX){
      		nedbor=sd[0].nedboer1Time;
      		tr=5; 
    	}
  	}

  	if(nedbor==FLT_MAX)
    	return 4;

  	if(t==6 && fRR24==FLT_MAX && sd.size()>1){
    	CISynopDataList it=sd.begin();
    	miutil::miTime tt=it->time();
    	
    	tt.addHour(-12);	
    
    	CISynopDataList it2=sd.find(tt);	
    	
    	if(it2!=sd.end() && it2->time()==tt){
    		bool hasPrecip=true;
    		fRR24=0.0;

         //If there is measured no precip in the 12 hour time
         //periode the nedboer12Time=-1.0.
         //We cant just test for for nedboer12Time==-1.0. 

			
			if(it->nedboer12Time!=FLT_MAX){
            if(it->nedboer12Time>=0.0)
	  			   fRR24+=it->nedboer12Time;
            else if(it->nedboer12Time<=-1.5f)
               hasPrecip=false;
         }else if(it->IIR!="3")
	  			hasPrecip=false;
	  			
	  		if(it2->nedboer12Time!=FLT_MAX){
            if(it2->nedboer12Time>=0.0)
   	  			fRR24+=it2->nedboer12Time;
            else if(it2->nedboer12Time<=-1.5f)
               hasPrecip=false;
         }else if(it2->IIR!="3")
	  			hasPrecip=false;

			if(!hasPrecip)
				fRR24=FLT_MAX;
    	}
  	}
  
  	if(nedbor<0.1){
    	nedbor=0.0;
    	return 3;
  	}

   	return 1;
}  


/**
 * Nedbï¿½r fra manuelle nedbï¿½rstasjoner. Bruker ITR for ï¿½ sjekke
 * om det er angitt manuell nedbï¿½r. Hvis ITR er satt nedbï¿½ren gitt.
 * Bruker ITR for ï¿½ finne rett nedbï¿½rparameter.
 *
 *  ITR    Nedbï¿½r parameter
 *  -----------------------
 *    1    nedboer6Time  (RR_6)
 *    2    nedboer12Time (RR_12)
 *    3    nedboer18Time (RR_18) 
 *    4    nedboer24Time (RR_24)
 *    5    nedboer1Time  (RR_1)
 *    6    nedboer2Time  (RR_2)  
 *    7    nedboer3Time  (RR_3)
 *    8    nedboer9Time  (RR_9)  
 *    9    nedboer15Time (RR_15) 
 *
 * Hvis ITR har en gyldig verdi og nedbï¿½ren er -1 angir dette tï¿½rt.
 */
int  
Synop::nedborFromRRRtr(float &nedbor, 
		       float &fRR24, 
		       int   &tr, 
		       SynopDataList &sd)
{
  	nedbor=FLT_MAX;
  	fRR24 =FLT_MAX;

  	if(sd.begin()->time().hour()==6) {
    	CISynopDataList it=sd.begin();
    	miutil::miTime tt=it->time();
	
    	tt.addHour(-12);	
    
    	CISynopDataList it2=sd.find(tt);	
    	
    	if(it2!=sd.end() && it2->time()==tt){
    		bool hasPrecip=true;
    		fRR24=0.0;
			
         //If there is measured no precip in the 12 hour time
         //periode the nedboer12Time=-1.0.
         //We cant just test for for nedboer12Time==-1.0. 
         
         //cerr << "nedboer12Time:  " << it->nedboer12Time << endl
         //    <<"nedboer12Time2: " << it2->nedboer12Time << endl;
                    
			if(it->nedboer12Time!=FLT_MAX){
            if(it->nedboer12Time>=0.0)
	  			   fRR24+=it->nedboer12Time;
            else if(it->nedboer12Time<=-1.5f)
               hasPrecip=false;
         }else if(it->IIR!="3")
	  			hasPrecip=false;
	  			
	  		if(it2->nedboer12Time!=FLT_MAX){
            if(it2->nedboer12Time>=0.0)
	  			   fRR24+=it2->nedboer12Time;
            else if(it2->nedboer12Time<=-1.5f)
               hasPrecip=false;
         }else if(it2->IIR!="3")
	  			hasPrecip=false;

			if(!hasPrecip)
				fRR24=FLT_MAX;
    	}
    	
    	if(fRR24==FLT_MAX){
    		//Do we have an RR_24 precip.
    		fRR24=sd.begin()->nedboer24Time;
    		
    		if(fRR24!=FLT_MAX && fRR24<0.0)
    			fRR24=FLT_MAX;
    	}
  	}

   //cerr << "sd[0].ITR: [" << (sd[0].ITR[0]-'0') << "]" << endl;

  	if(sd[0].ITR.empty()){
    	if(sd[0].IIR.empty() || sd[0].IIR[0]!='3') 
      		return 4;

    	nedbor=0.0;
    	return 3;
  	}

  	switch(sd[0].ITR[0]-'0'){
  	case 1: 
    	if(sd[0].nedboer6Time!=FLT_MAX){
      		nedbor=sd[0].nedboer6Time;
      		tr=1;
     	}
    	break;
  	case 2:
    	if(sd[0].nedboer12Time!=FLT_MAX){
      		nedbor=sd[0].nedboer12Time;
      		tr=2;
     	}
    	break;
  	case 3:
    	if(sd[0].nedboer18Time!=FLT_MAX){
      		nedbor=sd[0].nedboer18Time;
      		tr=3;
   		}
    	break;
  	case 4:
    	if(sd[0].nedboer24Time!=FLT_MAX){
      		nedbor=sd[0].nedboer24Time;
      		tr=4;
     	}
    	break;
  	case 5:
    	if(sd[0].nedboer1Time!=FLT_MAX){
      		nedbor=sd[0].nedboer1Time;
      		tr=5;
     	}
    	break;
  	case 6:
    	if(sd[0].nedboer2Time!=FLT_MAX){
      		nedbor=sd[0].nedboer2Time;
      		tr=6;
    	}
    	break;
  	case 7:
    	if(sd[0].nedboer3Time!=FLT_MAX){
      		nedbor=sd[0].nedboer3Time;
      		tr=7;
     	}
    	break;
  	case 8:
    	if(sd[0].nedboer9Time!=FLT_MAX){
      		nedbor=sd[0].nedboer9Time;
      		tr=8;
    	}
    	break;
  	case 9:
    	if(sd[0].nedboer15Time!=FLT_MAX){
      		nedbor=sd[0].nedboer15Time;
      		tr=9;
    	}
    	break;
  	default:
    	return 4;
  	}

  	if(nedbor==FLT_MAX) 
    	return 4;

  	if(static_cast<int>(round(nedbor))==-1) //tï¿½rt
    	return 3;
 
  	return 1;
}  



/**
 * 2003.03.14 Bxrge Moe
 *
 * nedborFromRR, beregner nedbï¿½ren fra RR (Times nedbï¿½r).
 * Nedbï¿½ren beregnes ved ï¿½ summere RR fra synoptidspunktet og
 * 12 (evt 6) timer tilbake. 

 * synoptidspunkt kl 00 og 12,  6 timers nedbï¿½r:
 *    nedbï¿½r= RR(t)+RR(t-1)+ .... +RR(t-6)
 *
 * synoptidspunkt kl 6 og 18,  12 timers nedbï¿½r:
 *    nedbï¿½r=RR(t)+RR(t-1)+ .... +RR(t-12)
 *
 * Nedbï¿½ren raporteres bare dersom den er over en gitt grense. For
 * ï¿½yeblikket er den hardkodet til 0.15. Dette kan endres dersom
 * obsdiv finner det nï¿½dvendig.
 *
 * Forutsetninger:
 *   dataList innholder en kontinuerlig rekke med timesverdier med
 *   en differanse pï¿½ 1 time.
 *
 * \param nedbor verdi som settes av funksjonen.
 * \param times  Time vi skal beregne nedbï¿½ren for.
 * \return ir
 */

int  
Synop::nedborFromRR(float &nedbor, float &fRR24, int &tr, SynopDataList &sd)
{
  	const float limit=0.2;
  	int   nTimeStr=sd.nContinuesTimes();
  	int   time=sd.begin()->time().hour();
  	int   nTimes;
  	float sum=0;

  	nedbor=FLT_MAX;
  	fRR24=FLT_MAX;

  	if(time==6 || time==18){
    	tr=2;
    	nTimes=12;
  	}else if(time==0 || time==12){
    	tr=1;
    	nTimes=6;
  	}else{
    	tr=1;
    	nTimes=1;
  	}
  	
  	if(nTimeStr<nTimes)
    	return 4; 

  	for(int i=0; i<nTimes; i++){
    	if(sd[i].nedboer1Time==FLT_MAX)
      		return 4;
    
    	if(sd[i].nedboer1Time>=0)
      		sum+=sd[i].nedboer1Time;
  	}

  	nedbor=sum;

  	if(time==6 && nTimeStr>=24){
    	int i;
    	sum=0.0;

    	for(i=0; i<24; i++){
      		if(sd[i].nedboer1Time==FLT_MAX)
				break;
      		else if(sd[i].nedboer1Time>=0.0)
				sum+=sd[i].nedboer1Time;
    	}
    
    	if(i==24)
      		fRR24=sum;
  	}
  
  	LOGDEBUG("synopTidspunkt:          " << sd[0].time() 
			 << "  RR=" <<  sd[0].nedboer1Time << endl
	   		 << "synopTidspunkt-" << nTimes << " timer : " 
	   		 << sd[nTimes-1].time() 
	   		 << "  RR=" <<  sd[nTimes-1].nedboer1Time << endl);

  
  	if(nedbor<=limit){
    	nedbor=-1.0;
    	return 3;
  	}

  	return 1;
}


/*
** Funksjonen splittar strengen ved karakter nr. index (ved nï¿½raste space),
** og legg inn linefeed og 6 space'ar ved denne.
*/
void 
Synop::SplittStreng(std::string &streng, std::string::size_type index)
{
  	std::string tmp;
  	int         i;
  	std::string::size_type t;
  	std::string::size_type p;
  	std::string::size_type n;
  	char        *seksjon[]={" 222// ", 
					        " 333 ", 
			  				" 444 ", 
			  				" 555 ",
			  				0};
  
  	i = 0;
  
  	while(seksjon[i]){
    	n=strlen(seksjon[i]);
    	t = streng.find(seksjon[i]);
    
    
    	if(t!=std::string::npos){      
      		tmp=streng.substr(t);
      		p=tmp.find_first_not_of(' ', n);
      
      		if(p!=std::string::npos){
				if(p<7)
	  				tmp.insert(n, 6-n, ' ');
				
				if(p>6)
	  				tmp.erase(7, p-7);
      		}
      
      		streng.erase(t);
      		streng+="\r\r\n     ";
      		streng+=tmp;
    	}
    
   		i++;
  	}
} /* SplittStreng */



Synop::Synop(EPrecipitation pre)
  :debug(false), precipitationParam(pre)
    
{
}

Synop::Synop():debug(false), precipitationParam(PrecipitationRA)
{
}

Synop::~Synop()
{
}

void 
Synop::replaceCCCXXX(std::string &synop, int ccx)
{
  	char tmp[10];
  	std::string::size_type i=synop.find(" CCCXXX");

  	if(i==std::string::npos)
    	return;

  	tmp[0]='\0';

  	if(ccx>0)
    	sprintf(tmp, " CC%c", 'A'+(ccx-1));
  
  	synop.replace(i, 7, tmp);
}
















