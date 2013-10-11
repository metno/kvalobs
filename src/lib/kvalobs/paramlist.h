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
 * \brief used as a cache element for an param cache. 
 */
class Param
{
	std::string kode_;
	int id_;

public:
	Param() :
			id_(-1)
	{
	}

	Param(const Param &p) :
			kode_(p.kode_), id_(p.id_)
	{
	}

	Param(const std::string &kode, int id) :
			kode_(kode), id_(id)
	{
	}

	Param& operator=(const Param &p)
	{
		if (&p == this)
			return *this;

		kode_ = p.kode_;
		id_ = p.id_;
		return *this;
	}

	bool valid() const
	{
		return (id_ >= 0);
	}

	int id() const
	{
		return id_;
	}
	std::string kode() const
	{
		return kode_;
	}
};

class ParamPredicate: public std::binary_function<Param, Param, bool>
{
public:
	result_type operator()(const first_argument_type &a1,
			const second_argument_type &a2) const
	{
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

std::ostream&
operator<<(std::ostream &os, const ParamList &p);

/** @} */

#endif
