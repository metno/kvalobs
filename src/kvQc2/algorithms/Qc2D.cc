/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id$                                                       

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
#include <list>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <algorithm>
#include "Qc2D.h"
#include "Distribute.h"
#include "StationSelection.h"
#include "BasicStatistics.h"
#include <dnmithread/mtcout.h>
#include <kvalobs/kvData.h>
//#include <kvalobs/kvKeyVal.h>
#include "kvQABaseTypes.h"
#include <map>

#include <netcdfcpp.h>

#include <milog/milog.h>

#include "ProcessControl.h"
#include "proj++.h"
#include "scone.h"

#include "table_delaunay.h"

using namespace std;
using namespace miutil;
using namespace dnmi;



///Method to clear() all of the vectors held in the Qc2D data structure.
void Qc2D::clean()
{
  stid_.clear();
  obstime_.clear();
  original_.clear();
  paramid_.clear();
  tbtime_.clear();
  typeid_.clear();
  sensor_.clear();
  level_.clear();
  corrected_.clear();
  controlinfo_.clear();
  useinfo_.clear();
  cfailed_.clear();

  intp_.clear();
  redis_.clear();
  lat_.clear();
  lon_.clear();
  ht_.clear();
  CP_.clear();

  stindex.clear();
}

Qc2D::
Qc2D(std::list<kvalobs::kvData>& QD, std::list<kvalobs::kvStation>& SL, ReadProgramOptions PPP)
{
  params=PPP;
  std::map<int, kvalobs::kvStation> Gsid;
  for ( std::list<kvalobs::kvStation>::const_iterator it = SL.begin(); it != SL.end(); ++it ) {
       Gsid[ it->stationID() ] = *it;
  }
  for (std::list<kvalobs::kvData>::const_iterator id = QD.begin(); id != QD.end(); ++id) {
          istid( id->stationID() );
          iobstime( id->obstime() );
          ioriginal( id->original() );
          iparamid( id->paramID() );
          itbtime( id->tbtime() );
          itypeid( id->typeID() );
          isensor( id->sensor() );
          ilevel( id->level() );
          icorrected( id->corrected() );
          icontrolinfo( id->controlinfo() );
          iuseinfo( id->useinfo() );
          icfailed( id->cfailed() );
          iintp( -10.0 );
          iredis( -10.0 );
          icp( -10.0 );
          iht(Gsid[ id->stationID() ].height());
          ilat(Gsid[ id->stationID() ].lat());
          ilon(Gsid[ id->stationID() ].lon());
          istindex( id->stationID() );
  }
}

Qc2D::
Qc2D(std::list<kvalobs::kvData>& QD, std::list<kvalobs::kvStation>& SL, ReadProgramOptions PPP, std::string GenerateMissing)
{
  params=PPP;
  std::map<int, kvalobs::kvStation> Gsid;
  kvalobs::kvControlInfo::kvControlInfo controlNULL;
  kvalobs::kvUseInfo::kvUseInfo useNULL;

  for ( std::list<kvalobs::kvStation>::const_iterator it = SL.begin(); it != SL.end(); ++it ) {
       Gsid[ it->stationID() ] = *it;
  }

  for (std::list<kvalobs::kvData>::const_iterator id = QD.begin(); id != QD.end(); ++id) {
          istid( id->stationID() );
          iobstime( id->obstime() );
          ioriginal( id->original() );
          iparamid( id->paramID() );
          itbtime( id->tbtime() );
          itypeid( id->typeID() );
          isensor( id->sensor() );
          ilevel( id->level() );
          icorrected( id->corrected() );
          icontrolinfo( id->controlinfo() );
          iuseinfo( id->useinfo() );
          icfailed( id->cfailed() );
          iintp( -10.0 );
          iredis( -10.0 );
          icp( -10.0 );

          iht(Gsid[ id->stationID() ].height());
          ilat(Gsid[ id->stationID() ].lat());
          ilon(Gsid[ id->stationID() ].lon());

          istindex( id->stationID() );
  }

/// Includes handling of missing rows.
//  This block will add entries for stations with no values. If an accumulated value is found for these
//  stations a reaccumulation is performed.
for ( std::list<kvalobs::kvStation>::const_iterator ist = SL.begin(); ist != SL.end(); ++ ist )
  {
      std::vector<int>::const_iterator vit = find (stid_.begin(), stid_.end(),  ist->stationID() );
      if ( vit  != stid_.end() )
      {
        //std::cout << "Celebrity Match" << std::endl;
      }
      else
      {
         //std::cout << "Celebrity Miss" << std::endl;
         std::list<kvalobs::kvData>::const_iterator zid=QD.begin();/// Need to check this logic!!!
         iobstime( zid->obstime() );
         iparamid( zid->paramID() );
         itbtime( miutil::miTime::nowTime() );
         isensor(0);
         //isensor( zid->sensor() );
         ilevel(0);                        // Check this ... is there a proxy one can use?
         //ilevel( zid->level() );
         icfailed( "Missing Row!" );
         istid(ist->stationID());
         ioriginal(params.missing);
         itypeid(999);
         icorrected(params.missing);
         iredis(-10.0);
         iintp(-10.0);
         icp( -10.0 );
         icontrolinfo(controlNULL);
         iuseinfo(useNULL);
         iht(Gsid[ ist->stationID() ].height());
         ilat(Gsid[ ist->stationID() ].lat());
         ilon(Gsid[ ist->stationID() ].lon());
         istindex( ist->stationID() );
      }
  }
}
                          
std::ostream& operator<<(std::ostream& stm, const Qc2D &Q)
{
    stm << "Qc2 Data:";
    for (unsigned int i=0; i<Q.stid_.size(); i++){
       stm <<"{"<< Q.stid_[i]
           <<","<< Q.obstime_[i]
           <<","<< Q.original_[i]
           <<","<< Q.paramid_[i]
           <<","<< Q.tbtime_[i]
           <<","<< Q.typeid_[i]
           <<","<< Q.sensor_[i]
           <<","<< Q.level_[i]
           <<","<< Q.corrected_[i]
           <<","<< Q.controlinfo_[i]
           <<","<< Q.useinfo_[i]
           <<","<< Q.cfailed_[i]
           <<","<< Q.intp_[i]
           <<","<< Q.redis_[i]
           <<","<< Q.lat_[i]
           <<","<< Q.lon_[i]
           <<","<< Q.ht_[i]
           <<","<< Q.CP_[i]
           <<"}"<< std::endl;
    }
    return stm;
}



