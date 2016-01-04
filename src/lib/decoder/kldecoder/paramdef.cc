/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: paramdef.cc,v 1.1.6.2 2007/09/27 09:02:29 paule Exp $                                                       

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
#include "paramdef.h"

ParamDef::ParamDef()
    : id_(-1),
      sensor_(0),
      level_(0),
      code_(false) {
}

ParamDef::ParamDef(const std::string &name, int id, int sensor, int level,
                   bool code)
    : name_(name),
      id_(id),
      sensor_(sensor),
      level_(level),
      code_(code) {
}

ParamDef::ParamDef(const ParamDef &pd)
    : name_(pd.name_),
      id_(pd.id_),
      sensor_(pd.sensor_),
      level_(pd.level_),
      code_(pd.code_) {
}

ParamDef::~ParamDef() {
}

bool ParamDef::operator==(const ParamDef &rhs) const {
  if (rhs.name_ == name_ && rhs.id_ == id_ && rhs.sensor_ == sensor_
      && rhs.level_ == level_ && rhs.code_ == code_)
    return true;
  else
    return false;
}

bool ParamDef::operator!=(const ParamDef &rhs) const {
  return !(*this == rhs);
}

ParamDef&
ParamDef::operator=(const ParamDef &rhs) {
  if (this != &rhs) {
    name_ = rhs.name_;
    id_ = rhs.id_;
    sensor_ = rhs.sensor_;
    level_ = rhs.level_;
    code_ = rhs.code_;
  }

  return *this;
}

