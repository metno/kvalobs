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

#ifndef KVDATAFLAG_H_
#define KVDATAFLAG_H_

#include <string>
#include <iosfwd>

namespace kvalobs
{

/**
 * \addtogroup dbutility
 * @{
 */

/**
 * \brief a base class for kvControlInfo and kvUseInfo.
 *
 * 64 bit data flag implemented as 16 characters
 * Each char represents a hex-value: [0 - F]
 *
 * includes subclasses for kvalobs-specific ControlInfo
 * and UseInfo flags.
 *
 */
class kvDataFlag
{
public:
	enum
	{
		size = 16
	}; /// number of 4-bit nibbles
protected:
	/// the actual dataflag
	unsigned char flag_[kvDataFlag::size];

	/// check if legal int
	bool legal_(const int i);
	/// convert a HEX-character (0-9,A-F) to int
	int chartoint_(const char c) const;
	/// convert an int to a HEX-character
	char inttochar_(const int i) const;

public:
	/// flag is all zero's
	kvDataFlag();
	kvDataFlag(const std::string& s);
	kvDataFlag(const unsigned char f[kvDataFlag::size]);
	kvDataFlag(const kvDataFlag& df);

	virtual ~kvDataFlag();

	/// flag as string
	std::string flagstring() const;
	/// get one nibble as char
	unsigned char cflag(const char index) const;
	/// get one nibble
	int flag(const char index) const;
	/// set one nibble
	void set(const char index, const int c);
	/// clear flag
	virtual void clear();

	kvDataFlag& operator=(const kvDataFlag &rhs);
	bool operator==(const kvDataFlag& rhs) const;
	bool operator!=(const kvDataFlag& rhs) const
	{
		return !(*this == rhs);
	}
	friend std::ostream& operator<<(std::ostream& output,
			const kvalobs::kvDataFlag& kd);
};

}

#endif /* KVDATAFLAG_H_ */
