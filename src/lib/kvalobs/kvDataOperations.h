/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: kvDataOperations.h,v 1.1.2.2 2007/09/27 09:02:29 paule Exp $

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
#ifndef KVDATAOPERATIONS_
#define KVDATAOPERATIONS_

#include "kvalobs/kvData.h"
#include <functional>

namespace kvalobs {

class kvStationInfo;

/**
 * \brief Have we got a legal corrected value?
 */
bool valid(const kvData & d);

/**
 * \brief Has the original value been modified in any way?
 */
bool corrected(const kvData & d);

/**
 * \brief Does parameter contain data in either original or corrected?
 */
bool missing(const kvData & d);

/**
 * \brief Is the original value missing?
 */
bool original_missing(const kvData & d);

/**
 * \brief True if there was an original value and it has been rejected
 */
bool rejected(const kvData & d);

/**
 * \brief Is corrected value a model value?
 */
bool has_model_value(const kvData & d);

/**
 * \brief Reject value, leaving corrected as missing.
 *
 * Has no effect is original already was missing.
 */
void reject(kvData & d);

/**
 * \brief Update corrected field.
 */
void correct(kvData & d, float new_val);

/**
 * \brief Generate a missing data object for the given parameters.
 */
kvData getMissingKvData(int stationID, const boost::posix_time::ptime & obsTime,
                        int paramID, int typeID, int sensor, int level);

/**
 * Creates kvData objects, beloning to a specific "observation", consisting
 * of stationid, observation time and typeid, along with sensor and level.
 * Created objects will have all these parameters, but with the option to
 * respecify observation time when creating an object. This is to support
 * creation of type 302-type observation, which has three different obstimes
 * in the same observation.
 */
class kvDataFactory {
 public:
  kvDataFactory(int stationID, const boost::posix_time::ptime & obsTime,
                int typeID, int sensor = 0, int level = 0);

  /**
   * Create a factory for creating observation data logically beloning to
   * the given input.
   */
  explicit kvDataFactory(const kvData & data);

  explicit kvDataFactory(kvalobs::kvStationInfo & stInfo);

  /**
   * Get a missing parameter for the given observation.
   */
  kvData getMissing(int paramID, const boost::posix_time::ptime & obsTime =
                        boost::posix_time::ptime()) const;

  /**
   * Get one piece of data for the observation.
   */
  kvData getData(float val, int paramID,
                 const boost::posix_time::ptime & obsTime =
                     boost::posix_time::ptime()) const;

  int stationID() const {
    return stationID_;
  }
  int typeID() const {
    return typeID_;
  }
  const boost::posix_time::ptime & obstime() const {
    return obstime_;
  }
  int sensor() const {
    return sensor_;
  }
  int level() const {
    return level_;
  }

 private:
  const int stationID_;
  const int typeID_;
  const boost::posix_time::ptime obstime_;
  const int sensor_;
  const int level_;
};

/**
 * HQC-specific operations
 */
namespace hqc {
void hqc_accept(kvData & d);
void hqc_reject(kvData & d);

void hqc_correct(kvData & d, float new_val);
void hqc_interpol(kvData & d, float new_val);
void hqc_distribute(kvData & d, float new_val);

/**
 * \brief Correction method will be chosen based on the state of d
 * and value of new_val.
 */
void hqc_auto_correct(kvData & d, float new_val);

/**
 * \brief Has HQC set any flags on this?
 */
bool hqc_touched(const kvData & d);
bool hqc_corrected(const kvData & d);
bool hqc_accepted(const kvData & d);
bool hqc_rejected(const kvData & d);
}

/**
 * Various functors for inspecting kvData objects
 */
namespace inspect {
class has_paramid {
  int paramid_;
 public:
  has_paramid(int paramid)
      : paramid_(paramid) {
  }
  template<typename T>
  bool operator()(const T & t) const {
    return t.paramID() == paramid_;
  }
};
}

namespace compare {
typedef std::binary_function<kvData, kvData, bool> kvDataCompare;

bool eq_sensor(int sA, int sB);
bool lt_sensor(int sA, int sB);

struct lt_kvData : public kvDataCompare {
  bool operator()(const kvData & a, const kvData & b) const;
  bool operator()(const kvData * a, const kvData * b) const;
};

struct same_kvData : public kvDataCompare {
  bool operator()(const kvData & a, const kvData & b) const;
};

template<typename Kv1 = kvData, typename Kv2 = kvData>
struct lt_kvData_without_paramID : public std::binary_function<Kv1, Kv2, bool> {
  bool operator()(const Kv1 & a, const Kv2 & b) const {
    if (a.stationID() != b.stationID())
      return a.stationID() < b.stationID();
    if (a.obstime() != b.obstime())
      return a.obstime() < b.obstime();
    if (a.typeID() != b.typeID())
      return a.typeID() < b.typeID();
    if (a.level() != b.level())
      return a.level() < b.level();
    return lt_sensor(a.sensor(), b.sensor());
  }
};

struct same_obs_and_parameter : public kvDataCompare {
  bool operator()(const kvData & a, const kvData & b) const;
};

struct exactly_equal_ex_tbtime : public kvDataCompare {
  bool operator()(const kvData & a, const kvData & b) const;
};

struct exactly_equal : public kvDataCompare {
  bool operator()(const kvData & a, const kvData & b) const;
};

}

/**
 * Flag enums for use with kvData objects
 */
namespace flag {
enum ControlFlag {
  fqclevel,  // Kontrollniv�
  fr,  // Grenseverdikontroll
  fcc,  // Formell konsistenskontroll
  fs,  // Sprangkontroll
  fnum,  // Prognostisk romkontroll
  fpos,  // Meldingskontroll
  fmis,  // Manglende observasjon
  ftime,  // Tidsserietilpasning
  fw,  // V�ranalyse
  fstat,  // Statistikkontroll
  fcp,  // Klimatologisk konsistenskontroll
  fclim,  // Klimatologikontroll
  fd,  // Fordeling av samleverdier
  fpre,  // Forh�ndskvalifisering
  fcombi,  // Kombinert vurdering
  fhqc  // Manuell kvalitetskontroll
};

enum UseInfoFlag {
  ui0,  // Kontrollniv� passert
  ui1,  // Originalverdiens avvik fra normert observasjonsprosedyre
  ui2,  // Kvalitetsniv� for originalverdi
  ui3,  // Originalverdi korrigert
  ui4,  // Viktigste kontrollmetode
  ui5,  // < reservert >
  ui6,  // < reservert >
  ui7,  // Forsinkelse
  ui8,  // Prosent konfidens, f�rste siffer av to
  ui9,  // Prosent konfidens, andre siffer av to
  ui10,  // < reservert >
  ui11,  // < reservert >
  ui12,  // < reservert >
  ui13,  // HQC-operat�rens l�penummer, f�rste siffer av to
  ui14,  // HQC-operat�rens l�penummer, andre siffer av to
  ui15  // Antall tester som har gitt utslag
};
}
}

#endif // KVDATAOPERATIONS_
