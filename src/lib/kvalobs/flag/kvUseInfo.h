/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 Copyright (C) 2010 met.no

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

#ifndef KVUSEINFO_H_
#define KVUSEINFO_H_

#include "kvDataFlag.h"

namespace kvalobs {

class kvControlInfo;

/**
 * \brief  UseInfo DataFlag
 */
class kvUseInfo : public kvDataFlag {
 public:
  kvUseInfo();
  kvUseInfo(const std::string& s);
  kvUseInfo(const unsigned char f[kvDataFlag::size]);
  kvUseInfo(const kvUseInfo& df);
  explicit kvUseInfo(const kvDataFlag& df);

  /// clear flag
  void clear();

  /**
   * \brief set Useinfo-flags, based on values in a controlflag.
   */
  bool setUseFlags(const kvControlInfo& cinfo);

  /**
   * \brief add 1 to error-count
   */
  void addToErrorCount();

  /**
   * \brief error count
   */
  int ErrorCount() const;

  /**
   * \brief set confidence (0-100)
   */
  void Confidence(const int& c);
  /**
   * \brief get confidence (0-100)
   */
  int Confidence() const;

  /**
   * \brief set HQC observer-id (0 - 255)
   */
  void HQCid(const int& c);

  /**
   * \brief get HQC observer-id
   */
  int HQCid() const;

  kvUseInfo& operator=(const kvUseInfo &rhs);
  kvUseInfo& operator=(const kvDataFlag &rhs);
  bool operator==(const kvUseInfo& rhs) const;
  bool operator!=(const kvUseInfo& rhs) const {
    return !(*this == rhs);
  }
  friend std::ostream& operator<<(std::ostream& output,
                                  const kvalobs::kvUseInfo& kd);
};

/** @} */
}

#endif /* KVUSEINFO_H_ */
