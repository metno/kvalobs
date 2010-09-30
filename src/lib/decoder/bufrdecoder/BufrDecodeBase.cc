/*
 * BufrDecodeBase.h
 *
 *  Created on: Sep 26, 2010
 *      Author: borgem
 */

#include <float.h>
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


      value.unit = descVal.unit;

      if( descVal.unit == "CCITTIA5" ) {
         value.type = String;

         if( descVal.value == RVIND )
            value.s = "";
         else
            value.s = bufr->getStringValue( static_cast<int>( descVal.value ) );

         return true;
      }

      value.type = Float;

      if( descVal.value == RVIND )
         value.d = DBL_MAX;
      else
         value.d = descVal.value;

      return true;
   }


}


namespace kvalobs {
namespace decoder {
namespace bufr {

/**
* @exception BufrSequenceException, BufrException
* @param descriptor Expected descriptor
* @param value Set the value from the bufr.
*/
bool
BufrDecodeBase::
getDescriptor( int descriptor,  double &value_, std::string &unit, bool mustexist )
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
getDescriptor( int descriptor,  std::string &value_, bool mustexist )
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

bool
BufrDecodeBase::
ignoreDescriptor( int descriptor, bool mustexist )
{
   Value val;

   bool ret = doDecode( bufrMessage, descriptor, val, mustexist );

   if( !ret )
      return ret;

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
getDescriptor( int descriptor, int kvparam,  BufrDecodeResultBase *result, int sensor, int level, bool mustexist )
{
   double dval;
   float fval;
   string unit;

   if( ! getDescriptor( descriptor, dval, unit, mustexist ) )
      return false;

   fval = unitConvert->convert( kvparam, dval, unit );

   result->add( fval, kvparam, sensor, level );
   return true;
}

void
BufrDecodeBase::
decodeBufrMessage( BufrMessage *bufr, BufrDecodeResultBase *result_ )
{
   if( ! bufr || ! result_ )
      throw BufrException("EXCEPTION: BuffrMessage. NULLPOINTER");

   bufrMessage = bufr;

   try {
      decode( result_ );
      return;
   }
   catch( const BufrException &ex ) {
      throw;
   }
}

}
}
}

