/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: kvStationInfoCommand.h,v 1.1.2.2 2007/09/27 09:02:30 paule Exp $

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
#ifndef __kvStationInfoCommand_h__
#define __kvStationInfoCommand_h__

#include <kvskel/commonStationInfo.hh>
#include <kvalobs/kvStationInfo.h>
#include <kvalobs/kvStationInfoExt.h>
#include <dnmithread/CommandQue.h>

namespace kvalobs {

/**
 * \addtogroup kvinternalhelpers
 * @{
 */

/**
 * \brief A command that is used to communicate kvStationInfo
 * elements between threads in the kvalobs system.
 *
 * The command can carry more than one kvStationInfo elements.
 */
class StationInfoCommand : public dnmi::thread::CommandBase {
  kvalobs::kvStationInfoExtList stationInfoList;
  kvalobs::kvStationInfoList stationInfoListLegacy;
 public:
  StationInfoCommand();

  /**
   * \brief initialize from the CORBA object CKvalObs::StationInfoList.
   * \param stInfo a CORBA StationInfoList object.
   */
  StationInfoCommand(const CKvalObs::StationInfoList &stInfo);
  StationInfoCommand(const CKvalObs::StationInfoExtList &stInfo);

  /**
   * \brief initialize from the CORBA object CKvalObs::StationInfo.
   * \param a CORBA StationInfo object.
   */
  StationInfoCommand(const CKvalObs::StationInfo &stInfo);
  StationInfoCommand(const CKvalObs::StationInfoExt &stInfo);

  virtual ~StationInfoCommand() {
  }
  ;

  /**
   * \brief get a list of all kvStationInfo object this command carries.
   * \return a list of kvStationInfo objects.
   */
  kvalobs::kvStationInfoList& getStationInfo();
  const kvalobs::kvStationInfoList & getStationInfo() const;

  kvalobs::kvStationInfoExtList& getStationInfoExt() {
    return stationInfoList;
  }
  const kvalobs::kvStationInfoExtList & getStationInfoExt() const {
    return stationInfoList;
  }

  /**
   *  \brief add a kvStationInfo object to this command.
   *
   * \param stationInfo The kvStationInfo object to add.
   * \return true on success and false otherwise.
   */
  bool addStationInfo(const kvStationInfo &stationInfo);

  bool addStationInfo(const kvStationInfoExt &stationInfo);

  bool executeImpl() {
    return true;
  }
  ;  //Do nothing

  void debugInfo(std::ostream &info) const;

};

/** @} */

}
;
#endif