/// Method to pass Qc2D data for redistribution of accumulated values. ((Needs to be reworked!! Encapsulate!))
void 
Qc2D::
distributor(const std::list<kvalobs::kvStation> & slist, std::list<kvalobs::kvData>& ReturnData,int ClearFlag)
{
 	static Distribute DataForRedistribution(slist,params);	
        if (ClearFlag) DataForRedistribution.clear_all();  //For cleaning up memory when all is done!
 	    
 	for (unsigned int i=0 ; i<original_.size() ; i++) { 	

             //if ( ControlFlag.condition(controlinfo_[i],params.Aflag)) { 
             if ( ControlFlag.condition(controlinfo_[i],params.Aflag) && typeid_[i]==params.tid) { 
             //if ( controlinfo_[i].flag( 12 ) == 2 ) {   // Only redistribute data for this case!

                  DataForRedistribution.add_element(stid_[i],original_[i],intp_[i],corrected_[i],redis_[i],
                                tbtime_[i],obstime_[i], sensor_[i], level_[i], typeid_[i], 
                                controlinfo_[i], useinfo_[i], cfailed_[i]);

                  if (original_[i] != params.missing) {        // This condition means the
                                                       // value is no longer missing
                                                       // This is data to Redistribute
                       DataForRedistribution.RedistributeStationData(stid_[i],ReturnData);
                       DataForRedistribution.clean_station_entry(stid_[i]);   

                  }
  	     }
        }        
}

/// Pointless interface to the interpolation method. Replace with interpolation algorithm strategy!!! 
void 
Qc2D::
Qc2_interp()
{


      StationSelection Adjacent;     // These options go with _sl
      std::map<int, std::list<int> > Neighbours=Adjacent.ReturnMap();

      int InterpCode = params.InterpCode;

      for (unsigned int i=0 ; i<original_.size() ; i++) {
  	 switch (InterpCode) {
            case 1:
  	        calculate_intp_all(i);
                break;
            case 2:
                idw_intp_limit(i);
                break;
            case 3:
  	        calculate_intp_h(i);  // This option would add a 10% correction to the height for every 100m.
                break;
            case 4:
  	        calculate_intp_sl(i,Neighbours[ stid_[i] ]);  // Perform the interpolation over a station list 
                break;
            case 5:
  	        calculate_trintp_sl(i,Neighbours[ stid_[i] ]);  // Perform the interpolation over a station list 
                                                                // and performs linear gradient interpolation
                break;
            case 6:
                calculate_intp_wet_dry(i);
                break;
            case 7:
                intp_delaunay(i);
                break;
            default:
                std::cout << "No valid Interpolation Code Provided. Case: " << InterpCode << std::endl;
                break;
         }     
      }
}
 
