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

#include "kvControlInfo.h"

using namespace kvQCFlagTypes;

namespace kvalobs
{

// controlpart --> c_flags map
std::map<int, kvQCFlagTypes::c_flags> kvControlInfo::lockedControlFlags_;
// main_qcx string --> main_qc enum map
std::map<std::string, kvQCFlagTypes::main_qc> kvControlInfo::mainQCXint_;

// list of QcxInfo (from db)
std::list<kvQcxInfo> kvControlInfo::qcxinfolist_;

// medium_qcx --> main_qc enum map
std::map<std::string, kvQCFlagTypes::main_qc> kvControlInfo::mainQCX_;
// medium_qcx --> controlpart map
std::map<std::string, int> kvControlInfo::controlPart_;

kvControlInfo::kvControlInfo() :
	kvDataFlag()
{
	init_();
}

kvControlInfo::kvControlInfo(const unsigned char f[kvDataFlag::size]) :
	kvDataFlag(f)
{
	init_();
}

kvControlInfo::kvControlInfo(const std::string& s) :
	kvDataFlag(s)
{
	init_();
}

kvControlInfo::kvControlInfo(const kvControlInfo& df)
{
	init_();
	for (int i = 0; i < kvDataFlag::size; i++)
		flag_[i] = df.flag_[i];
}

kvControlInfo::kvControlInfo(const kvDataFlag& df) :
	kvDataFlag(df)
{
	init_();
}

/*
 Fill static structures from list of kvQcxInfo
 Should be called once in initialisation phase
 */
void kvControlInfo::setQcxInfo(const std::list<kvalobs::kvQcxInfo>& qcxi)
{
	qcxinfolist_ = qcxi;

	// reset static structures
	mainQCX_.clear();
	controlPart_.clear();

	std::list<kvalobs::kvQcxInfo>::const_iterator pqi = qcxinfolist_.begin();

	// Loading qcxinfo

	for (; pqi != qcxinfolist_.end(); pqi++)
	{

		std::string medium_qcx = pqi->medium_qcx();
		std::string main_qcx = pqi->main_qcx();

		if (mainQCXint_.count(main_qcx) == 0)
		{
			std::clog << "unknown main_qcx:" << main_qcx << ":" << std::endl;

		}
		else
		{
			// combining medium_qcx with main_qcx
			mainQCX_[medium_qcx] = mainQCXint_[main_qcx];

		}

		// combining medium_qcx with controlpart
		controlPart_[medium_qcx] = pqi->controlpart();
	}

}

// common init
void kvControlInfo::init_()
{
	// controlpart --> c_flags map
	// these parts of the controlflag locked
	if (lockedControlFlags_.size() == 0)
	{
		lockedControlFlags_[f_fr] = f_fr;
		lockedControlFlags_[f_fcc] = f_fcc;
		lockedControlFlags_[f_fs] = f_fs;
		lockedControlFlags_[f_fnum] = f_fnum;
		lockedControlFlags_[f_fpos] = f_fpos;
		lockedControlFlags_[f_fmis] = f_fmis;
		lockedControlFlags_[f_ftime] = f_ftime;
		lockedControlFlags_[f_fw] = f_fw;
		lockedControlFlags_[f_fstat] = f_fstat;
		lockedControlFlags_[f_fcp] = f_fcp;
		lockedControlFlags_[f_fclim] = f_fclim;
		lockedControlFlags_[f_fd] = f_fd;
		lockedControlFlags_[f_fpre] = f_fpre;
		lockedControlFlags_[f_fcombi] = f_fcombi;
		lockedControlFlags_[f_fhqc] = f_fhqc;
	}

	// main_qcx string --> main_qc enum map
	if (mainQCXint_.size() == 0)
	{
		mainQCXint_["QC1"] = main_qc1;
		mainQCXint_["QC2d"] = main_qc2d;
		mainQCXint_["QC2m"] = main_qc2m;
		mainQCXint_["HQC"] = main_hqc;
	}
}

// Assignment operator
kvControlInfo& kvControlInfo::operator=(const kvControlInfo &rhs)
{
	if (this == &rhs)
		return *this;

	// elementwise copy
	for (int i = 0; i < kvDataFlag::size; i++)
		flag_[i] = rhs.flag_[i];

	return *this;
}

kvControlInfo& kvControlInfo::operator=(const kvDataFlag &rhs)
{
	// elementwise copy
	for (int i = 0; i < kvDataFlag::size; i++)
		set(i, rhs.flag(i));

	return *this;
}

// Equality operator
bool kvControlInfo::operator==(const kvControlInfo& rhs) const
{
	for (int i = 0; i < kvControlInfo::size; i++)
		if (flag_[i] != rhs.flag_[i])
			return false;
	return true;
}

// ostream operator
std::ostream & operator <<(std::ostream& output,
		const kvalobs::kvControlInfo& kd)
{
	output << "[";
	for (int i = 0; i < kvDataFlag::size - 1; i++)
	{
		output << kd.cflag(i) << "|";
	}
	output << kd.cflag(kvDataFlag::size - 1) << "]";

	return output;
}

/*
 Controlinfo-flag routine.
 get controlflag in controlinfo - based on checktype
 return false if unknown checktype
 */
bool kvControlInfo::getControlFlag(const std::string& medium_qcx, int& control)
{
	if (controlPart_.count(medium_qcx) > 0)
	{
		control = flag(controlPart_[medium_qcx]);
		return true;
	}
	std::clog << "unknown medium_qcx:" << medium_qcx << std::endl;
	return false;
}

/*
 Controlinfo-flag routine.
 set controlflag in controlinfo - based on checktype,
 calls setFqclevel!
 return false if unknown checktype
 */
bool kvControlInfo::setControlFlag(const std::string& medium_qcx,
		const int& control)
{
	if (controlPart_.count(medium_qcx) > 0)
	{
		int nibble = controlPart_[medium_qcx];
		set(nibble, control);
		return true;
	}
	std::clog << "unknown medium_qcx:" << medium_qcx << std::endl;
	return false;
}

/*
 Controlinfo-flag routine.
 force controlflag in specified part of controlinfo
 */
bool kvControlInfo::setControlFlag(const kvQCFlagTypes::c_flags cf,
		const int& control)
{
	int nibble = static_cast<int> (cf);
	set(nibble, control);

	return true;
}

bool kvControlInfo::qc1Done() const
{
	return flag(f_fr) or flag(f_fcc) or flag(f_fs) or flag(f_fnum) or flag(
			f_fpos) or flag(f_fcp) or flag(f_fd) or flag(f_fpre) or flag(
			f_fcombi);
}

bool kvControlInfo::qc2dDone() const
{
        return (flag( f_fs ) > 3) or flag( f_ftime ) or flag( f_fw ) or flag( f_fstat );
}

bool kvControlInfo::qc2mDone() const
{
        return flag( f_fclim ) or (flag( f_fd ) > 6);
}

bool kvControlInfo::qc2Done() const
{
	return qc2dDone() or qc2mDone();
}

bool kvControlInfo::hqcDone() const
{
	return flag(f_fhqc);
}

/*
 convenience-functions to set/get the missing-flag (QC1-0)
 */
void kvControlInfo::MissingFlag(const int& v)
{
	if (v >= kvQCFlagTypes::status_ok && v
			<= kvQCFlagTypes::status_orig_and_corr_missing)
	{
		set(f_fmis, v);

	}
	else
		std::clog << "illegal missing-value:" << v << std::endl;
}

int kvControlInfo::MissingFlag() const
{
	return flag(f_fmis);
}

}
