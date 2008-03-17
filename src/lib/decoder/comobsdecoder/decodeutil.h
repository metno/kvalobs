/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: decodeutil.h,v 1.1.2.2 2007/09/27 09:02:24 paule Exp $                                                       

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
#ifndef __decode_util_h__
#define __decode_util_h__

#include "decodeddata.h"

namespace kvalobs {
  namespace decodeutil{

    bool doWdWdWd(kvalobs::decodeutil::DecodedDataElem &data, 
		  const std::string &buf);

    bool doKvEsss(kvalobs::decodeutil::DecodedDataElem &data, 
		  const std::string &Esss);

    bool doKvNhClCmCh(kvalobs::decodeutil::DecodedDataElem &data, 
		      const std::string &NhClCmCh);

    bool dohVV(kvalobs::decodeutil::DecodedDataElem &data, 
	       const std::string &hVV);

    bool doNsChshs(kvalobs::decodeutil::DecodedDataElem &data, 
		   const std::string &NsChshs);

    bool dowwW1W2(kvalobs::decodeutil::DecodedDataElem &data, 
		  const std::string &NsChshs);

    void doVT(kvalobs::decodeutil::DecodedDataElem &data, 
	      const std::string &vt,
	      const std::string &vt1, 
	      const std::string &vt2);
  }
}




#endif
