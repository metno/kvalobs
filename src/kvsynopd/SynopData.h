/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: SynopData.h,v 1.8.6.6 2007/09/27 09:02:23 paule Exp $                                                       

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
#ifndef __SynopData_h__
#define __SynopData_h__

#include <stdexcept>
#include <list>
#include <string>
#include <puTools/miTime.h>


class  SynopData
{          
  	miutil::miTime time_;

  	friend class SynopDataList;
  	//  friend class SynopDataList::SynopDataProxy  
 public:
  	float  tempNaa;       
  	float  tempMid;       
  	float  tempMin;       
  	float  tempMax;       
  	float  fuktNaa;       
  	float  fuktMid;       
  	float  vindHastNaa;   
  	float  vindHastMid;   
  	float  vindHastGust;
  	float  vindHastMax;   
  	float  FX_3;
  	float  vindRetnNaa;   
  	float  vindRetnMid;   
  	float  vindRetnGust;  
  	float  vindRetnMax;
  	float  DX_3;   
  	float  nedboerTot;    
  	float  nedboer1Time; 
  	float  nedboer2Time;
  	float  nedboer3Time;  
  	float  nedboer6Time;
  	float  nedboer9Time;
  	float  nedboer12Time;
  	float  nedboer15Time;
  	float  nedboer18Time;
  	float  nedboer24Time; 
  	float  nedboerJa;     
  	float  trykkQFENaa;  //PO, stasjonstrykk.    
  	float  trykkQFEMid;  //POM, stasjonstrykk.  
  	float  trykkQFEMin;  //PON, stasjonstrykk.  
  	float  trykkQFEMax;  //POX, stasjonstrykk.  
  	float  trykkQNHNaa;  //PH, trykk redusert til havets niv�, ICAO standard.   
  	float  trykkQFFNaa;  //PR, trykk redusert til havets niv�.
  	float  trykkTendens;  
  	float  TAN_12;
  	float  TAX_12;
  	float  TW;
  	float  TWM;
  	float  TWN;
  	float  TWX;
  	float  TGN;
  	float  TGN_12;
  	float  FG;    //Gust since last observation
  	float  FX;    //Max wind since last observation
  	float  WAWA;
  	float  HLN;
  	float  EM;    //Snow state to the gound (Markas tilstand).
  	float  SA;    //Snow depth.
  	float  Vmor;  //Automatic measured horizontal visibility
  	float  VV;    //Human estimated horizontal visibility
  	float  HL;
  	std::string nedboerInd_verInd;
  	std::string hoeyde_sikt;
  	std::string skydekke;
  	//  std::string nedboermengde;
  	std::string verGenerelt;
  	std::string skyer;
  	std::string verTillegg;
  	std::string skyerEkstra1;
  	std::string skyerEkstra2;
  	std::string skyerEkstra3;
  	std::string skyerEkstra4;
  	std::string sjoeTemp;
  	std::string sjoegang;
  	std::string snoeMark;
  	std::string AA;
  	std::string ITZ;
  	std::string IIR; //nedb�r indikator
  	std::string ITR;

  	SynopData();
  	SynopData(const SynopData &p);
  	SynopData& operator=(const SynopData &p);
  	~SynopData();

  	bool setData(const int  &param, 
				 const std::string &data_);

  	/**
  	 * Removes data that only generates groups with slashes.
  	 */
  	void cleanUpSlash();

  	void           time(const miutil::miTime &t){time_=t;}
  	miutil::miTime time()const{ return time_;}

  	bool undef()const{ return time_.undef();}

  	friend std::ostream& operator<<(std::ostream& ost,
									  const SynopData& sd);
};

typedef std::list<SynopData>                   TSynopDataList;
typedef std::list<SynopData>::iterator        ISynopDataList;
typedef std::list<SynopData>::const_iterator  CISynopDataList;

class SynopDataList{
  	TSynopDataList  dataList;

  	friend class SynopDataProxy;

  	//setTime is a hack to set the time_ field in SynopData. It is 
  	//needed because I cant manage to make SynopDataProxy 
  	//a friend of SynopData. I am not sure if this is a defect of
  	//g++ 3.3.x or if the construct 'friend class SynopDataList::SynopDataProxy'
  	//is ilegal c++. I cant see any reason why this shouldnt be allowed.

  	void setTime(std::list<SynopData>::iterator it, 
			     const miutil::miTime &t){ it->time_=t;}
public:
  
  	class SynopDataProxy{
    	//SynopDataProxy is a helper class that is used 
    	//to deceide if the array operator [] is used
    	//as a lvalue or a rvalue.
    
    	SynopDataList                  *sdl;
    	miutil::miTime                 timeIndex;
    
  	public:
    	SynopDataProxy(SynopDataList *sdl_,
					   const miutil::miTime &t)
      		:sdl(sdl_), timeIndex(t){}

    	SynopDataProxy& operator=(const SynopData &rhs); //used as lvalue use
    
    	operator SynopData()const; //used as rvalue
 	};

  	SynopDataList();
  	SynopDataList(const SynopDataList &d);
  	~SynopDataList();  
  
  	//SynopDataList& operator=(const SynopDataList &rhs);

  	void clear(){ dataList.clear();}

  	/**
  	 * If used as a lvalue the SynopData record wil be inserted if it don't
  	 * exist.  The current record at timeIndex will be replaced if it exist.
  	 * if we use the operator as a rvalue it will throw std::out_of_range
  	 * if there is now SynopData record at timeIndex.
  	 *
  	 * \exception std::out_of_range, used as rvalue, if there is now SynopData 
  	 *            at timeIndex.
  	 */
  	const SynopDataProxy operator[](const miutil::miTime &timeIndex)const;
  	SynopDataProxy operator[](const miutil::miTime &timeIndex);
  
  	/**
  	 * \exception std::out_of_range if index is not in [0, size()>
  	 */
  	const SynopData& operator[](const int index)const;
  	SynopData&       operator[](const int index);
  

  	/**
  	 * Returns the number of times, from the start of the list, that
  	 * has one hour diffs between them.
  	 */
  	int nContinuesTimes()const;

  	/**
  	 * Insert SynopData at timeIndex in the list.
  	 * 
  	 * \param timeIndex, the time the SynopData (sd) is for.
  	 * \param sd the SynopData to insert at timeIndex.
  	 * \param replace, shall sd replace the SynopData at timeIndex if
  	 *        it exist.
  	 *
  	 * \return true on success, false otherwise. It may only return false if
  	 *         replace is false and there allready is a SynopData record at 
  	 *         timeIndex.
  	 */
 	 bool      insert(const miutil::miTime &timeIndex,
		   			  const SynopData &sd,
		   			  bool replace=false);

  	int       size()const { return dataList.size();}

  	ISynopDataList find(const miutil::miTime &from);
  	CISynopDataList find(const miutil::miTime &from)const;
    
  	ISynopDataList  begin(){ return dataList.begin();}
  	CISynopDataList begin()const{ return dataList.begin();}
  	ISynopDataList  end(){ return dataList.end();}
  	CISynopDataList end()const { return dataList.end();}

  	friend std::ostream& operator<<(std::ostream& ost,
				 					  const SynopDataList& sd);
};


std::ostream& operator<<(std::ostream& ost,
						  const SynopData& sd);

std::ostream& operator<<(std::ostream& ost,
						  const SynopDataList& sd);

#endif
