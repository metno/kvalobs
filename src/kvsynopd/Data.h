/*
  Kvalobs - Free Quality Control Software for Meteorological Observations

  $Id: Data.h,v 1.2.6.2 2007/09/27 09:02:22 paule Exp $

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
#ifndef __kvsynop_Data_h__
#define __kvsynop_Data_h__

#include <kvalobs/kvData.h>
#include <kvalobs/kvDataFlag.h>
#include <kvalobs/kvDbBase.h>
#include <iostream>

class Data : public kvalobs::kvDbBase {
private:
	int              stationid_;
	miutil::miTime   obstime_;
	std::string      original_;
	int              paramid_;
	int              typeid_;
	int              sensor_;
	int              level_;
	std::string      controlinfo_;
	std::string      useinfo_;


	void createSortIndex();

public:
	Data() {clean();}
	Data(const kvalobs::kvData &data){ set(data);}
	Data(const dnmi::db::DRow &r){set(r);}
	Data(int                      pos,
			const miutil::miTime    &obt,
			const std::string       &org,
			int                      par,
			int                      typ,
			int                      sen,
			int                      lvl,
			const std::string &controlinfo="",
			const std::string &useinfo="" )
	{ set(pos, obt, org, par, typ, sen, lvl, controlinfo, useinfo);}

	bool set(int                      pos,
			const miutil::miTime    &obt,
			const std::string       &org,
			int                      par,
			int                      typ,
			int                      sen,
			int                      lvl,
			const std::string &controlinfo="",
			const std::string &useinfo="");

	bool set(const dnmi::db::DRow&);
	bool set(const kvalobs::kvData &data);

	void clean();

	const char* tableName() const {return "data";}
	miutil::miString toSend()   const;
	miutil::miString toUpdate() const;
	miutil::miString uniqueKey() const;

	int              stationID()   const { return stationid_;  }
	miutil::miTime   obstime()     const { return obstime_;    }
	std::string      original()    const { return original_;   }
	int              paramID()     const { return paramid_;    }
	int              typeID()      const { return typeid_;     }
	int              sensor()      const { return sensor_-'0'; }
	int              level()       const { return level_;      }
	kvalobs::kvControlInfo controlinfo()const;
	kvalobs::kvUseInfo useinfo()const;

	friend std::ostream& operator<<( std::ostream& ost,
									 const Data& data );
};

std::ostream& operator<<( std::ostream& ost,
						  const Data& data );
#endif
