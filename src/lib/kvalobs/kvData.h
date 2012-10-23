/*
  Kvalobs - Free Quality Control Software for Meteorological Observations

  $Id: kvData.h,v 1.1.2.2 2007/09/27 09:02:29 paule Exp $

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
#ifndef __kvData_h__
#define __kvData_h__

#include <ostream>
#include <kvalobs/kvDbBase.h>
#include <kvalobs/kvDataFlag.h>

/*
 * Created by DNMI/IT: borge.moe@met.no
 * at Tue Aug 28 07:53:16 2002
 */

namespace kvalobs{
  /**
   * \addtogroup  dbinterface
   *
   * @{
   */


  /**
   * \brief Interface to the table data in the kvalobs database.
   */


class kvData : public kvDbBase {
private:
  int              stationid_;
  miutil::miTime   obstime_;
  float            original_;
  int              paramid_;
  miutil::miTime   tbtime_;
  int              tbtimemsec_;
  int              typeid_;
  int              sensor_;
  int              level_;
  float            corrected_;
  kvControlInfo    controlinfo_;
  kvUseInfo        useinfo_;
  std::string cfailed_;

  void createSortIndex();

public:
  kvData() {clean();}
  kvData( const kvData &d );

  kvData(const dnmi::db::DRow &r){set(r);}
  kvData(int                      pos,
	 const miutil::miTime &   obt,
	 float                    org,
	 int                      par,
	 const miutil::miTime &   tbt,
	 int                      typ,
	 int                      sen,
	 int                      lvl,
	 float                    cor,
	 const kvControlInfo &    cin,
	 const kvUseInfo &        uin,
         const std::string & fai)
  { set(pos, obt, org, par, tbt, typ, sen, lvl, cor, cin, uin, fai);}

  bool set(int                      pos,
	   const miutil::miTime &   obt,
	   float                    org,
	   int                      par,
	   const miutil::miTime &   tbt,
	   int                      typ,
	   int                      sen,
	   int                      lvl,
	   float                    cor,
	   const kvControlInfo &    cin,
	   const kvUseInfo &        uin,
	   const std::string & fai);

  bool set(int                   pos,
	   const miutil::miTime& obt,
	   float                 org,
	   int                   par,
	   const miutil::miTime &tbt,
	   int                   typ,
	   int                   lvl);

  bool set(const dnmi::db::DRow&);

  void clean();


  // typeid for observations artificially created by kvalobs
  enum { kv_typeid = 5 };

  const char* tableName()           const {return "data";}

  kvData& operator=(const kvData &rhs );

  std::string toSend()   const;
  std::string toUpdate() const;
  std::string toUpload() const;
  std::string uniqueKey() const;

  int              stationID()   const { return stationid_;  }
  miutil::miTime   obstime()     const { return obstime_;    }
  float            original()    const { return original_;   }
  int              paramID()     const { return paramid_;    }
  miutil::miTime   tbtime()      const { return tbtime_;     }
  int              tbtimemsec()  const { return tbtimemsec_; }
  int              typeID()      const { return typeid_;     }
  int              sensor()      const { return sensor_;     }
  int              level()       const { return level_;      }
  float            corrected()   const { return corrected_;  }
  kvControlInfo    controlinfo() const { return controlinfo_;}
  kvUseInfo        useinfo()     const { return useinfo_;    }
  std::string cfailed()     const { return cfailed_;    }

  void tbtime( const miutil::miTime &tbtime, int msec=0);
  void typeID(int t)                       { typeid_      =t; }
  void corrected(float c)                  { corrected_   = c;}
  void controlinfo(const kvControlInfo &f) { controlinfo_ = f;}
  void useinfo(const kvUseInfo &f)         { useinfo_     = f;}
  void useinfo(int flag, char newVal);
  void cfailed(const std::string& cf) { cfailed_     =cf;}

  friend std::ostream& operator<<( std::ostream& output,
                                   const kvalobs::kvData &d );
};

/** @} */
};
#endif