/// Inverse distance weighting interpolation prototype.
/// Algorithm to construct a model value by inverse distance weighting from neighbouring stations.
/// This optionincludes experimental investigation of various uniformity tests.
void 
Qc2D::
calculate_intp_wet_dry(unsigned int index)
{
 	  const double RADIUS=6371.0;
 	  float temp_distance;
 	  float data_point;
 	  float weight;
 	  float inv_dist;
 	  float delta_lat;
 	  float delta_lon;
          double a, c;
 	  const double radish = 0.01745329251994329509;
 	  
 	  typedef pair <float,int> id_pair;
 	  
 	  // pindex will hold the "station_distance" and the index in the
 	  // original data array.
 	  
 	  std::vector<id_pair> pindex;
 	  std::vector<id_pair>::const_iterator ip;

 	  for (unsigned int i=0 ; i<original_.size() ; i++) {
 	  	
            delta_lon=(lon_[index]-lon_[i])*radish;
            delta_lat=(lat_[index]-lat_[i])*radish;
            a        = sin(delta_lat/2)*sin(delta_lat/2) +
                       cos(lat_[i]*radish)*cos(lat_[index]*radish)*
                       sin(delta_lon/2)*sin(delta_lon/2);
            c        =2.0 * atan2(sqrt(a),sqrt(1-a));

            temp_distance = RADIUS*c;                
            
            pindex.push_back( id_pair(temp_distance,i) );
      }	
       
      //sort the data, i.e. by the distance to each neighbour
      //the value in the second part of the pair is the index in the original array 
       
 	  sort(pindex.begin(),pindex.end());
 	  	  
 	  //Find index for stations < max_distance km distant
 	  
 	  int imax=0;
          //float max_distance=100.0;
          float max_distance=50.0;
 	  
 	  for (unsigned int i=0 ; i<original_.size() ; i++) {
 	  	  if (pindex[i].first < max_distance) imax=i;
 	  	  }
 	   	  


 	  inv_dist = 0.0;
 	  weight   = 0.0;
          //int idog=0;
          //bool DoInt=true;
          bool DoInt=false;

          //LOGINFO("ZZY Station of Interest:" << stid_[index] << " " << original_[index] << " " <<lat_[index] 
                                         //<< " " << lon_[index] << " " <<ht_[index]);
 	  
                        //LOGINFO("ZZY ...................:" << stid_[pindex[i].second] << " " 
                          //<< original_[pindex[i].second] << " " <<lat_[pindex[i].second] 
                          //<< " " << lon_[pindex[i].second] << " " <<ht_[pindex[i].second]);
          
          float sumPP=0.0;     //   MP | PP
          float sumPM=0.0;     //   _ _|_ _
          float sumMM=0.0;     //      |
          float sumMP=0.0;     //   MM | PM
          int nPP=0;
          int nPM=0;
          int nMM=0;
          int nMP=0;
          float mPP=-999.0;
          float mPM=-999.0;
          float mMM=-999.0;
          float mMP=-999.0;  	

          int NumWetQ=0;
          int NumDryQ=0;

          int nPPwet=0;
          int nPMwet=0;
          int nMMwet=0;
          int nMPwet=0;
          int nPPdry=0;
          int nPMdry=0;
          int nMMdry=0;
          int nMPdry=0;

          float ftemp;

          std::vector<float> Difs;
          Difs.clear();

          //std::cout << stid_[index] << " ?=? " << stid_[pindex[0].second] << std::endl;
          //std::cout << original_[index] << " ?=? " << original_[pindex[0].second] << std::endl;
          //std::cout << "Distance = " << pindex[0].first << std::endl; // Just checking algorithm is working.

 	  for (int i=1 ; i<imax+1 ; i++) { //implement a uniformity check
            data_point=original_[pindex[i].second];
            if (data_point == -1) data_point = 0; 
            if (imax > 1 && data_point > -1 && pindex[i].first > 0 && 
 	  	                              controlinfo_[pindex[i].second].flag( 12 ) == 1  ) {

                   if (lat_[pindex[i].second] >  lat_[index] &&
                       lon_[pindex[i].second] >  lon_[index]) {
                                                               sumPP += data_point;                      
                                                                 nPP += 1; 
                                                                 mPP =sumPP/nPP;
                                                                 if (data_point>0.0) {++nPPwet;}
                                                                 else                {++nPPdry;}
                   }
                   if (lat_[pindex[i].second] <= lat_[index] &&
                       lon_[pindex[i].second] >  lon_[index]) {
                                                               sumPM += data_point;                      
                                                                 nPM += 1;
                                                                 mPM =sumPM/nPM;
                                                                 if (data_point>0.0) {++nPMwet;}
                                                                 else                {++nPMdry;}
                   }
                   if (lat_[pindex[i].second] <= lat_[index] &&
                       lon_[pindex[i].second] <= lon_[index]) {
                                                               sumMM += data_point;                      
                                                                 nMM += 1;
                                                                 mMM =sumMM/nMM;
                                                                 if (data_point>0.0) {++nMMwet;}
                                                                 else                {++nMMdry;}
                   }
                   if (lat_[pindex[i].second] >  lat_[index] &&
                       lon_[pindex[i].second] <= lon_[index]) {
                                                               sumMP += data_point;                      
                                                                 nMP += 1;
                                                                 mMP =sumMP/nMP;
                                                                 if (data_point>0.0) {++nMPwet;}
                                                                 else                {++nMPdry;}
                   }
            }
          }

          // only interpolate if all surrounding data is all wet
          if (nPP==nPPwet && nPM==nPMwet && nMM==nMMwet && nMP==nMPwet) DoInt=true; 
          // ...or dry
          if (nPP==nPPdry && nPM==nPMdry && nMM==nMMdry && nMP==nMPdry) DoInt=true; 

          if (mPP != -999.0 && mPM != -999.0) Difs.push_back(fabs(mPP-mPM));
          if (mPP != -999.0 && mMM != -999.0) Difs.push_back(fabs(mPP-mMM));
          if (mPP != -999.0 && mMP != -999.0) Difs.push_back(fabs(mPP-mMP));
          if (mPM != -999.0 && mMM != -999.0) Difs.push_back(fabs(mPM-mMM));
          if (mPM != -999.0 && mMP != -999.0) Difs.push_back(fabs(mPM-mMP));
          if (mMM != -999.0 && mMP != -999.0) Difs.push_back(fabs(mMM-mMP));

                  //std::cout << "Vector analysis: ";
                  //for (std::vector<float>::const_iterator jj=Difs.begin(); jj<Difs.end();++jj){
                       //std::cout<<*jj<<" ";
                  //}
                  //std::cout<<std::endl;

          NumWetQ=((sumPP>0.0) ? 1 : 0)  + ((sumPM>0.0) ? 1 : 0)+ ((sumMM>0.0) ? 1 : 0)+ ((sumMP>0.0) ? 1 : 0);
          NumDryQ=((sumPP==0.0 && nPP>0) ? 1 : 0) + ((sumPM==0.0 && nPM>0) ? 1 : 0) + ((sumMM==0.0 && nMM>0) ? 1 : 0) +
                  ((sumMP==0.0 && nMP>0) ? 1 : 0); 

          //std::cout << "Wet Regions : " << NumWetQ << std::endl;
          //std::cout << "Dry Regions : " << NumDryQ << std::endl;
          //std::cout << "Stats " << NumWetQ << " " << NumDryQ << std::endl;

          int Biggies=0;
          for (std::vector<float>::const_iterator jj=Difs.begin(); jj<Difs.end();++jj){
              ftemp = *jj; 
              if (ftemp > 20.0) Biggies += 1;
          }


          //if all neighbours are wet interpolate
          //if (sumPP > 0.0 && sumPM > 0.0 && sumMM > 0.0 && sumMP > 0.0) {
                  //std::cout << "Stats All Wet " << sumPP << " " << sumPM << " " << sumMM << " " << sumMP <<std::endl;
                  //DoInt=true;
          //}

          //if all neighbours are dry interpolate
          //if ((sumPP+sumPM+sumMM+sumMP)==0.0 && nPP>0 && nPM>0 && nMM>0 && nMP>0) {
                  //std::cout << "Stats All Dry " << sumPP<< " " <<sumPM<< " " <<sumMM<< " " <<sumMP <<std::endl;
                  //DoInt=true;
          //}


          //if one neighbour (quadrant) is very different to the others, do not interpolate 

          //if (Biggies == 3) {
           //       DoInt=false;
          //}

          //if neighbouring regions include wet and dry regions (mixed) ... do not interpolate

          //if (NumWetQ > 0 && NumDryQ > 0) DoInt=false;

          //LOGINFO("ZZYX: "<<"("<<sumPP<<","<<nPP<<","<<mPP<<") "
                          //<<"("<<sumPM<<","<<nPM<<","<<mPM<<") "
                          //<<"("<<sumMM<<","<<nMM<<","<<mMM<<") "
                          //<<"("<<sumMP<<","<<nMP<<","<<mMP<<") ");
          //std::cout << "XZYX: "<<mPP<<"  "
                          //<<mPM<<"  "
                          //<<mMM<<"  "
                          //<<mMP<< std::endl;

          //std::cout << "Stations : " <<stid_[index];

 	  for (int i=1 ; i<imax+1 ; i++) {  //NB i=0 corresponds to the station for which we do an interpolation
 	  	  
 	  	  data_point=original_[pindex[i].second];
 	  	  
 	  	  if (data_point == -1) data_point = 0; // These are bone dry measurments
 	  	                                        // as opposed to days when there
 	  	                                        // may have been rain but none
 	  	                                        // was measurable
 	  	  
 	  	  //if (imax > 1 && data_point > -1 && pindex[i].first > 0 && data_point < 40 &&
 	  	  if (DoInt && imax > 1 && data_point > -1 && pindex[i].first > 0 && 
 	  	                              controlinfo_[pindex[i].second].flag( 12 ) == 1  ) {


                       //std::cout << " " << stid_[pindex[i].second];
 	  	  	
                        //inv_dist += 1.0/(pindex[i].first); 
                        //weight += data_point/(pindex[i].first);

                        inv_dist += 1.0/(pindex[i].first*pindex[i].first); 
                        weight += data_point/(pindex[i].first*pindex[i].first);

                        //inv_dist += 1.0/(pindex[i].first*pindex[i].first*pindex[i].first); 
                        //weight += data_point/(pindex[i].first*pindex[i].first*pindex[i].first);

                        //inv_dist += 1.0/(pindex[i].first*pindex[i].first*pindex[i].first*pindex[i].first); 
                        //weight += data_point/(pindex[i].first*pindex[i].first*pindex[i].first*pindex[i].first);

                        //LOGINFO("ZZY ...................:" << stid_[pindex[i].second] << " " 
                          //<< original_[pindex[i].second] << " " <<lat_[pindex[i].second] 
                          //<< " " << lon_[pindex[i].second] << " " <<ht_[pindex[i].second]);

                        //idog += 1;
 	  	  }
 	  }

          //std::cout << std::endl;
 	     

 	  if (inv_dist > 0.0) {
 	     intp_[index] = weight/inv_dist; 
 	  }  
 
          //std::cout << "Number of points used in interpolation = " << idog << std::endl;
}

