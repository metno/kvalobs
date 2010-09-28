/*
 * BufrDecodeBase.h
 *
 *  Created on: Sep 26, 2010
 *      Author: borgem
 */

#include "BufrDecodeBase.h"

using namespace std;

namespace {
   using namespace kvalobs::decoder::bufr;
   typedef enum { String, Float } ValueType;
   struct Value {
      string unit;
      ValueType type;
      double d;
      std::string s;
   };

   bool doDecode( BufrMessage *bufr, int descriptor, Value &value, bool mustexist ) {
      DescriptorFloatValue descVal;
      double val;

      if( !bufr->nextFloatValueDescriptor( descVal ) ) {
          if( mustexist )
             throw BufrEndException();
          else
             return false;
      }

      if( descVal.descriptor != descriptor )
         throw BufrSequenceException( descriptor, descVal.descriptor );

      if( descVal.value == RVIND )
          return true;

      value.unit = descVal.unit;

      if( descVal.unit == "CCITTIA5" ) {
         value.type = String;
         value.s = bufr->getStringValue( static_cast<int>( descVal.value ) );
         return true;
      }

      value.type = Float;
      value.d = descVal.value;
      return true;
   }


}


namespace kvalobs {
namespace decoder {
namespace bufr {

void
BufrDecodeResult::
add( float value, int kvparamid, int sensor, int level )
{
   if( stationid == INT_MAX )
      return;

   if( obstime.undef() )
      return;

   data.push_back( kvalobs::kvData( stationid, obstime, value, kvparamid, tbtime, typeid_, sensor, level, value,
                                    kvalobs::kvControlInfo(), kvalobs::kvUseInfo(), "" ) );
}

/**
* @exception BufrSequenceException, BufrException
* @param descriptor Expected descriptor
* @param value Set the value from the bufr.
*/
bool
BufrDecodeBase::
decode( int descriptor,  double &value_, std::string &unit, bool mustexist )
{
   Value val;

   bool ret = doDecode( bufrMessage, descriptor, val, mustexist );

   if( !ret )
      return ret;

   if( val.type != Float )
      throw BufrException("Wrong type: expecting floating point value, got character value.");

   unit = val.unit;
   value_ = val.d;
   return true;
}

bool
BufrDecodeBase::
decode( int descriptor,  std::string &value_, bool mustexist )
{
   Value val;

   bool ret = doDecode( bufrMessage, descriptor, val, mustexist );

   if( !ret )
      return ret;

   if( val.type != String )
      throw BufrException("Wrong type: expecting character value, got floating point value.");

   value_ = val.s;
   return true;

}


/**
* @exception BufrSequenceException, BufrException
* @param descriptor Expected descriptor.
* @param kvparam kvalobs parameter id.
* @param sensor kvalobs  sensor
* @param level kvalobs level.
*/
bool
BufrDecodeBase::
decode( int descriptor, int kvparam, int sensor, int level, bool mustexist )
{
   double dval;
   float fval;
   string unit;

   if( ! decode( descriptor, dval, unit, mustexist ) )
      return false;

   fval = unitConvert->convert( kvparam, dval, unit );

   result->add( fval, kvparam, sensor, level );
   return true;
}

BufrDecodeResult*
BufrDecodeBase::
decode( BufrMessage *bufr )
{
   if( ! bufr )
      throw BufrException("EXCEPTION: BuffrMessage. NULLPOINTER");

   bufrMessage = bufr;

   try {
      result = new BufrDecodeResult();
   }
   catch( ... ) {
      throw BufrException("EXCEPTION: NO MEM to allocate BufrDecodeResult.");
   }

   try {
      decode();
      return result;
   }
   catch( const BufrException &ex ) {
      delete result;
      throw;
   }
}

}
}
}

