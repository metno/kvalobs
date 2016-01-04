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

#include <float.h>
#include "BufrDecodeBase.h"

using namespace std;

namespace kvalobs {
namespace decoder {
namespace bufr {

bool BufrDecodeBase::doDecode(int descriptor, BufrValue &value,
                              bool mustexist) {
  DescriptorFloatValue descVal;
  double val;

  if (!bufrMessage->nextFloatValueDescriptor(descVal)) {
    if (mustexist)
      throw BufrEndException();
    else
      return false;
  }

  if (descVal.descriptor != descriptor)
    throw BufrSequenceException(descriptor, descVal.descriptor);

  value.unit = descVal.unit;

  if (descVal.unit == "CCITTIA5") {
    value.type = BufrValue::String;

    if (descVal.value == RVIND)
      value.sValue = "";
    else
      value.sValue = bufrMessage->getStringValue(
          static_cast<int>(descVal.value));

    return true;
  }

  value.type = BufrValue::Float;

  if (descVal.value == RVIND)
    value.dValue = DBL_MAX;
  else
    value.dValue = descVal.value;

  return true;
}

/**
 * @exception BufrSequenceException, BufrException
 * @param descriptor Expected descriptor
 * @param value Set the value from the bufr.
 */
bool BufrDecodeBase::getDescriptor(int descriptor, double &value_,
                                   std::string &unit, bool mustexist) {
  BufrValue val;

  bool ret = doDecode(descriptor, val, mustexist);

  if (!ret)
    return ret;

  if (val.getType() != BufrValue::Float)
    throw BufrException(
        "Wrong type: expecting floating point value, got character value.");

  unit = val.unit;
  value_ = val.dValue;
  return true;
}

bool BufrDecodeBase::getDescriptor(int descriptor, std::string &value_,
                                   bool mustexist) {
  BufrValue val;

  bool ret = doDecode(descriptor, val, mustexist);

  if (!ret)
    return ret;

  if (val.type != BufrValue::String)
    throw BufrException(
        "Wrong type: expecting character value, got floating point value.");

  value_ = val.sValue;
  return true;

}

bool BufrDecodeBase::ignoreDescriptor(int descriptor, bool mustexist) {
  BufrValue val;

  bool ret = doDecode(descriptor, val, mustexist);

  if (!ret)
    return ret;

  return true;
}

#if 0
/**
 * @exception BufrSequenceException, BufrException
 * @param descriptor Expected descriptor.
 * @param kvparam kvalobs parameter id.
 * @param sensor kvalobs  sensor
 * @param level kvalobs level.
 */
bool
BufrDecodeBase::
getDescriptor( int descriptor, int kvparam, BufrDecodeResultBase *result, bool mustexist )
{
  double dval;
  float fval;
  string unit;

  if( ! getDescriptor( descriptor, dval, unit, mustexist ) )
  return false;

  fval = unitConvert->convert( kvparam, dval, unit );

  result->add( fval, kvparam, obstime );
  return true;
}
#endif

bool BufrDecodeBase::nextDescriptor(int &descriptor, BufrValue &value) {
  DescriptorFloatValue descVal;
  double val;

  if (!bufrMessage->nextFloatValueDescriptor(descVal))
    return false;

  descriptor = descVal.descriptor;
  value.unit = descVal.unit;

  if (descVal.unit == "CCITTIA5") {
    value.type = BufrValue::String;

    if (descVal.value == RVIND)
      value.sValue = "";
    else
      value.sValue = bufrMessage->getStringValue(
          static_cast<int>(descVal.value));

    return true;
  }

  value.type = BufrValue::Float;

  if (descVal.value == RVIND)
    value.dValue = DBL_MAX;
  else
    value.dValue = descVal.value;

  return true;
}

bool BufrDecodeBase::peekAt(int &descriptor, BufrValue &value, int index) {
  DescriptorFloatValue descVal;
  double val;

  if (!bufrMessage->peekAtFloatValueDescriptor(descVal, index))
    return false;

  descriptor = descVal.descriptor;
  value.unit = descVal.unit;

  if (descVal.unit == "CCITTIA5") {
    value.type = BufrValue::String;

    if (descVal.value == RVIND)
      value.sValue = "";
    else
      value.sValue = bufrMessage->getStringValue(
          static_cast<int>(descVal.value));

    return true;
  }

  value.type = BufrValue::Float;

  if (descVal.value == RVIND)
    value.dValue = DBL_MAX;
  else
    value.dValue = descVal.value;

  return true;
}

void BufrDecodeBase::decodeBufrMessage(BufrMessage *bufr,
                                       BufrDecodeResultBase *result_) {
  if (!bufr || !result_)
    throw BufrException("EXCEPTION: BuffrMessage. NULLPOINTER");

  bufrMessage = bufr;

  try {
    decode(result_);
    return;
  } catch (const BufrException &ex) {
    throw;
  }
}

}
}
}

