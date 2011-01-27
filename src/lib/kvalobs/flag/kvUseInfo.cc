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

#include "kvUseInfo.h"
#include "kvControlInfo.h"
#include <kvalobs/kvQCFlagTypes.h>

using namespace kvQCFlagTypes;

namespace kvalobs
{

kvUseInfo::kvUseInfo() :
	kvDataFlag()
{
	clear();
}

kvUseInfo::kvUseInfo(const unsigned char f[kvDataFlag::size]) :
	kvDataFlag(f)
{
}

kvUseInfo::kvUseInfo(const std::string& s) :
	kvDataFlag(s)
{
}

kvUseInfo::kvUseInfo(const kvUseInfo& df)
{
	clear();
	for (int i = 0; i < kvDataFlag::size; i++)
		flag_[i] = df.flag_[i];
}

kvUseInfo::kvUseInfo(const kvDataFlag& df) :
	kvDataFlag(df)
{
}

// clear flag
void kvUseInfo::clear()
{
	for (int i = 0; i < kvDataFlag::size; i++)
		flag_[i] = (i < 5 ? '9' : '0');
}

// Assignment operator
kvUseInfo& kvUseInfo::operator=(const kvUseInfo &rhs)
{
	if (this == &rhs)
		return *this;

	// elementwise copy
	for (int i = 0; i < kvDataFlag::size; i++)
		flag_[i] = rhs.flag_[i];

	return *this;
}

kvUseInfo& kvUseInfo::operator=(const kvDataFlag &rhs)
{
	// elementwise copy
	for (int i = 0; i < kvDataFlag::size; i++)
		set(i, rhs.flag(i));

	return *this;
}

// Equality operator
bool kvUseInfo::operator==(const kvUseInfo& rhs) const
{
	for (int i = 0; i < kvControlInfo::size; i++)
		if (flag_[i] != rhs.flag_[i])
			return false;
	return true;
}

// ostream operator
std::ostream & operator <<(std::ostream& output, const kvalobs::kvUseInfo& kd)
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
 set Useinfo-flags, based on values in a controlflag.
 */
bool kvUseInfo::setUseFlags(const kvControlInfo& cinfo)
{
	int ui[kvDataFlag::size];

	for (int i = 0; i < kvDataFlag::size; i++)
		ui[i] = flag(i);

	const int fagg = cinfo.flag(f_fagg);
	const int fr = cinfo.flag(f_fr);
	const int fcc = cinfo.flag(f_fcc);
	const int fs = cinfo.flag(f_fs);
	const int fnum = cinfo.flag(f_fnum);
	const int fpos = cinfo.flag(f_fpos);
	const int fmis = cinfo.flag(f_fmis);
	const int ftime = cinfo.flag(f_ftime);
	const int fw = cinfo.flag(f_fw);
	const int fstat = cinfo.flag(f_fstat);
	const int fcp = cinfo.flag(f_fcp);
	const int fclim = cinfo.flag(f_fclim);
	const int fd = cinfo.flag(f_fd);
	const int fpre = cinfo.flag(f_fpre);
	const int fcombi = cinfo.flag(f_fcombi);
	const int fhqc = cinfo.flag(f_fhqc);

	// useinfo(0) : "Kontrollniv� passert"
	if (cinfo.qc1Done() and cinfo.qc2Done() and cinfo.hqcDone())
		ui[0] = 1;
	else if (!cinfo.qc1Done() and cinfo.qc2Done() and cinfo.hqcDone())
		ui[0] = 2;
	else if (cinfo.qc1Done() and !cinfo.qc2Done() and cinfo.hqcDone())
		ui[0] = 3;
	else if (!cinfo.qc1Done() and !cinfo.qc2Done() and cinfo.hqcDone())
		ui[0] = 4;
	else if (cinfo.qc1Done() and cinfo.qc2Done() and !cinfo.hqcDone())
		ui[0] = 5;
	else if (!cinfo.qc1Done() and cinfo.qc2Done() and !cinfo.hqcDone())
		ui[0] = 6;
	else if (cinfo.qc1Done() and !cinfo.qc2Done() and !cinfo.hqcDone())
		ui[0] = 7;
	else
		ui[0] = 9;

	// useinfo(1) : "Avvik fra normert observasjon"
	// NB: After useinfo[6] and useinfo[7]
	ui[1] = 9;
	//if ( cinfo.qc1Done() )
	{
		if (fmis == 1 || fmis == 3)
			ui[1] = 8;

		else if ((fd == 0 || fd == 1) && ui[7] == 0)
			ui[1] = 0;

		else if ((fd == 0 || fd == 1) && ui[7] > 0)
			ui[1] = 1;

		else if (fd == 3 && ui[7] == 0)
			ui[1] = 2;

		else if ((fd == 2 || fd == 6 || fd == 7 || fd == 0xA || fd == 0xB)
				&& ui[7] == 0)
			ui[1] = 3;

		else if (fd == 3 && ui[7] > 0)
			ui[1] = 4;

		else if ((fd == 2 || fd == 6 || fd == 7 || fd == 0xA || fd == 0xB)
				&& ui[7] > 0)
			ui[1] = 5;
	}

	// useinfo(2) : "Kvalitetsniv� for originalverdi"
	ui[2] = 9;
	//if ( cinfo.qc1Done() or cinfo.qc2Done() or cinfo.hqcDone() )
	{
		if (fmis == 1 || fmis == 3)
			ui[2] = 9;

		else if (fhqc == 1 || fhqc == 2)
			ui[2] = 0;

		else if (fagg > 4 || fr == 6 || fr == 0xA || fcc >= 0xA || fcp >= 0xA || fs >= 8
				|| fnum == 6 || fw == 6 || fpos >= 4 || fd == 2 || fd >= 6
				|| fpre >= 4 || fclim == 3 || fcombi >= 9 || fhqc >= 6)
			ui[2] = 3;

		else if (fagg == 3 || ((fr == 4 || fr == 5) && fcombi != 2 && fcombi
				!= 1) || fcc == 3 || fcc == 4 || fcc == 6 || fcc == 7 || fcp
				== 3 || fcp == 4 || fcp == 6 || fcp == 7 || fs == 3 || fw == 4
				|| fw == 5 || fpos == 3 || fstat == 2 || fd == 3)
			ui[2] = 2;

		else if (fagg == 2 || fr == 2 || fr == 3 || fcc == 2 || fcp == 2 || fs
				== 2 || fs == 4 || fs == 5 || fs == 7 || fw == 2 || fw == 3
				|| fclim == 2 || fcombi == 2)
			ui[2] = 1;

		else if (fagg == 1 || fr == 1 || fcc == 1 || fcp == 1 || fs == 1 || fw
				== 1 || fpos == 1 || fstat == 1 || fclim == 1 || fd == 1
				|| fcombi == 1)
			ui[2] = 0;
	}

	// useinfo(3) : "Originalverdi korrigert"
	ui[3] = 9;
	//if ( cinfo.qc1Done() or cinfo.qc2Done() or cinfo.hqcDone() )
	{
		if (fmis == 3)
			ui[3] = 9;

		else if (fhqc == 6 || fagg == 8)
			ui[3] = 5;

		else if (fd > 5 || fagg == 9)
			ui[3] = 6;

		else if (fhqc == 5 || fagg == 5 || (ftime == 1 && fmis == 1))
			ui[3] = 2;

		else if (fhqc == 7 || fagg == 4 || fr == 0xA || (ftime == 1 && fmis == 4))
			ui[3] = 1;

		else if (fhqc == 1 || fhqc == 2)
			ui[3] = 0;

		else if (fmis == 2 )
			ui[3] = 8;

		else if (fmis == 1)
			ui[3] = 4;

		else if (fagg == 6 || fcc == 0xA || fcc == 0xB || fcp == 0xA || fcp
				== 0xB || fs == 9 || fs == 0xA || fpos == 4 || fpre == 4
				|| fclim == 3 || ((fnum == 6 || fw == 6 || ftime == 2) && fmis == 4))
			ui[3] = 3;

		else
			ui[3] = 0;
	}

	// useinfo(4) : "Viktigste kontrollmetode"
	ui[4] = 9;
	// NB: After useinfo[2]
	//if ( cinfo.qc1Done() or cinfo.qc2Done() or cinfo.hqcDone() )
	{
		if (ui[2] == 0)
			ui[4] = 0;

		else if (fhqc >= 5)
			ui[4] = 9;

		else if (fd == 2 || fd >= 6 || fr == 7)
			ui[4] = 9;

		else if ((fr == 0xA || (7 > fr && fr > 1)) && fcc <= 1 && fcp <= 1 && fs <= 1 && fnum
				<= 5 && fpos <= 1 && ftime <= 1 && fw <= 1 && fstat <= 1
				&& fclim <= 1)
			ui[4] = 1;

		else if ((fcc == 2 || fcc == 3 || fcc == 6 || fcc == 9 || fcc == 0xA
				|| fcc == 0xD || fcp == 2 || fcp == 3 || fcp == 0xA) && fs <= 1
				&& fnum <= 5 && fpos <= 1 && ftime <= 1 && fw <= 1 && fstat
				<= 1 && fclim <= 1)
			ui[4] = 2;

		else if ((fs > 1 || fpos > 1) && fnum <= 5 && ftime <= 1 && fw <= 1
				&& fstat <= 1 && fclim <= 1)
			ui[4] = 3;

		else if ((fcc == 4 || fcc == 7 || fcc == 0xB || fcp == 4 || fcp == 7
				|| fcp == 0xB) && fnum <= 5 && ftime <= 1 && fw <= 1 && fstat
				<= 1 && fclim <= 1)
			ui[4] = 4;

		else if ((fw > 1 || fclim > 1) && fnum <= 5 && ftime <= 1 && fstat <= 1)
			ui[4] = 5;

		else if (ftime > 1 && fnum <= 5 && fstat <= 1)
			ui[4] = 6;

		else if (fnum > 5 && fstat <= 1)
			ui[4] = 7;

		else if (fstat > 1)
			ui[4] = 8;
	}

	// useinfo( 6) : "observasjonstid i forhold til normert tidsintervall"
	// useinfo( 7) : "Forsinkelse"
	// useinfo( 8) : "Konfidens"
	// useinfo( 9) : "----"----"
	// useinfo(13) : "HQC-operat�rens identifikator"
	// useinfo(14) : "-------------"---------------"


	// useinfo(15) : "Tester som har gitt utslag (cfailed)"
	ui[15] = 0;
	if (fhqc > 0)
		ui[15]++;
	if (fr > 1)
		ui[15]++;
	if (fcc > 1)
		ui[15]++;
	if (fcp > 1)
		ui[15]++;
	if (fs > 1)
		ui[15]++;
	if (fnum > 1)
		ui[15]++;
	if (fpos > 1)
		ui[15]++;
	if (ftime > 1)
		ui[15]++;
	if (fw > 1)
		ui[15]++;
	if (fstat > 1)
		ui[15]++;
	if (fclim > 1)
		ui[15]++;
	if (fpre > 1)
		ui[15]++;

	// Finally- set the useflags
	for (int i = 0; i < kvDataFlag::size; i++)
		set(i, ui[i]);

	return true;
}

/*
 add 1 to error-count
 */
void kvUseInfo::addToErrorCount()
{
	int c = flag(15) + 1;
	if (c > 15)
		c = 15;
	set(15, c);
}

/*
 error count
 */
int kvUseInfo::ErrorCount() const
{
	return flag(15);
}

/*
 Useinfo: set confidence (0-100)
 */
void kvUseInfo::Confidence(const int& c)
{
	if (c < 0 || c > 100)
	{
		CERR("kvUseInfo::Confidence ERROR setting illegal confidence:"
				<< c << std::endl);
		return;
	}
	int i1 = c / 16;
	int i2 = c % 16;

	// confidence lies in useflag, nibbles 8 and 9
	set(8, i1);
	set(9, i2);
}

/*
 Useinfo: get confidence (0-100)
 */
int kvUseInfo::Confidence() const
{
	// confidence lies in useflag, nibbles 8 and 9
	int i1 = flag(8);
	int i2 = flag(9);

	return i1 * 16 + i2;
}

/*
 Useinfo: set HQC observer-id (0 - 255)
 */
void kvUseInfo::HQCid(const int& c)
{
	if (c < 0 || c > 255)
	{
		CERR("kvUseInfo::HQCid ERROR setting illegal id:"
				<< c << std::endl);
		return;
	}
	int i1 = c / 16;
	int i2 = c % 16;

	// HQCid lies in useflag, nibbles 13 and 14
	set(13, i1);
	set(14, i2);
}

/*
 Useinfo: get HQC observer-id
 */
int kvUseInfo::HQCid() const
{
	// HQCid lies in useflag, nibbles 13 and 14
	int i1 = flag(13);
	int i2 = flag(14);

	return i1 * 16 + i2;
}

}
