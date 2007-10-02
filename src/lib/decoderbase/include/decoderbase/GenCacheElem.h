/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: GenCacheElem.h,v 1.1.2.2 2007/09/27 09:02:27 paule Exp $                                                       

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
#ifndef __decoderbase_gen_cache_element_h__
#define __decoderbase_gen_cache_element_h__


namespace kvalobs{
  /**
   * \brief The namespace the decoder interface is in.
   */
  namespace decoder{
    /**
     * \ingroup kvdecoder Data decoders
     * @{
     */
    
    /**
     * \brief A class used to hold values for generated type.
     *.
     */


    class GenCachElem{
      long stationid_;
      int  typeid_;
      bool generated_;
      
    public:
      GenCachElem(long sid, int tid, bool gen)
	:stationid_(sid), typeid_(tid), generated_(gen)
	{
	}
      
      GenCachElem(const GenCachElem &e)
	:stationid_(e.stationid_), typeid_(e.typeid_), 
	generated_(e.generated_)
	{
	}
      
      GenCachElem& operator=(const GenCachElem &rhs){
	if(this!=&rhs){
	  stationid_=rhs.stationid_;
	  typeid_=rhs.typeid_; 
	  generated_=rhs.generated_;
	}
	
	return *this;
      }
      
      long stationID() const { return stationid_;}
      int  typeID()const{ return typeid_;}
      bool generated()const{ return generated_;}
    };

    /** @} */
  }
}
#endif
