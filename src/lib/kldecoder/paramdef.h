/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: paramdef.h,v 1.1.6.2 2007/09/27 09:02:29 paule Exp $                                                       

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
#ifndef __paramdef_h__
#define __paramdef_h__

#include <string>

class ParamDef
{
  	std::string name_;
  	int         id_;
  	int         sensor_;
  	int         level_;
  	bool        code_;

 public:
  	ParamDef();
  	ParamDef(std::string &name, int id, int sensor, int level, bool code);
  	ParamDef(const ParamDef &pd);
  	~ParamDef();

  	ParamDef& operator=(const ParamDef &rhs);

  	std::string name()const { return name_; } 
  	int           id()const { return id_; }
  	int       sensor()const { return sensor_;}
  	int        level()const { return level_;}
  	bool        code()const { return code_;}
};

#endif
