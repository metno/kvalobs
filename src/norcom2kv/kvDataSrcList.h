/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvDataSrcList.h,v 1.1.6.2 2007/09/27 09:02:37 paule Exp $                                                       

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
#ifndef __kvDataSrcList_h__
#define __kvDataSrcList_h__

#include <string>
#include <kvskel/datasource.hh>
#include <list>

class KvDataSrc{
  std::string                     name_;
  CKvalObs::CDataSource::Data_var ref_;

 public:
  KvDataSrc(const std::string &name__, 
	    CKvalObs::CDataSource::Data_ptr ref__=CKvalObs::CDataSource::Data::_nil())
    :name_(name__), ref_(ref__)
    {
    }

  KvDataSrc():ref_(CKvalObs::CDataSource::Data::_nil()){}

  ~KvDataSrc(){}

  KvDataSrc& operator=(const KvDataSrc &lhs){
    if(this!=&lhs){
      name_=lhs.name_;
      ref_=CKvalObs::CDataSource::Data::_duplicate(lhs.ref_);
    }
    return *this;
  }

  std::string name()const { return name_; }
  CKvalObs::CDataSource::Data_ptr ref()const{ return ref_;}
  void ref(const CKvalObs::CDataSource::Data_ptr ref__){ ref_=ref__;}
};

typedef std::list<KvDataSrc>                   TKvDataSrcList;
typedef std::list<KvDataSrc>::iterator        ITKvDataSrcList;
typedef std::list<KvDataSrc>::const_iterator CITKvDataSrcList;

#endif