///Inverse distance weighting interpolation prototype.
/// Algorithm to construct a model value by inverse distance weighting from neighbouring stations.
/// This optionincludes experimental investigation of various uniformity tests.
void 
Qc2D::
idw_intp_limit(unsigned int index)
{
 	  const double RADIUS=6371.0;
 	  float temp_distance;
 	  float data_point;
 	  float weight;
 	  float inv_dist;
 	  float delta_lat;
 	  float delta_lon;
          double a, c;
 	  const double radish = 0.01745329251994329509;

          ProcessControl CheckFlags;

          std::vector<float> NeighboursUsed;
          NeighboursUsed.clear();
          double sum, mean, var, dev, skew, kurt;

 	  
 	  typedef pair <float,int> id_pair;
 	  
 	  // pindex will hold the "station_distance" and the index in the
 	  // original data array.
 	  
 	  std::vector<id_pair> pindex;
 	  std::vector<id_pair>::const_iterator ip;

 	  for (unsigned int i=0 ; i<original_.size() ; i++) {
 	  	
            delta_lon=(lon_[index]-lon_[i])*radish;
            delta_lat=(lat_[index]-lat_[i])*radish;
            a        = sin(delta_lat/2)*sin(delta_lat/2) +
                       cos(lat_[i]*radish)*cos(lat_[index]*radish)*
                       sin(delta_lon/2)*sin(delta_lon/2);
            c        =2.0 * atan2(sqrt(a),sqrt(1-a));

            temp_distance = RADIUS*c;                
            
            pindex.push_back( id_pair(temp_distance,i) );
      }	
       
      //sort the data, i.e. by the distance to each neighbour
      //the value in the second part of the pair is the index in the original array 
       
 	  sort(pindex.begin(),pindex.end());
 	  	  
 	  //Find index for stations < max_distance km distant
 	  
 	  int imax=0;
          float max_distance=params.InterpolationLimit;
 	  
 	  for (unsigned int i=0 ; i<original_.size() ; i++) {
 	  	  if (pindex[i].first < max_distance) imax=i;
 	  	  }
 	   	  


 	  inv_dist = 0.0;
 	  weight   = 0.0;

          //bool DoInt=false;


 	  for (int i=1 ; i<imax+1 ; i++) {  //NB i=0 corresponds to the station for which we do an interpolation
 	  	  
 	  	  data_point=original_[pindex[i].second];
 	  	  
 	  	  if (data_point == -1) data_point = 0; // These are bone dry measurments
 	  	                                        // as opposed to days when there
 	  	                                        // may have been rain but none
 	  	                                        // was measurable
 	  	  
 	  	  if (imax > 1 && data_point > -1 && pindex[i].first > 0 && 
                                              CheckFlags.condition(controlinfo_[i],params.Iflag)) {

                        inv_dist += 1.0/(pindex[i].first*pindex[i].first); 
                        weight += data_point/(pindex[i].first*pindex[i].first);
                        NeighboursUsed.push_back(data_point);
 	  	  }
 	  }

          if (NeighboursUsed.size() > 0) {
          computeStats(NeighboursUsed.begin( ), NeighboursUsed.end( ), sum, mean, var, dev, skew, kurt);
          }

          /// How many neighbours to use is an open question.
          /// Best option is to do the triangulation !!!!!!!!!!!!!!!!!!! ??????????????+
 	  /// if (inv_dist > 0.0 && NeighboursUsed.size() > 0 && NeighboursUsed.size() < 7) {
 	  if (inv_dist > 0.0) {
 	     intp_[index] = weight/inv_dist; 
             CP_[index]=dev;
 	  }  
 
} 


/// Inverse distance weighting interpolation prototype.
/// Algorithm to construct a model value by inverse distance weighting from neighbouring stations.
/// Includes a 10 % modification to the rainfall with 100 m or altitude up to 1000m and 5% for
/// every 100m above 1000m.
void 
Qc2D::
calculate_intp_h(unsigned int index)
{
 	  const double RADIUS=6371.0;
 	  float temp_distance;
 	  float data_point;
 	  float data_point_h;
 	  float weight;
 	  float inv_dist;
 	  float delta_lat;
 	  float delta_lon;
          double a, c;
 	  const double radish = 0.01745329251994329509;
 	  
 	  typedef pair <float,int> id_pair;
 	  
 	  // pindex will hold the "station_distance" and the index in the
 	  // original data array.
 	  
 	  std::vector<id_pair> pindex;
 	  std::vector<id_pair>::const_iterator ip;

 	  for (unsigned int i=0 ; i<original_.size() ; i++) {
 	  	
            delta_lon=(lon_[index]-lon_[i])*radish;
            delta_lat=(lat_[index]-lat_[i])*radish;
            a        = sin(delta_lat/2)*sin(delta_lat/2) +
                       cos(lat_[i]*radish)*cos(lat_[index]*radish)*
                       sin(delta_lon/2)*sin(delta_lon/2);
            c        =2.0 * atan2(sqrt(a),sqrt(1-a));

            temp_distance = RADIUS*c;                
            
            pindex.push_back( id_pair(temp_distance,i) );
      }	
       
      //sort the data, i.e. by the distance to each neighbour
      //the value in the second part of the pair is the index in the original array 
       
 	  sort(pindex.begin(),pindex.end());
 	  	  
 	  //Find index for stations < 50 km distant ... up to a maximumu of 20 points
 	  
 	  int imax=0;
          float max_distance=100.0;
          //float max_distance=50.0;
 	  
 	  for (unsigned int i=0 ; i<original_.size() ; i++) {
 	  	  if (pindex[i].first < max_distance) imax=i;
 	  	  }


 	  inv_dist = 0.0;
 	  weight   = 0.0;
          //int idog=0;
 	  
 	  for (int i=1 ; i<imax+1 ; i++) {
 	  	  
 	  	  data_point=original_[pindex[i].second];
 	  	  
 	  	  if (data_point == -1) data_point = 0; // These are bone dry measurments
 	  	                                        // as opposed to days when there
 	  	                                        // may have been rain but none
 	  	                                        // was measurable

// reduce the data point to sea-level

 	  	 
 
                                                
 	  	  if (imax > 1 && data_point > -1 && pindex[i].first > 0 && 
 	  	                              controlinfo_[pindex[i].second].flag( 12 ) == 1  ) {
                          data_point_h = data_point;
                          if (ht_[pindex[i].second] > 1000.0) {
                               data_point = data_point - data_point*( (ht_[pindex[i].second]-1000) / 2000.0);
                          }
                          if (ht_[pindex[i].second] <= 1000.0) {
                               data_point = data_point - data_point*(ht_[pindex[i].second]/1000.0);
                          }
                          /*std::cout << "HEIGHT: " << stid_[pindex[i].second] <<" "
                                          << data_point_h << " "
                                          << data_point   << " "
                                          << lat_[pindex[i].second] <<" "
                                          << lon_[pindex[i].second] <<" "
                                          <<  ht_[pindex[i].second] <<" "
                                          << std::endl; */
                          //std::cout << "Still need to add 5% mod above 1000m" << std::endl;
 	  	  	  inv_dist += 1.0/(pindex[i].first*pindex[i].first); 
 	  	          weight += data_point/(pindex[i].first*pindex[i].first);
                          //idog += 1;
 	  	  }
 	  }
 	     

 	  if (inv_dist > 0.0) {
 	  
 	     intp_[index] = weight/inv_dist; 
             if (ht_[index] <= 1000.0){
 	         intp_[index] = intp_[index]+intp_[index]*(ht_[index]/1000.0); // put back to correct altitude
             }
             if (ht_[index] > 1000.0){
 	         intp_[index] = 2.0*intp_[index]; // the bit up to 1000 km !!!!!!!! 
                 intp_[index] = (intp_[index]+intp_[index]*(ht_[index]/2000.0)); // put back to correct altitude
             }
 	     //std::cout << intp_[index] << std::endl; 
 	  }  
          //std::cout << "Number of points used in interpolation = " << idog << std::endl;
 
}

