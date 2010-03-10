/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: decodeddata.h,v 1.1.2.3 2007/09/27 09:02:24 paule Exp $                                                       

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
#ifndef __DecodedData_h__
#define __DecodedData_h__

#include <sstream>
#include <kvalobs/kvData.h>
#include <kvalobs/kvTextData.h>
#include <kvalobs/paramlist.h>
#include <list>
#include <string>
#include <puTools/miTime.h>


namespace kvalobs{
  namespace decodeutil{

    class DecodedDataElem{
      friend class DecodedData;
      
      DecodedDataElem();
      
      
      std::list<kvalobs::kvData>     data_;
      std::list<kvalobs::kvTextData> textData_;
      const ParamList                *params;
      long                           sid;
      long                           tid;
      
      miutil::miTime dtObs;
      miutil::miTime tbtime;
            
      DecodedDataElem(const ParamList *params_, long stationid, long typeid_):
	params(params_),sid(stationid), tid(typeid_)
	{
	}
      
    public:
      
      DecodedDataElem(const DecodedDataElem &elem)
	:data_(elem.data_), textData_(elem.textData_),
	params(elem.params)
	{}
      
      DecodedDataElem& operator=(const DecodedDataElem &rhs)
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
      long stationID()const{ return sid; }
      long typeID()const{ return tid; }
  
      
    };

    
    std::ostream& operator<<(std::ostream&, const DecodedDataElem &el);
    typedef std::list<DecodedDataElem>                   TDecodedDataElem;
    typedef std::list<DecodedDataElem>::iterator        ITDecodedDataElem;
    typedef std::list<DecodedDataElem>::const_iterator CITDecodedDataElem;
      
    class DecodedData{
      DecodedData();
      DecodedData(const DecodedData &);
      DecodedData& operator=(const DecodedData &);
      
      
      const ParamList    &params;
      TDecodedDataElem data_;
      long         sid;
      long         tid;
      
    public:
      DecodedData(const ParamList &params, long stationid, long typeid_);
      ~DecodedData();
      
      static std::string createTimestamp(const miutil::miTime &date);
      DecodedDataElem        createDataElem()const{ 
	return DecodedDataElem(&params, sid, tid);
      } 
      
      long stationID()const{ return sid; }
      long typeID()const{ return tid; }
      void               add(const DecodedDataElem &elem);
      TDecodedDataElem       *data(){ return &data_;}
      
    };
  }
}

#endif
