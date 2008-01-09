/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: synop.h,v 1.8.2.3 2007/09/27 09:02:23 paule Exp $                                                       

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
#ifndef __SYNOP_H__
#define __SYNOP_H__

#include <vector>
#include <string>
#include "SynopData.h"
#include "StationInfo.h"

#define GROUPSIZE                  6

/*
 * Quick and dirty fix to let the windspeed be in m/s, ie dont
 * convert to knop
 */

#define KNOPFAKTOR     1
//#define KNOPFAKTOR     1.94384449244

class Synop
{
    Synop(const Synop &);
    Synop& operator=(const Synop &);

 public:
    enum EPrecipitation{NoPrecipitation,
                        PrecipitationRA, //akumulert nedb�r   (Automatisk)
			PrecipitationRR, //Fra 1 times nedb�r (Automatisk)
			PrecipitationRRR, //Fra manuell nedb�r
			//PrecipitationRR_24 //Fra RR_24
			PrecipitationRR_N //Fra RR_1, RR_3, RR_6, RR_12 eller RR_24
    };

 protected:
    bool           debug;
    std::string    errorMsg;
    EPrecipitation precipitationParam;

    int  Vis_ir_Kode(const std::string &str);
    bool Sjekk_Gruppe(int grpNr, std::string &kode, const std::string &str);
    void Tid_Kode(std::string &kode, int time);
    void Naa_Vind_Kode(std::string &kode, float retn, float hast);
    void Temp_Kode(std::string &kode, float temp);
    

    /**
     * Nedboer_Kode, create the precipitation code
     * for 6RRRtr and 555 .... 4RtWdWdWd. It assumes that we shall create
     * a precipitation for data set.
     *
     * @param kode the 6RRRtr code on return.
     * @param vertilleggKode the 4RtWdWdWd on return.
     * @param totalNedboer the total precipitation for the period given by tr.
     * @param time the observation time to report the precipitation for.
     * @param tr the period the precipitation totalNedboer is acumulated for.
     * @param ir a value that give information about the percipitation in
     *        the tr period: 1 it has been precipitation, 3 No precipitation, 4
     *        no mearsument in the period.
     *
     * @see doNedboerKode
     */
    void Nedboer_Kode(std::string &kode,           //RRRtr
		      std::string &vertilleggKode, //555 ... 4RtWdWdWd
		      std::string &sRR24Kode,          //333 ... 7RR24
		      float totalNedboer,
		      float fRR24,
		      int time,
		      int &tr,
		      int ir);
    void doNedboerKode(std::string &nedboerKode,
		       std::string &verTilleggKode,
		       std::string &rr24,
		       int         &tr,
		       SynopDataList &sd);
    int  nedborFromRA(float &nedbor, float &fRR24, int &tr, SynopDataList &sd);
    int  nedborFromRR(float &nedbor, float &fRR24, int &tr, SynopDataList &sd);
    int  nedborFromRR_N(float &nedbor, 
			float &fRR24, 
			int   &tr, 
			SynopDataList &sd);
    int  nedborFromRRRtr(float &nedbor,
			 float &fRR24, 
			 int   &tr, 
			 SynopDataList &sd);
    int RR_24_from_RR_N(SynopDataList &sd, float &fRR24);
 	 
    void Dugg_Kode(std::string &kode, float temp, float fukt);
    void Min_Max_Kode(std::string &kode, SynopDataList &sd);
    void Max_Min_Kode(std::string &kode, SynopDataList &sd);
    void Max_Vind_Gust_Kode(std::string &kode, SynopDataList &sd);
    void Max_Vind_Max_Kode(std::string &kode, SynopDataList &sd);
    void Trykk_Kode(int prefix, std::string &kode, float trykk);
    void Tendens_Kode(std::string &kode, 
		      int   time, 
		      float trykk1, 
		      float trykk2, 
		      float trykk3, 
		      float trykk4);
    void Tendens_Kode(std::string &kode, const SynopData &data);
    void Skydekke_Kode(std::string &kode, const std::string &str);
    void Hoyde_Sikt_Kode(std::string &kode, const SynopData &data);
    int  ix_Kode(const std::string &str);
    bool doVerGenerelt(std::string &kode, int &ix, const SynopData &data);
    bool SjoeTempKode(std::string &kode, const SynopData &data);
    bool SjekkEsss(std::string &kode, const std::string &str);
    void doEsss( std::string &kode, const SynopData &data );
    void GressTempKode(std::string &kode, SynopDataList &sd);
    void SplittStreng(std::string &streng, std::string::size_type index);

 public:
    
    
    Synop(EPrecipitation precipitation);
    Synop();

    ~Synop();

    std::string getErrorMsg()const { return errorMsg;}
    void        setDebug(){ debug=true;}
    
    /**
     * doSynop,
     *
     * \param create_CCA_template, angir at vi skal lage en template
     *        p� formen 'CCCXXX'  som senere skal skiftes
     *        ut med aktuell CCA verdi.
     */
    bool    doSynop(int                  synopno,
		    const std::string    &usteder,
		    int                  listenummer,
		    std::string          &synop,
		    StationInfoPtr       info,
		    SynopDataList        &synopData,
		    bool                 create_CCA_template=false);


    /**
     * replaceCCCXXX erstatter CCCXXX templaten, hvis den finnes,
     * med verdien angitt med ccx. Hvis ccx er 0 skal vi bare fjerne
     * templaten, for ccx=1 f�r vi CCA, for ccx=2 f�r vi CCB osv. Hvis
     * ccx > 26, blir templaten bare fjernet.
     */
    static void replaceCCCXXX(std::string &synop, int ccx);
};

#endif