/// perform an interpolation based on a list of allowed stations (sl).
void 
Qc2D::
calculate_intp_sl(unsigned int index, std::list<int> BestStations) 
{
 	  const double RADIUS=6371.0;
 	  float temp_distance;
 	  float data_point;
 	  float weight;
 	  float inv_dist;
 	  float delta_lat;
 	  float delta_lon;
          double a, c;
 	  const double radish = 0.01745329251994329509;

          unsigned int i;
 	  
          // For the given station there is a list of station ids which we can use!!! 
          // therefore do not need this complicated sorting algorithms as in the nearest neighbours 
          // approach ...
          // for now recode to analyse all neighbours!!!
          // this can be easily updated to manage a fixed station list

 	  inv_dist = 0.0;
 	  weight   = 0.0;

          //std::cout << stid_[index] << " ";
          //std::cout << index << " " << stindex[stid_[index]] << " ";
          //for (std::list<int>::const_iterator ist=BestStations.begin(); ist != BestStations.end(); ++ist){
               //std::cout << *ist << " ";
          //}
          //std::cout << std::endl;
          

          for (std::list<int>::const_iterator ist=BestStations.begin(); ist != BestStations.end(); ++ist){

              i=stindex[ *ist ];

              //std::cout << stid_[index] << " " << stid_[i] << std::endl;

              if (i != index){
                   delta_lon=(lon_[index]-lon_[i])*radish;
                   delta_lat=(lat_[index]-lat_[i])*radish;
                   a        = sin(delta_lat/2)*sin(delta_lat/2) +
                              cos(lat_[i]*radish)*cos(lat_[index]*radish)*
                              sin(delta_lon/2)*sin(delta_lon/2);
                   c        =2.0 * atan2(sqrt(a),sqrt(1-a));
  
                   temp_distance = RADIUS*c;                

                   data_point=original_[i];
 	  	   if (data_point == -1) data_point = 0; 

 	  	   if (data_point > -1 && temp_distance > 0 && controlinfo_[i].flag( 12 ) == 1 ){
                               inv_dist += 1.0/(temp_distance*temp_distance); 
                               weight += data_point/(temp_distance*temp_distance);
                               //inv_dist += 1.0/(temp_distance*temp_distance*temp_distance*temp_distance); 
                               //weight += data_point/(temp_distance*temp_distance*temp_distance*temp_distance);
 	  	       }
              }
          }	
 	     
 	  if (inv_dist > 0.0) {
 	     intp_[index] = weight/inv_dist; 
 	  }  
}

/// perform interpolation based on a list triangulation points. 
/// assuming a linear trend in precipitation in the latitude and longitude 
/// directions. This is an approximation, strict interpolation should be 
/// along geodescis???
void 
Qc2D::
calculate_trintp_sl(unsigned int index, std::list<int> BestStations) 
{

///This needs checking and testing

 	  //const double RADIUS=6371.0;
 	  const double radish = 0.01745329251994329509;

 	  double data_point;
          double x[3], y[3],rr[3];
          double xp[3], yp[3];

          double xt, yt;

          double Denom,d_rr_over_d_x,d_rr_over_d_y;

          unsigned int i;
          unsigned int j;

          bool OKTRI;

          //Setup and test the UTM business

         //std::string prms("proj=utm ellps=clrk80 lon_0=10 lat_0=50");
         //std::string prms("proj=utm ellps=clrk80");
         std::string prms("proj=utm lon_0=15e datum=WGS84"); //From Matthias
         proj pj(prms);
         projUV utm;


          //In this algorithm there are only three neighbours!

          if (BestStations.size() == 3){ 	 

               j=0;

               for (std::list<int>::const_iterator ist=BestStations.begin(); ist != BestStations.end(); ++ist){
                     OKTRI=true;
                     i=stindex[ *ist ];
                     //std::cout << "Station: "<< *ist << std::endl;

                     x[j]=(double) lon_[i]*radish;
                     y[j]=(double) lat_[i]*radish;        

                     utm.u=x[j];  //Longitude
                     utm.v=y[j];  //latitude

                     //utm.u *= DEG_TO_RAD;
                     //utm.v *= DEG_TO_RAD;   ///radish does this but probably DEG_TO_RAD is better !!!

                     utm = pj.ll2xy(utm);

                     xp[j] = utm.u;  //Longitude  -> X-cartesian
                     yp[j] = utm.v;  //Latitude   -> Y-cartesian


                     //std::cout << y[j] << " " << x[j] << std::endl;
                     //std::cout << utm.u << " " << utm.v << std::endl;

                     //xp[j]=x[j]; 
                     //yp[j]=sin( y[j] );  /// should I make a projection to a planar triangle or not ???
                     ///TO BE FIXED .. needs full conversion to UTM here
                     /// see notes from Matthias !!!!!!!!!!!!!!!!!!!!!!!

                     if (xp[j] < 0.009) OKTRI=false;
                     if (yp[j] < 0.009) OKTRI=false;


                     data_point=(double) original_[i];
 	  	     if (data_point == -1.0) data_point = 0; 
                     rr[j]=data_point;

                     ++j;
                  }

                if (rr[0] > -1 && rr[1] > -1 && rr[2] > -1 && OKTRI){

                     utm.u=(double) lon_[index]*radish;
                     utm.v=(double) lat_[index]*radish;        
                     utm = pj.ll2xy(utm);
                     xt = utm.u;
                     yt = utm.v;
       
                     //std::cout << rr[0] << " %% " << rr[1]  << " %% " << rr[2] << std::endl; 
                     Denom=(xp[2]-xp[0])*(yp[1]-yp[0])-(xp[1]-xp[0])*(yp[2]-yp[0]);
                     d_rr_over_d_x=((yp[1]-yp[0])*(rr[2]-rr[0])-(yp[2]-yp[0])*(rr[1]-rr[0]))/Denom;
                     d_rr_over_d_y=((xp[2]-xp[0])*(rr[1]-rr[0])-(xp[1]-xp[0])*(rr[2]-rr[0]))/Denom;
                     intp_[index]=rr[0]+(xt-xp[0])*d_rr_over_d_x+(yt-yp[0])*d_rr_over_d_y;
                     //intp_[index]=rr[0]+(lon_[index]*radish-xp[0])*d_rr_over_d_x+((sin(lat_[index]*radish))-yp[0])*d_rr_over_d_y;
    

                //std::cout << Denom << " "
                         //<< d_rr_over_d_x << " "
                         //<< d_rr_over_d_y << " "
                         //<< rr[0] << " "
                         //<< rr[1] << " "
                         //<< rr[2] << " "
                         //<< intp_[index] << " "
                         //<< original_[index] << " "
                         //<< corrected_[index] << " Fundango "
                         //<< std::endl; 
                }
          } 


} 


