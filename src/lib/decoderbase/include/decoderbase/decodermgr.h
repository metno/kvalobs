/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: decodermgr.h,v 1.1.2.2 2007/09/27 09:02:27 paule Exp $                                                       

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
#ifndef __dnmi_decoder_DecoderMgr_h__
#define __dnmi_decoder_DecoderMgr_h__

#include <puTools/miString>
#include <db/db.h>
#include <fileutil/dso.h>
#include <decoderbase/decoder.h>
#include <kvalobs/kvTypes.h>
#include <list>

/**
 * \addtogroup kvdecoder
 *
 * @{
 */

extern "C" {
    typedef kvalobs::decoder::DecoderBase* 
            (*decoderFactory)(dnmi::db::Connection &con_,
			      const ParamList &params,
			      const std::list<kvalobs::kvTypes> &typeList,
			      int   decoderId_,
			      const miutil::miString &observationType_,
			      const miutil::miString &observation_);

    typedef void (*releaseDecoderFunc)(kvalobs::decoder::DecoderBase* decoder);

    typedef std::list<miutil::miString> (*getObsTypes)();
}

namespace kvalobs{
  namespace decoder{
    
    /**
     * \brief DecoderMgr is responsible for loading of decoders. 
     */
    class DecoderMgr{
      struct DecoderItem{
	decoderFactory     factory;
	releaseDecoderFunc releaseFunc;
	dnmi::file::DSO    *dso;
	time_t             modTime;
	int                decoderCount;
	int                decoderId;
	std::list<miutil::miString> obsTypes;
	
	
	DecoderItem(decoderFactory     factory_, 
		    releaseDecoderFunc releaseFunc_, 
		    dnmi::file::DSO    *dso_, 
		    time_t             mTime)
	  :factory(factory_), releaseFunc(releaseFunc_), dso(dso_), 
	     modTime(mTime), decoderCount(0), decoderId(-1)
	{
	}

	~DecoderItem(){
	  delete dso;
	}
      };
      
      typedef std::list<DecoderItem*>                 DecoderList;
      typedef std::list<DecoderItem*>::iterator       IDecoderList;
      typedef std::list<DecoderItem*>::const_iterator CIDecoderList;
    
      DecoderList      decoders;
      miutil::miString decoderPath;
      
    public:
      DecoderMgr(const miutil::miString &decoderPath_);
      DecoderMgr(){};
      ~DecoderMgr();
      void setDecoderPath(const miutil::miString &decoderPath_);
  

      /**
       * returns true when all decoder has a decoderCount of 0.
       */
      bool readyForUpdate();
      void updateDecoders();


      DecoderBase *findDecoder(dnmi::db::Connection   &connection,
			       const ParamList        &params,
			       const std::list<kvalobs::kvTypes> &typeList,
			       const miutil::miString &obsType,
			       const miutil::miString &obs,
			       miutil::miString &errorMsg);

      void releaseDecoder(DecoderBase *dec);
      
      int  numberOfDecoders()const{ return decoders.size(); }
      void obsTypes(std::list<std::string> &list);
    };
    
    /** @} */
  }
}
      
#endif
