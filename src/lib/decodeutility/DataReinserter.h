/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: DataReinserter.h,v 1.1.2.2 2007/09/27 09:02:27 paule Exp $                                                       

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
#ifndef __kvalobs__DataReinserter_h__
#define __kvalobs__DataReinserter_h__

#include <cstdlib>
#include <string>
#include <list>
#include <kvalobs/kvData.h>
#include <kvskel/datasource.hh>
#include "kvalobsdata.h"

namespace kvalobs {

  /**
   * @brief Handles the reinsertion of manually corrected observations.
   *
   * DataReinserter objects handles communication with a \em
   * CKvalObs::CDataSource::Data object (e.g the kvDataInput daemon's
   * data entry interface). A connection to the Data object is made as
   * the \em DataReinserter is created, or by calling the \em connect
   * method. Corrections to data may by submitted by using one of the \em
   * insert methods.
   */
  template<class kvApp>
  class DataReinserter {

  public:

    DataReinserter( kvApp *app, int operatorID );

    /**
     *
     */
    virtual ~DataReinserter();

    /**
     * @deprecated
     *
     * \brief Submit a single piece of data to the Data object.
     *
     * @param d The data to be sent.
     *
     * @return the result of the information. See \em
     * CKvalObs::CDataSource::Result_var for more information.
     *
     * @throw CORBA::SystemException (of some variant) if any
     * connection related error occurs. Specifically, if connection is
     * lost, CORBA::TRANSIENT will be thrown.
     */
    virtual const CKvalObs::CDataSource::Result_var
      insert( kvalobs::kvData &d ) const;

    /**
     * @deprecated
     *
     * Submit a list of data to the Data object.
     *
     * @param dl The data to be sent.
     *
     * @return the ressult of the information. See \em
     * CKvalObs::CDataSource::Result_var for more information.
     *
     * @throw CORBA::SystemException (of some variant) if any
     * connection related error occurs. Specifically, if connection is
     * lost, CORBA::TRANSIENT will be thrown.
     */
    virtual const CKvalObs::CDataSource::Result_var
      insert( std::list<kvalobs::kvData> &dl ) const;

    /**
     * @brief Submit a list of kvData and kvTextData objects to kvalobs.
     *
     * @param data The list of data to be submitted.
     *
     * @throw CORBA::SystemException (of some variant) if any
     * connection related error occurs. Specifically, if connection is
     * lost, CORBA::TRANSIENT will be thrown.
     */
    virtual const CKvalObs::CDataSource::Result_var
        insert( const kvalobs::serialize::KvalobsData & data ) const;


    /**
     * Get a reference to the kvApp instance in use.
     *
     * @return A kvApp reference.
     */
    virtual const kvApp *getApp() const { return app; }

  protected:

    kvApp *app;
    const int operatorID;
    std::string logfilename;

    static const char *decoderID;
  };
}

#include "bits/DataReinserter.tcc"

#endif // __kvalobs__DataReinserter_h__