/// perform an inverse distanced weighted interpolation based on all the neighbours. 
void
Qc2D::
calculate_intp_all(unsigned int index)
{

          const double RADIUS=6371.0;
          float temp_distance;
          float data_point;
          float weight;
          float inv_dist;
          float delta_lat;
          float delta_lon;
          double a, c;
          const double radish = 0.01745329251994329509;

          ProcessControl CheckFlags;

          inv_dist = 0.0;
          weight   = 0.0;

          for (unsigned int i=0 ; i<original_.size() ; i++) {

              if (i != index){
                   delta_lon=(lon_[index]-lon_[i])*radish;
                   delta_lat=(lat_[index]-lat_[i])*radish;
                   a        = sin(delta_lat/2)*sin(delta_lat/2) +
                              cos(lat_[i]*radish)*cos(lat_[index]*radish)*
                              sin(delta_lon/2)*sin(delta_lon/2);
                   c        =2.0 * atan2(sqrt(a),sqrt(1-a));

                   temp_distance = RADIUS*c;

                   data_point=original_[i];
                   if (data_point == -1) data_point = 0;

                   //if (data_point > -1 && temp_distance > 0 && controlinfo_[i].flag( 12 ) == 1 ){
                   if (data_point > -1 && temp_distance > 0 && CheckFlags.condition(controlinfo_[i],params.Iflag) ){
                               inv_dist += 1.0/(temp_distance*temp_distance);
                               weight += data_point/(temp_distance*temp_distance);
                       }
              }
          }

          if (inv_dist > 0.0) {
             intp_[index] = weight/inv_dist;
          }
}


