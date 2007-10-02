/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: SynopWorker.h,v 1.12.2.7 2007/09/27 09:02:23 paule Exp $                                                       

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
#ifndef __SynopWorker_h__
#define __SynopWorker_h__

#include <sstream>
#include "App.h"
#include <kvcpp/kvevents.h>
#include "DataList.h"
#include "SynopData.h"
#include "obsevent.h"
#include "StationInfo.h"


class SynopWorker {
  typedef enum{RdOK, 
		 RdNoStation, 
		 RdNoData, 
		 RdMissingObstime,
		 RdERROR} EReadData; 
  
  App                      &app;
  dnmi::thread::CommandQue &que;  
  dnmi::thread::CommandQue &replayQue;  
  dnmi::db::Connection     *con;
  std::ostringstream       &swmsg;

  /**
   * readData loads the data from the datacache. The 
   * data at the head is the newest data.
   * 
   * The data that is loaded is:
   *   from: event.obstime() - 25 h. (>=)
   *     to: event.obstime()         (<=)
   *
   * \return RdOK on success, 
   *         RdNoStation, when no stations is configured.
   *         RdNoData, when no data is retrievd form the data cache.
   *         RdMissingObstime, when data for the obstime is missing.
   *         RdError, on all others failures.
   *
   */
  EReadData readData(dnmi::db::Connection &con,
		     ObsEvent             &event,
		     DataEntryList        &data)const;
  
  void loadSynopData(const DataEntryList &dl, SynopDataList &sd, 
		     StationInfoPtr info)const;

  /**
   * checkTypes, check if all data types that is needed to
   * generate a synop is received.
   *
   * \param data A list of data to be used to encode a synop.
   * \param stInfo A pointer to the station information.
   * \param obstime The obstime to create a synop for.
   * \param mustHaveTypes An output variable. It is true if at least
   *                all must have type id is pressent for obstime.
   *
   * \return true if all data type ids are pressent for obstime. False
   *           otherwise.
   */
  bool checkTypes(const DataEntryList &data, 
		  StationInfoPtr      stInfo,
		  const miutil::miTime obstime,
		  bool  &mustHaveTypes)const;


  /**
   * readyForSynop, check if we are ready to create a synop 
   * for the pressent data. If we don't have enogh data delay
   * if necesarry. 
   */
  bool readyForSynop(const DataEntryList  &data, 
		     ObsEvent             &event)const;

  /**
   * saveTo, use the \e copy and \e copyto from the configuration file 
   * to save a copy of the \a wmomsg to disk. The \a wmomsg is saved
   * to the path given by \e copyto if \e copy is true. If \e copy is
   * false the \a wmomsg is not saved to disk.
   * 
   * The file is named: wmono-termin-ccx.synop, termin: hhDD.
   *  ex. 1492-0625.synop, Blindern, kl 06 dag 25.
   *      1492-0625-A.synop, same as above, but a CCA message.
   *
   * \param inf A pointer to the StationInfo for the station.
   * \param obstime The observation time to the \a wmomsg.
   * \param wmomsg The wmo message to save to disk.
   * \param ccx The CC? indicator.
   */
  void saveTo(StationInfoPtr inf,  
	      const miutil::miTime &obstime, 
	      const std::string &wmomsg, 
	      int ccx);

  bool checkContinuesTypes(ObsEvent            &event, 
			   const DataEntryList &data)const;

 public:
  SynopWorker(App &app, 
	      dnmi::thread::CommandQue &que,
	      dnmi::thread::CommandQue &replayQue);


  void newObs(ObsEvent &event);
  void operator()();
}; 



#endif
