/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: DataSrcImpl.h,v 1.4.2.2 2007/09/27 09:02:16 paule Exp $                                                       

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
#ifndef __DataSrcImpl_h__
#define __DataSrcImpl_h__

#include <iostream>
#include <kvskel/datasource.hh>
#include <kvskel/adminInterface.h>
#include "DataSrcApp.h"

/**
 * \addtogroup kvDatainputd
 *
 * @{
 */

/**
 * \brief Implementation of CORBA interface CKvalObs::CDataSource::Data.
 */
class DataSrcImpl : public virtual POA_CKvalObs::CDataSource::Data,
    public virtual micutil::AdminInterface,
    public PortableServer::RefCountServantBase {
  DataSrcApp &app;

  // Make sure all instances are built on the heap by making the
  // destructor non-public
  virtual ~DataSrcImpl();
 public:
  DataSrcImpl(DataSrcApp &app);

  //Defined IDL attributes and operations
  CKvalObs::CDataSource::Result* newData(const char* data, const char* obsType);

};

/** @} */
#endif
