/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: rr.h,v 1.1.2.6 2007/09/27 09:02:16 paule Exp $                                                       

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
#ifndef __agregator_rr_h__
#define __agregator_rr_h__

#include "AbstractAgregator.h"
#include "paramID.h"
#include <kvalobs/kvData.h>
#include <puTools/miClock>
#include <set>

namespace boost {
  class thread;
}

namespace agregator
{
  /**
   * \brief Calculate RR_1 values based on RR_01 values, by adding all
   * RR_01 values together.
   *
   * As observations of type RR_01 only will come to kvalobs when it
   * is actually raining, this object behaves a little diffferently
   * from other agregator objects. Aggregates from RR_01 are made once 
   * every day, in its own thread (\c GenerateZero). Corrections to 
   * RR_01 values will still propagate to the RR_1 observation, if that
   * day's RR_01 aggregation has already been done.
   *
   */
  class rr_1 : public AbstractAgregator
  {
    boost::thread *thread;
    bool threadStopping;
  public:

    rr_1();

    virtual ~rr_1();

    bool threadIsStopping() const { return threadStopping; }

    /**
     * \return False unless trigger.original() == GenerateZero::obsVal()
     */
    virtual bool shouldProcess( const kvalobs::kvData &trigger,
				const kvDataList &observations );

    virtual float generateKvData( const kvDataList &data, 
				  const kvalobs::kvData &trigger );
  };


  /**
   * \brief A framework for calculating sum of rainfall over a period
   * of time.
   */
  class rr : public AbstractAgregator
  {
  public:
    /**
     * \brief Constructor. Set up the basic parameters of the
     * AbstractAgregator object.
     *
     * \param readParam The paramID that incoming data should
     * have. All data which comes to this object has this paramID.
     *
     * \param writeParam The paramID of generated data.
     *
     * \param interestingHours How many hours of data back in time are
     * we interested in?
     *
     * \param generateWhen The times of day when we will generate
     * data. Agregates will only be generated for these times.
     */
    rr( int readParam, int writeParam, int interestingHours, 
	const std::set<miutil::miClock> &generateWhen );

    virtual float generateKvData( const kvDataList &data, 
				  const kvalobs::kvData &trigger );
  };


  /**
   * \brief A specialisation of rr, for calculating RR_12, based on RR_1.
   */ 
  class rr_12 : public rr
  {
  public:
    /**
     * \brief Constructor.
     */
    rr_12();
  };


  /**
   * A specialisation of rr, for calculating RR_24, based on RR_12.
   */
  class rr_24 : public rr
  {
  public:
    /**
     * \brief Constructor.
     */ 
    rr_24();
    
    /**
     * \brief Determine if enough data has been received in order to
     * create an agregate.
     *
     * \param trigger The piece of data which triggered the call to
     * this object.
     *
     * \param observations The list returned by \a getRelevantObsList.
     *
     * \return true if exactly two relevant observations has been
     * found. False otherwise.
     */
    virtual bool shouldProcess( const kvalobs::kvData &trigger,
				const kvDataList &observations );

    virtual float generateKvData( const kvDataList &data, 
				  const kvalobs::kvData &trigger );
  };
}

#endif // __agregator_rr_h__
