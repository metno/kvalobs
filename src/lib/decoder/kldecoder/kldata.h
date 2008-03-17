/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kldata.h,v 1.1.2.3 2007/09/27 09:02:29 paule Exp $                                                       

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
#ifndef __kvalobs_decoder_kldecoder_kldata_h__
#define __kvalobs_decoder_kldecoder_kldata_h__

#include <string>
#include <vector>
#include <kvalobs/kvDataFlag.h>

namespace kvalobs{
  	namespace decoder{
    	namespace kldecoder{

      	class KlData{
			std::string            val_;
			kvalobs::kvControlInfo cinfo_;
			kvalobs::kvUseInfo     uinfo_;
	
      	public:
			KlData(){}
			KlData(const KlData &d)
	  			:val_(d.val_), cinfo_(d.cinfo_), uinfo_(d.uinfo_){}
			KlData(const std::string &v, 
	       		   const kvalobs::kvControlInfo &c,
	       		   const kvalobs::kvUseInfo &u):val_(v), cinfo_(c), uinfo_(u){}
	
			KlData& operator=(const KlData &rhs){
	  					if(this!=&rhs){
	    					val_=rhs.val_;
	    					cinfo_=rhs.cinfo_;
	    					uinfo_=rhs.uinfo_;
	  					}
	  
	 					return *this;
					}
	
			bool empty()const { return val_.empty();}
			std::string              val()const { return val_;}
			kvalobs::kvControlInfo cinfo()const { return cinfo_; }
			kvalobs::kvUseInfo     uinfo()const { return uinfo_;}
      	};
    
      	typedef std::vector<KlData>                 KlDataArray;
      	typedef std::vector<KlData>::iterator       IKlDataArray;
      	typedef std::vector<KlData>::const_iterator CIKlDataArray;
    }
  }
}
#endif