/// Interpolation based on the construction of local Delaunay triangles
/// and subsequent linear interpolation.
void 
Qc2D::
intp_delaunay(unsigned int index)
{
 	  const double RADIUS=6371.0;
 	  float temp_distance;
 	  float data_point;
 	  float weight;
 	  float inv_dist;
 	  float delta_lat;
 	  float delta_lon;
          double a, c;
 	  const double radish = 0.01745329251994329509;

          ProcessControl CheckFlags;

          std::vector<float> NeighboursUsed;
          NeighboursUsed.clear();
          double sum, mean, var, dev, skew, kurt;

 	  
 	  typedef pair <float,int> id_pair;
 	  
 	  // pindex will hold the "station_distance" and the index in the
 	  // original data array.
 	  
 	  std::vector<id_pair> pindex;
 	  std::vector<id_pair>::const_iterator ip;

 	  for (unsigned int i=0 ; i<original_.size() ; i++) {
 	  	
            delta_lon=(lon_[index]-lon_[i])*radish;
            delta_lat=(lat_[index]-lat_[i])*radish;
            a        = sin(delta_lat/2)*sin(delta_lat/2) +
                       cos(lat_[i]*radish)*cos(lat_[index]*radish)*
                       sin(delta_lon/2)*sin(delta_lon/2);
            c        =2.0 * atan2(sqrt(a),sqrt(1-a));

            temp_distance = RADIUS*c;                
            
            pindex.push_back( id_pair(temp_distance,i) );
      }	
       
      //sort the data, i.e. by the distance to each neighbour
      //the value in the second part of the pair is the index in the original array 
       
 	  sort(pindex.begin(),pindex.end());
 	  	  
 	  //Find index for stations < max_distance km distant
          //Only triangulate using this set of local stations!!!
 	  
 	  int imax=0;
          float max_distance=params.InterpolationLimit;
 	  
 	  for (unsigned int i=0 ; i<original_.size() ; i++) {
 	  	  if (pindex[i].first < max_distance) imax=i;
 	  	  }
 	   	  
          std::cout << "IMAX: " << imax << std::endl;

 	  inv_dist = 0.0;
 	  weight   = 0.0;

          //bool DoInt=false;
   
          std::cout << "--------------------------" << std::endl;
          std::cout << "Target:" << " "
                    << lon_[index] << " "
                    << lat_[index] << std::endl;
          std::cout << "Target Check:" << " "
                    << lon_[pindex[0].second] << " "
                    << lat_[pindex[0].second] << std::endl;

          std::cout << "Triangulate over these neighbours" << std::endl; 

          if (imax >=3){         // only makes sense for triangulation (must be at least threee nodes surrounding)


          // Setup projection engine business
          
                   std::string prms("proj=utm lon_0=15e datum=WGS84"); //From Matthias
                   proj pj(prms);
                   projUV utm;
          
                   std::vector<double> lat;
                   std::vector<double> lon;
                   std::vector<int> stid;
          
                   int node_num;
                   double *table;
                   int *triangle_neighbor;
                   int *triangle_node;
                   int triangle_num;
                   bool node_show = true;
                   bool triangle_show = true;

                   int result;
          
                   node_num=imax-1;  // These are only the neighbours 
                             table= new double[2*imax];
          
          
 	            for (int i=0 ; i<imax+1 ; i++) { 
          
          //Loop Through Stations And Convert To UTM co-ordinates
          
                        utm.u= lon_[pindex[i].second]*radish;   /// Use DEG_TO_RAD instead !!!!! ????????
                        utm.v= lat_[pindex[i].second]*radish;
                        utm = pj.ll2xy(utm);
             
                        lon.push_back( utm.u );
                        lat.push_back( utm.v );
                        if (i > 0) {                // i=0 corresponds to the target
                          table[2*(i-1)]=utm.u;
                          table[2*(i-1)+1]=utm.v;
                          std::cout << "TABLE: " << utm.u << " " << utm.v << std::endl;
                        }
                   }
          
                std::cout << "TARGET in UTM : " << lon[0] << " " << lat[0] << std::endl;
          
                int m2 = 3;
                triangle_node = new int[m2*3*imax];
                triangle_neighbor = new int[m2*3*imax];  /// Funny non-intuitive counting here array is (1+N) where n=1 is the centre???
          
          
                //Calculate the Triangles For Each Target Station
                std::cout << "CALLING DTRIS!" << std::endl;
                std::cout << "Fusion" << std::endl;
                std::cout << node_num << " " << table[0] << " " << table[1] << std::cout;
                result=dtris2 ( node_num, table, &triangle_num, triangle_node, triangle_neighbor );  
                std::cout << "ESCAPING FROM GEOMPACK ... " << std::endl;
          
                intp_[index]=-10.0;  /// This does nothing more yet !!!!!!!!!!!!!!!!!!!
                CP_[index]=-10.0;
                std::cout << "RESULT: " << result << std::endl; 
                if (!result) {  //-------------------------
                  cout << "\n";
                  cout << "  Computed the triangulation.\n";
                  cout << "  Number of triangles is " << triangle_num << "\n";
          
          //We have to find which triangle is home for our station
          
          //Loop over all triangles
                int tri;
                int point_within_triangle=-999;
                int point_within_how_many_triangles=0;                                   
                double x_triangle[3]; // corresponds to the longitude !!!
                double y_triangle[3]; // corresponds to the latitude  !!!
                double dot00,dot01,dot02,dot11,dot12,invDenom,u,v;

                std::cout  << "Arithmetic: " << *triangle_node << std::endl;
          
                for (tri=0; tri<triangle_num; ++tri) {
                // still do not understand the content of dtris output ... lets just print it all out and have a look ...
                // Table: coordinates at (0,1) correspond to the centre point. The coordinates at (2,3) are the first
                // neighbour ...
                   std::cout << triangle_node[3*tri] << " " <<
                   triangle_node[3*tri+1] << " " <<
                   triangle_node[3*tri+2] << std::endl;
                   std::cout << triangle_neighbor[3*tri] << " " <<
                   triangle_neighbor[3*tri+1] << " " <<
                   triangle_neighbor[3*tri+2] << std::endl;
                //ok that output made some sense to me ...

                //A triangle
                   x_triangle[0] = table[ 2*triangle_node[3*tri] ];
                   y_triangle[0] = table[ 2*triangle_node[3*tri] + 1 ];
                   std::cout << 2*triangle_node[3*tri] << " " <<  2*triangle_node[3*tri] + 1  << std::endl;

                   x_triangle[1] = table[ 2*triangle_node[3*tri+1] ];
                   y_triangle[1] = table[ 2*triangle_node[3*tri+1] + 1 ];
                   std::cout << 2*triangle_node[3*tri+1] << " " <<  2*triangle_node[3*tri+1] + 1  << std::endl;

                   x_triangle[2] = table[ 2*triangle_node[3*tri+2] ];
                   y_triangle[2] = table[ 2*triangle_node[3*tri+2] + 1 ];
                   std::cout << 2*triangle_node[3*tri+2] << " " <<  2*triangle_node[3*tri+2] + 1  << std::endl;

                   std::cout << "--------------------------------------------------------" << std::endl;
                 //Into which triangle does table[0],table[1] fall ?

//  Perform "Point in triangle test" according to Barycentric Technique (see http://www.blackpawn.com/texts/pointinpoly/default.html) 

                   dot00=(x_triangle[3]-x_triangle[1])*(x_triangle[3]-x_triangle[1]) + (y_triangle[3]-y_triangle[1])*(y_triangle[3]-y_triangle[1]);
                   dot01=(x_triangle[3]-x_triangle[1])*(x_triangle[2]-x_triangle[1]) + (y_triangle[3]-y_triangle[1])*(y_triangle[2]-y_triangle[1]);
                   dot02=(x_triangle[3]-x_triangle[1])*(lon[0]         -x_triangle[1]) + (y_triangle[3]-y_triangle[1])*(lat[1]         -y_triangle[1]);
                   dot11=(x_triangle[2]-x_triangle[1])*(x_triangle[2]-x_triangle[1]) + (y_triangle[2]-y_triangle[1])*(y_triangle[2]-y_triangle[1]);
                   dot12=(x_triangle[2]-x_triangle[1])*(lon[0]         -x_triangle[1]) + (y_triangle[2]-y_triangle[1])*(lat[1]         -y_triangle[1]);

                   invDenom = 1.0 / (dot00 * dot11 - dot01 * dot01);  
                   u = (dot11 * dot02 - dot01 * dot12) * invDenom;
                   v = (dot00 * dot12 - dot01 * dot02) * invDenom;

                   if(u >= 0. && v >= 0. && u + v <= 1.0) {         
                     point_within_triangle=tri;
                     point_within_how_many_triangles=point_within_how_many_triangles+1;
                   }           

           std::cout << "RESULT-> " << point_within_triangle << " " << point_within_how_many_triangles << std::endl;



           char epsfilename[]="outX";

           char buffer[30];
           sprintf(epsfilename,"%d%s",stid_[index],".eps");
           std::cout << epsfilename << std::endl;
           
           triangulation_order3_plot_eps ( epsfilename, node_num, table, triangle_num, triangle_node, node_show, triangle_show, lon[0], lat[0] );
           



                }
           }   //------------------ 
                delete [] table;
                delete [] triangle_node;
                delete [] triangle_neighbor;


}
//***********
 	  //***********for (int i=1 ; i<imax+1 ; i++) {  //NB i=0 corresponds to the station for which we do an interpolation
 	  	  //***********
 	  	  //***********data_point=original_[pindex[i].second];
 	  	  //***********
 	  	  //***********if (data_point == -1) data_point = 0; // These are bone dry measurments
 	  	                                        //***********// as opposed to days when there
 	  	                                        //***********// may have been rain but none
 	  	                                        //***********// was measurable
 	  	  //***********
 	  	  //***********if (imax > 1 && data_point > -1 && pindex[i].first > 0 && 
                                              //***********CheckFlags.condition(controlinfo_[i],params.Iflag)) {
//***********
                        //***********inv_dist += 1.0/(pindex[i].first*pindex[i].first); 
                        //***********weight += data_point/(pindex[i].first*pindex[i].first);
                        //***********NeighboursUsed.push_back(data_point);
 	  	  //***********}
 	  //***********}
//***********
          //***********if (NeighboursUsed.size() > 0) {
          //***********computeStats(NeighboursUsed.begin( ), NeighboursUsed.end( ), sum, mean, var, dev, skew, kurt);
          //***********}
//***********
          //***********/// How many neighbours to use is an open question.
          //***********/// Best option is to do the triangulation !!!!!!!!!!!!!!!!!!! ??????????????+
 	  //***********/// if (inv_dist > 0.0 && NeighboursUsed.size() > 0 && NeighboursUsed.size() < 7) {
 	  //***********if (inv_dist > 0.0) {
 	     //***********intp_[index] = weight/inv_dist; 
             //***********CP_[index]=dev;
 	  //***********}  
 
} 


