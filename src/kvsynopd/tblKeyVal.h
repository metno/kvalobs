/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: tblKeyVal.h,v 1.1.6.2 2007/09/27 09:02:23 paule Exp $                                                       

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
#ifndef __kvsynop_tblKeyVal_h__
#define __kvsynop_tblKeyVal_h__

#include <kvalobs/kvDbBase.h>

class TblKeyVal : public kvalobs::kvDbBase {
private:
  std::string key_;
  std::string val_;
  
  void createSortIndex();

public:
  TblKeyVal() {clean();}
  TblKeyVal(const TblKeyVal &keyVal){ set(keyVal);}
  TblKeyVal(const dnmi::db::DRow &r){set(r);}
  TblKeyVal(const std::string &key,
	    const std::string &val)
  { set(key, val);}

  bool set(const std::string &key, const std::string &val);

  bool set(const dnmi::db::DRow&);
  bool set(const TblKeyVal &keyVal);
  
  TblKeyVal& operator=(const TblKeyVal &keyVal){
                  if(&keyVal!=this)
		    set(keyVal); 
		  return *this;
             }

  void clean();

  char* tableName()            const {return "keyval";}
  miutil::miString toSend()    const;
  miutil::miString toUpdate()  const;
  miutil::miString uniqueKey() const;
 
  std::string  key() const { return key_; } 
  std::string  val() const { return val_; } 
  void         val(const std::string &v) { val_=v; } 
};

#endif
