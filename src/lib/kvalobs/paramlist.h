/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: paramlist.h,v 1.1.2.2 2007/09/27 09:02:30 paule Exp $

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
#ifndef __paramlist_h__
#define __paramlist_h__

#include <functional>
#include <string>
#include <set>
#include <ostream>

/**
 * \addtogroup kvinternalhelpers
 * @{
 */

/**
 * \brief Param is used as a cache element for an param cache.
 *
 */
class Param {
  ///The name the param is identified with in the param table in kvalobs
  std::string kode_;

  ///The paramid the param is identified with in the param table in kvalobs
  int id_;

  /**
   * Tells if the data for this parameter is to be loaded into
   * the data table (isScalar=true) or the text_data table (isScalar=false).
   */
  bool isScalar_;

 public:
  Param()
      : kode_(""),
        id_(-1),
        isScalar_(false) {
  }

  Param(const Param &p)
      : kode_(p.kode_),
        id_(p.id_),
        isScalar_(p.isScalar_) {
  }

  Param(const std::string &kode, int id, bool isScalar)
      : kode_(kode),
        id_(id),
        isScalar_(isScalar) {
  }

  Param& operator=(const Param &p) {
    if (&p == this)
      return *this;

    kode_ = p.kode_;
    id_ = p.id_;
    isScalar_ = p.isScalar_;
    return *this;
  }

  /**
   * The parametere is a valid parameter definition if
   * the paramid >= 0.
   *
   * \return true if this is a valid parameter definition and false otherwise.
   */
  bool valid() const {
    return (id_ >= 0);
  }

  /**
   * Return the paramid the parameter is has in kvalobs.
   * \return The paramid to the parameter.
   */
  int id() const {
    return id_;
  }

  /**
   * Return the param name the parameter is known by in kvalobs.
   * \return The name of the parameter.
   */
  std::string kode() const {
    return kode_;
  }

  /**
   * Tells if the data for this parameter is to be loaded into
   * the data table or the text_data table.
   *
   * @return true if the data is for the data table and false if the data
   * is for the text_data table.
   */
  bool isScalar() const {
    return isScalar_;
  }
};

class ParamPredicate : public std::binary_function<Param, Param, bool> {
 public:
  result_type operator()(const first_argument_type &a1,
                         const second_argument_type &a2) const {
    if (a1.kode() < a2.kode())
      return true;

    return false;
  }
};

///a type that can be used as a param cache.
typedef std::set<Param, ParamPredicate> ParamList;
///an iterator for ParamList
typedef std::set<Param, ParamPredicate>::iterator IParamList;
///a const iterator for ParamList
typedef std::set<Param, ParamPredicate>::const_iterator CIParamList;

/**
 * findParamIdInList, finds the name of the param given with id in
 * a ParamList.
 *
 * \param pl the ParamList to search.
 * \param id the id to search after in the ParamList, pl. 
 * \return the paramname on success and an empty string otherwise.
 */
std::string
findParamIdInList(const ParamList &pl, int id);

/**
 * findParamInList, finds the Param definition where name is equal to paramName
 * in the ParamList.
 *
 * \param pl the ParamList to search.
 * \param paramName the name to search after in the ParamList, pl.
 * \param[out] The param definition for the paramName, if defined in the param table.
 * \return true if paramName is defined and false of not.
 */
bool
findParamInList(const ParamList &pl, const std::string &paramName,
                Param &param);

/**
 * Reads the param definitions from a CSV file with the following format:
 * paramid,name,scalar
 *
 * Lines that starts with a the character '#' (comment) is ignored.
 *
 * The file can be generated from stinfosys with the command:
 * \copy (select paramid,name,scalar from param  order by paramid) to 'params.csv' delimiter as ',';
 *
 * \param filename The CSV file to read the parameters from.
 * \param[out] paramList The list that the parameters is saved to.
 * \return true if the method succeed to read the file and false otherwise.
 */
bool
readParamsFromFile(const std::string &filename, ParamList &paramList);

bool
isParamListsEqual(const ParamList &oldList, const ParamList &newList);

std::ostream&
operator<<(std::ostream &os, const ParamList &p);

/** @} */

#endif