///Method to append the interpolated model data to a netCDF file
int 
Qc2D::
write_cdf(const std::list<kvalobs::kvStation> & slist)
{

   int result=0;
   //static const int NC_ERR = 2;
   long counter=0;
   long netcdf_index;
  
   //static const int NX=slist.size(); 
   int NX=slist.size(); 
   static const int NP=3; 
   static const int NF=16; 
   
   long          CdfStation[NX];
   int           Cdftypeid[NX];
   float           Original[NX];
   float          Corrected[NX];
   float       Interpolated[NX];
   float       Redistributed[NX];
   float       ConfidenceParameter[NX];
   std::string        QFlag[NX];
   std::string        UFlag[NX];
   char               QQ[NX][16];
   char               UU[NX][16];
   float              Location[NX][NP];
   
   char dougal[17], zebedee[17];
   
   int month=obstime_[0].month();      /// Fix this here
   ///int month=obstime_[0].month();      ***********  Fix this here
   ///int month=obstime_[0].month();      ***********  Fix this here
   ///int month=obstime_[0].month();      ***********  Fix this here
   ///int month=obstime_[0].month();      ***********  Fix this here


    for ( std::list<kvalobs::kvStation>::const_iterator it = slist.begin(); it != slist.end(); ++ it )
    {
        CdfStation[counter]=it->stationID();
         
        std::vector<int>::iterator iv = std::find(stid_.begin(), stid_.end(), it->stationID());

        if (iv != stid_.end()) {
         	
         	vector<int>::difference_type d = distance(stid_.begin(), iv); 
            
            Cdftypeid[counter]=typeid_[d];

            Original[counter]=original_[d];
            Corrected[counter]=corrected_[d];
            Interpolated[counter]=intp_[d];
         	Redistributed[counter]=redis_[d];
         	ConfidenceParameter[counter]=CP_[d];
         	
            Location[counter][0]=lat_[d];
            Location[counter][1]=lon_[d];
            Location[counter][2]=ht_[d];
            
            QFlag[counter]=controlinfo_[d].flagstring();
            UFlag[counter]=useinfo_[d].flagstring();
            
            strcpy(dougal,  controlinfo_[d].flagstring().c_str());
            strcpy(zebedee, useinfo_[d].flagstring().c_str());
            for ( int k=0 ; k!=16 ; k++ ){
                 QQ[counter][k] = dougal[k];
                 UU[counter][k] = zebedee[k];
               }  
            } 
       else {
            Cdftypeid[counter]=999;
            Original[counter]=-999.0;
            Corrected[counter]=-999.0;
       	    Interpolated[counter]=-999.0;
            Redistributed[counter]=-999.0;
         	ConfidenceParameter[counter]=-999.0;
         	Location[counter][0]=-999.0;
            Location[counter][1]=-999.0;
            Location[counter][2]=-999.0;
   
              for (int k=0 ; k!=16 ; k++ ){
                 QQ[counter][k] = '-';
                 UU[counter][k] = '-';
               }  
            }
        counter++;
    }
   
   netcdf_index=0;

   NcFile dataFile("interpolations.nc", NcFile::New);
   if (dataFile.is_valid())
   {  
   	  NcDim* xDim  = dataFile.add_dim("x", NX); 
      NcDim* Loc   = dataFile.add_dim("loc",NP);
      NcDim* nFlag = dataFile.add_dim("flag",NF);
      NcDim* nTime = dataFile.add_dim("time");
            
      NcVar *intdata1 = dataFile.add_var("Interpolations", ncFloat,nTime, xDim);
      NcVar *intdata10 = dataFile.add_var("Redistributed", ncFloat,nTime, xDim);
      NcVar *intdata80 = dataFile.add_var("ConfidenceParameter", ncFloat,nTime, xDim);
      NcVar *intdata2 = dataFile.add_var("Original", ncFloat, nTime, xDim);
      NcVar *intdata20 = dataFile.add_var("Corrected", ncFloat, nTime, xDim);
      NcVar *intdata3 = dataFile.add_var("Location", ncFloat, nTime, xDim, Loc);
      NcVar *intdata5 = dataFile.add_var("ControlInfo", ncChar,nTime, xDim, nFlag);
      NcVar *intdata6 = dataFile.add_var("UseInfo", ncChar, nTime, xDim, nFlag);
      NcVar *intdata60 = dataFile.add_var("TypeID", ncInt, nTime, xDim);
      NcVar *intdata7 = dataFile.add_var("Time", ncInt, nTime);   
    
      intdata1->put(&Interpolated[0], 1, NX);
      intdata10->put(&Redistributed[0], 1, NX);
      intdata80->put(&ConfidenceParameter[0], 1, NX);
      intdata2->put(&Original[0],1, NX);
      intdata20->put(&Corrected[0],1, NX);
      intdata3->put(&Location[0][0], 1, NX, NP);   
      intdata5->put(&QQ[0][0],1,NX,NF);
      intdata6->put(&UU[0][0],1,NX,NF);   
      intdata60->put(&Cdftypeid[0],1,NX);   
      intdata7->put(&month,1);
      
   } else{
   	  
       NcFile dataFile("interpolations.nc", NcFile::Write);
       NcDim* nTime = dataFile.rec_dim();
       
       netcdf_index = nTime->size();
          
       if (dataFile.is_valid())
       {      
   	     NcVar *appdata1 = dataFile.get_var("Interpolations");
             NcVar *appdata10 = dataFile.get_var("Redistributed");
             NcVar *appdata80 = dataFile.get_var("ConfidenceParameter");
             NcVar *appdata2 = dataFile.get_var("Original");
             NcVar *appdata20 = dataFile.get_var("Corrected");
             NcVar *appdata3 = dataFile.get_var("Location");
             NcVar *appdata5 = dataFile.get_var("ControlInfo");
             NcVar *appdata6 = dataFile.get_var("UseInfo");
             NcVar *appdata60 = dataFile.get_var("TypeID");
             NcVar *appdata7 = dataFile.get_var("Time");   
       
             appdata1->set_cur( netcdf_index, 0);
             appdata10->set_cur( netcdf_index, 0);
             appdata80->set_cur( netcdf_index, 0);
             appdata2->set_cur( netcdf_index, 0);
             appdata20->set_cur( netcdf_index, 0);
             appdata3->set_cur( netcdf_index, 0,0);            
             appdata5->set_cur( netcdf_index, 0,0);
             appdata6->set_cur( netcdf_index, 0,0);
             appdata60->set_cur( netcdf_index, 0);
             appdata7->set_cur( netcdf_index );
 
             appdata1->put(&Interpolated[0], 1, NX);
             appdata10->put(&Redistributed[0], 1, NX);
             appdata80->put(&ConfidenceParameter[0], 1, NX);
             appdata2->put(&Original[0],1, NX);
             appdata20->put(&Corrected[0],1, NX);
             appdata3->put(&Location[0][0],1, NX, NP);   
             appdata5->put(&QQ[0][0],1,NX,NF);
             appdata6->put(&UU[0][0],1,NX,NF);   
             appdata60->put(&Cdftypeid[0],1,NX);   
             appdata7->put(&month,1);           
       }
   }
   
   //if (!dataFile.is_valid())
   //{
    //  cout << "Couldn't open file!\n";
     // return NC_ERR;
   //}
   
   dataFile.close();
   
   return result;
}

//
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
//

///Ad hoc filter. e.g. use to remove missing values before calculating the variance of a data set and 
///set -1s to 0s. e.g.: 
/// Object.filter(fdata, -1, 200.0, -1.0, 0.0);
void
Qc2D::
filter(vector<float>& fdata, float Min, float Max, float IfMod=0.0, float Mod=0.0)
{
  fdata.clear();
  float dude;
  for (unsigned int i=0 ; i<original_.size() ; i++) {
     dude = (original_[i] <= Max && original_[i] >= Min) ? original_[i] :params.missing;
     if (dude == IfMod) { dude=Mod; }
     if (dude != params.missing) {
           fdata.push_back(dude); 
           //std::cout << "| "<< original_[i] << " : " << dude << " ";
     }
  }
  //std::cout << std::endl;
}