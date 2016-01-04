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

#include "kvDataFlag.h"
#include <iostream>

namespace kvalobs {

kvDataFlag::kvDataFlag() {
  clear();
}

kvDataFlag::kvDataFlag(const unsigned char f[kvDataFlag::size]) {
  clear();
  for (int i = 0; i < kvDataFlag::size; i++)
    flag_[i] = f[i];
}

kvDataFlag::kvDataFlag(const std::string& s) {
  clear();
  if (s.length() >= kvDataFlag::size) {
    for (int i = 0; i < kvDataFlag::size; i++)
      flag_[i] = (unsigned char) (s[i]);
  }
}

kvDataFlag::kvDataFlag(const kvDataFlag& df) {
  clear();
  for (int i = 0; i < kvDataFlag::size; i++)
    flag_[i] = df.flag_[i];
}

kvDataFlag::~kvDataFlag() {
}

// clear flag
void kvDataFlag::clear() {
  for (int i = 0; i < kvDataFlag::size; i++)
    flag_[i] = '0';
}

// flag as string
std::string kvDataFlag::flagstring() const {
  std::string s;
  for (int i = 0; i < kvDataFlag::size; i++)
    s += flag_[i];
  return s;
}

// get one nibble as char
unsigned char kvDataFlag::cflag(const char index) const {
  if (index < 0 || index >= kvDataFlag::size)
    return '-';

  return flag_[index];
}

// get one nibble
int kvDataFlag::flag(const char index) const {
  if (index < 0 || index >= kvDataFlag::size)
    return 0;

  return chartoint_(flag_[index]);
}

// set one nibble
void kvDataFlag::set(const char index, const int c) {
  if (index < 0 || index >= kvDataFlag::size || !legal_(c))
    return;

  flag_[index] = inttochar_(c);
}

// Assignment operator
kvDataFlag& kvDataFlag::operator=(const kvDataFlag &rhs) {
  if (this == &rhs)
    return *this;

  // elementwise copy
  for (int i = 0; i < kvDataFlag::size; i++)
    flag_[i] = rhs.flag_[i];

  return *this;
}

// Equality operator
bool kvDataFlag::operator==(const kvDataFlag& rhs) const {
  for (int i = 0; i < kvDataFlag::size; i++)
    if (flag_[i] != rhs.flag_[i])
      return false;
  return true;
}

// ostream operator
std::ostream & operator <<(std::ostream& output,
                           const kvalobs::kvDataFlag& kd) {
  output << "[";
  for (int i = 0; i < kvDataFlag::size - 1; i++) {
    output << kd.cflag(i) << "|";
  }
  output << kd.cflag(kvDataFlag::size - 1) << "]";

  return output;
}

// check if legal int
bool kvDataFlag::legal_(const int i) {
  return (i >= 0 && i < 16);
}

// convert a HEX-character (0-9,A-F) to int
int kvDataFlag::chartoint_(const char c) const {
  const int zv = int('0');
  const int nv = int('9');
  const int av = int('A');
  const int fv = int('F');

  int v = int(c);

  if (v >= zv && v <= nv)
    return v - zv;
  else if (v >= av && v <= fv)
    return v - av + 10;

  // illegal character
  return 0;
}

// convert an int to a HEX-character
char kvDataFlag::inttochar_(const int i) const {
  const int zv = int('0');
  const int av = int('A');

  if (i >= 0 && i <= 9)
    return char(zv + i);
  else if (i >= 10 && i <= 15)
    return char(av + i - 10);

  // illegal int
  return '0';
}

}
