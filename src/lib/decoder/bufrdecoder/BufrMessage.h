/*
 * BufrMessage.h
 *
 *  Created on: Sep 26, 2010
 *      Author: borgem
 */

#ifndef __BUFRMESSAGE_H__
#define __BUFRMESSAGE_H__

#include <iostream>
#include <string>
#include "bufrdefs.h"


namespace kvalobs {
namespace decoder {
namespace bufr {

struct DescriptorFloatValue
{
   int descriptor;
   int index;
   int replication;
   double value;
   std::string unit;
   std::string name;

   friend std::ostream& operator<<( std::ostream &out, const DescriptorFloatValue &v );
};

class BufrMessage
{
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
   BufrMessage()
      :valid( false ), kdtlen( KELEM ), kdtexplen( KELEM ), kelem( KELEM ), kvals( KELEM )
   {}

   static bool bufrExpand( const std::string &bufr, BufrMessage &bufrMessage );

   DescriptorFloatValue* nextFloatValueDescriptor( DescriptorFloatValue &descriptor );
   std::string getStringValue( int charDescriptorValue );

   int descriptorTbl()const;

   friend std::ostream& operator<<( std::ostream &out, const kvalobs::decoder::bufr::BufrMessage &msg );
};

std::ostream& operator<<( std::ostream &out, const kvalobs::decoder::bufr::BufrMessage &msg );
std::ostream& operator<<( std::ostream &out, const DescriptorFloatValue &v );

}
}
}

//std::ostream& operator<<( std::ostream &out, const kvalobs::decoder::bufr::BufrMessage &msg );
#endif /* BUFRMESSAGE_H_ */
