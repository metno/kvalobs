/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: smsdata.h,v 1.3.2.2 2007/09/27 09:02:24 paule Exp $                                                       

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
#ifndef __SmsData_h__
#define __SmsData_h__

#include <sstream>
#include <kvalobs/kvData.h>
#include <kvalobs/kvTextData.h>
#include <kvalobs/paramlist.h>
#include <list>
#include <string>
#include <puTools/miTime>


namespace kvalobs{
  namespace decoder{
    namespace comobsdecoder{

      class SmsDataElem{
	friend class SmsData;
	
	SmsDataElem();
	

	std::list<kvalobs::kvData>     data_;
	std::list<kvalobs::kvTextData> textData_;
	const ParamList                *params;
	long                           sid;
	long                           tid;

	miutil::miTime dtObs;
	miutil::miTime tbtime;
	


	SmsDataElem(const ParamList *params_, long stationid, long typeid_):
	  params(params_),sid(stationid), tid(typeid_)
	  {
	  }

      public:
	
	SmsDataElem(const SmsDataElem &elem)
	  :data_(elem.data_), textData_(elem.textData_),
	  params(elem.params)
	  {}
	
	SmsDataElem& operator=(const SmsDataElem &rhs)
	  {
	    if(this!=&rhs){
	      params=rhs.params;
	      data_=rhs.data_;
	      textData_=rhs.textData_;
	      dtObs=rhs.dtObs;
	      tbtime=rhs.tbtime;
	    }
	    
	    return *this;
	  }

	bool findParam(const std::string &pname, 
		       long &paramid, 
		       bool &isText)const;

	std::string findParam(long paramid)const;

	
	bool addData(const std::string &param, 
		     const std::string &data,
		     const kvUseInfo &uf=kvalobs::kvUseInfo(),
		     const kvalobs::kvControlInfo &cf=kvalobs::kvControlInfo(),
		     const int  level=0,
		     const int  sensor=0);
	
	void setDate(const miutil::miTime &date){ 
	                dtObs=date;
			tbtime=miutil::miTime::nowTime();
	              }

	const std::list<kvalobs::kvData> &data()const{ return data_;}
	const std::list<kvalobs::kvTextData> &textData()const{ 
	                                                  return textData_;
                                                        }
	miutil::miTime getDate()const{ return dtObs;}  
	void clean(){ data_.clear(); textData_.clear();dtObs=miutil::miTime();}

	friend std::ostream& operator<<(std::ostream&, const SmsDataElem &el);
      };


      typedef std::list<SmsDataElem>                   TSmsDataElem;
      typedef std::list<SmsDataElem>::iterator        ITSmsDataElem;
      typedef std::list<SmsDataElem>::const_iterator CITSmsDataElem;
      
      class SmsData{
	SmsData();
	SmsData(const SmsData &);
	SmsData& operator=(const SmsData &);
	

	const ParamList    &params;
	TSmsDataElem data_;
	long         sid;
	long         tid;
	
      public:
	SmsData(const ParamList &params, long stationid, long smscode);
	~SmsData();
	
	static std::string createTimestamp(const miutil::miTime &date);
	SmsDataElem        createDataElem()const{ 
	                         return SmsDataElem(&params, sid, tid);
	                   } 
	void               add(const SmsDataElem &elem);
	TSmsDataElem       *data(){ return &data_;}

      };
    }
  }
}

#endif
