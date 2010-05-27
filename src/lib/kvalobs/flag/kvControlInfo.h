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

#ifndef KVCONTROLINFO_H_
#define KVCONTROLINFO_H_

#include "kvDataFlag.h"
#include <kvalobs/kvQCFlagTypes.h>
#include <kvalobs/kvQcxInfo.h>
#include <map>
#include <set>
#include <list>

namespace kvalobs
{

/**
 * \brief  ControlInfo DataFlag
 */
class kvControlInfo: public kvDataFlag
{
protected:
	/// controlpart --> c_flags map
	static std::map<int, kvQCFlagTypes::c_flags> lockedControlFlags_;
	/// main_qcx string --> main_qc enum map
	static std::map<std::string, kvQCFlagTypes::main_qc> mainQCXint_;

	/// list of QcxInfo (from db)
	static std::list<kvalobs::kvQcxInfo> qcxinfolist_;

	/// medium_qcx --> main_qc enum map
	static std::map<std::string, kvQCFlagTypes::main_qc> mainQCX_;
	/// medium_qcx --> controlpart map
	static std::map<std::string, int> controlPart_;

	/// list of flag values (by c_part) which trigger destruction of 'corrected'
	static std::map<int, std::set<int> > badValues_;

	/// init static structures etc.
	void init_();

public:
	/// flag is all zero's
	kvControlInfo();
	kvControlInfo(const std::string& s);
	kvControlInfo(const unsigned char f[kvDataFlag::size]);
	kvControlInfo(const kvControlInfo& df);
	kvControlInfo(const kvDataFlag& df);

	/**
	 * \brief Fill static structures from list of kvQcxInfo
	 *
	 * Should be called once in initialisation phase
	 */
	void setQcxInfo(const std::list<kvalobs::kvQcxInfo>& qcxi);

	/**
	 * \brief get controlflag - based on checktype.
	 * \return  false if unknown checktype
	 */
	bool getControlFlag(const std::string& medium_qcx, int& control);
	/**
	 * \brief set controlflag - based on checktype,
	 *
	 * optionally calls setFqclevel
	 * \return false if unknown checktype
	 */
	bool setControlFlag(const std::string& medium_qcx, const int& control,
			bool setfqcl = true);
	/**
	 * \brief force controlflag in specified part of controlinfo
	 *
	 * optionally calls setFqclevel
	 */
	bool setControlFlag(const kvQCFlagTypes::c_flags cf, const int& control,
			bool setfqcl = true);
	/**
	 * \brief set the FqcLevel flag in controlinfo dataflags
	 * (Nibble 0)
	 *
	 * Usually called by setControlFlag()
	 */
	void setFqclevel();
	void setFqclevel(const std::string& medium_qcx);

	bool qc1Done() const;
	bool qc2dDone() const;
	bool qc2mDone() const;
	bool qc2Done() const;
	bool hqcDone() const;

	/**
	 * \brief convenience-functions to set the missing-flag (QC1-0)
	 */
	void MissingFlag(const int& v);

	/**
	 * \brief convenience-functions to get the missing-flag (QC1-0)
	 */
	int MissingFlag() const;

	/**
	 * \brief check if value for one control part corresponds to a rejected 'corrected'
	 */
	bool iznogood(const std::string& medium_qcx) const;

	kvControlInfo& operator=(const kvControlInfo &rhs);
	kvControlInfo& operator=(const kvDataFlag &rhs);
	bool operator==(const kvControlInfo& rhs) const;
	bool operator!=(const kvControlInfo& rhs) const
	{
		return !(*this == rhs);
	}
	friend std::ostream& operator<<(std::ostream& output,
			const kvalobs::kvControlInfo& kd);
};

}

#endif /* KVCONTROLINFO_H_ */
