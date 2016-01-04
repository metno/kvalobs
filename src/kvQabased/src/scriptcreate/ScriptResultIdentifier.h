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

#ifndef ScriptResultIdentifier_H_
#define ScriptResultIdentifier_H_

#include <Exception.h>
#include <string>
#include <iosfwd>

namespace qabase {

/**
 * Parsed version of the result of having run a check script.
 *
 * \ingroup group_scriptcreate
 */
class ScriptResultIdentifier {
 public:
  enum CorrectionType {
    Undefined,
    Flag,
    Corrected,
    Missing
  };

  ScriptResultIdentifier()
      : timeIndex_(0),
        stationIndex_(0),
        correctionType_(Undefined) {
  }
  explicit ScriptResultIdentifier(const std::string & identifier);
  ScriptResultIdentifier(const std::string & parameter, int timeIndex,
                         int stationIndex, CorrectionType correctionType);

  const std::string & parameter() const {
    return parameter_;
  }
  int timeIndex() const {
    return timeIndex_;
  }
  int stationIndex() const {
    return stationIndex_;
  }
  CorrectionType correctionType() const {
    return correctionType_;
  }

  void parameter(const std::string & val) {
    parameter_ = val;
  }
  void timeIndex(int val) {
    timeIndex_ = val;
  }
  void stationIndex(int val) {
    stationIndex_ = val;
  }
  void correctionType(CorrectionType val) {
    correctionType_ = val;
  }

  QABASE_EXCEPTION(Exception);QABASE_DERIVED_EXCEPTION(SyntaxError, Exception);QABASE_DERIVED_EXCEPTION(UndefinedCorrectionType, Exception);

 private:
  std::string parameter_;
  int timeIndex_;
  int stationIndex_;
  CorrectionType correctionType_;
};

std::ostream & operator <<(std::ostream & s,
                           const ScriptResultIdentifier & sri);
}

#endif /* ScriptResultIdentifier_H_ */
