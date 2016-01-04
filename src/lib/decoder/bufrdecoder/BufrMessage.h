/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: comobsentry.cc,v 1.1.2.1 2007/09/27 09:02:24 paule Exp $

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

#ifndef __BUFRMESSAGE_H__
#define __BUFRMESSAGE_H__

#include <iostream>
#include <string>
#include "bufrdefs.h"

namespace kvalobs {
namespace decoder {
namespace bufr {

struct DescriptorFloatValue {
  int descriptor;
  int index;
  int replication;
  double value;
  std::string unit;
  std::string name;

  friend std::ostream& operator<<(std::ostream &out,
                                  const DescriptorFloatValue &v);
};

class BufrMessage {
  bool valid;
  long int ksup[9];
  long int ksec0[3];
  long int ksec1[40];
  long int ksec2[4096];
  long int ksec3[4];
  long int ksec4[2];
  long int key[46];
  char cnames[KELEM][64];
  char cunits[KELEM][24];
  double vals[KVALS];
  char cvals[KVALS][80];
  long int kdtlen;
  long int kdtlist[KELEM];
  long int kdtexplen;
  long int kdtexplist[KELEM];
  long int kelem;
  long int kvals;
  int valueIndex;

 public:
  std::string error;

  BufrMessage()
      : valid(false),
        kdtlen( KELEM),
        kdtexplen( KELEM),
        kelem( KELEM),
        kvals( KELEM) {
  }

  static bool bufrExpand(const std::string &bufr, BufrMessage &bufrMessage);

  DescriptorFloatValue* nextFloatValueDescriptor(
      DescriptorFloatValue &descriptor);
  DescriptorFloatValue* peekAtFloatValueDescriptor(
      DescriptorFloatValue &descriptor, int index = 1);

  std::string getStringValue(int charDescriptorValue);

  int descriptorTbl() const;

  friend std::ostream& operator<<(
      std::ostream &out, const kvalobs::decoder::bufr::BufrMessage &msg);
};

std::ostream& operator<<(std::ostream &out,
                         const kvalobs::decoder::bufr::BufrMessage &msg);
std::ostream& operator<<(std::ostream &out, const DescriptorFloatValue &v);

}
}
}

//std::ostream& operator<<( std::ostream &out, const kvalobs::decoder::bufr::BufrMessage &msg );
#endif /* BUFRMESSAGE_H_ */
