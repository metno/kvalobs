/*
  Kvalobs - Free Quality Control Software for Meteorological Observations

  $Id: decodeutility.h,v 1.1.2.3 2007/09/27 09:02:27 paule Exp $

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


#include "getUseInfo7.h"

namespace decodeutility {


int
getUseinfo7Code( int typeId,
                 const boost::posix_time::ptime &now,
                 const boost::posix_time::ptime &obt,
                 const std::list<kvalobs::kvTypes> &typeList )
{
	std::list<kvalobs::kvTypes>::const_iterator it=typeList.begin();

	for(; it!=typeList.end(); it++){
		if( it->typeID()==typeId )
			break;
	}

	if( it == typeList.end() )
		return -1;

	int diff = (now - obt).total_seconds() / 60; // difference in minutes

	if(diff > it->lateobs()) //tbt>obt  (diff>=0)
		return 4;
	else if(diff < (-1*it->earlyobs())) //tbt<obt (diff<0)
		return 3;
	else
		return 0;
}

}

