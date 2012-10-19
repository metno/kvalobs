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

#include <iomanip>
#include <iostream>
#include <sstream>
#include <miutil/trimstr.h>
#include "BufrMessage.h"


using namespace std;

namespace {
   string copyToString(const char *chStr, int size )
   {
      string ret( chStr, size );
      miutil::trimstr( ret );
      return ret;
   }
}


namespace kvalobs {
namespace decoder {
namespace bufr {


bool
BufrMessage::
bufrExpand( const std::string &bufr, BufrMessage &bufrMessage )
{
   bufrMessage.kdtlen = KELEM;
   bufrMessage.kdtexplen = KELEM;
   bufrMessage.kelem = KELEM;
   bufrMessage.kvals = KELEM;
   long int kerr;

   bufrMessage.error="No error!";

   if( bufr.empty() )
      return false;

   bufrMessage.valid = false;
   long int length = bufr.length()/4;

   bufrex_( &length , reinterpret_cast<unsigned long int*>( const_cast<char*>(bufr.data())),
            bufrMessage.ksup, bufrMessage.ksec0, bufrMessage.ksec1, bufrMessage.ksec2,
            bufrMessage.ksec3, bufrMessage.ksec4, &bufrMessage.kelem,
            (char **)bufrMessage.cnames, (char **)bufrMessage.cunits,
            &bufrMessage.kvals, bufrMessage.vals, (char **)bufrMessage.cvals, &kerr );

   if( kerr != 0 ) {
      ostringstream ost;

      ost << "Failed to decode bufr. BUFR error code: " << kerr <<".";
      bufrMessage.error = ost.str();
      return false;
   }

   busel_( &bufrMessage.kdtlen, bufrMessage.kdtlist,
           &bufrMessage.kdtexplen, bufrMessage.kdtexplist, &kerr );

   if( kerr != 0 ) {
      ostringstream ost;

      ost << "Failed to decode descriptors. BUFR error code: " << kerr <<".";
      bufrMessage.error = ost.str();
      return false;
   }

   bufrMessage.valueIndex = 0;
   bufrMessage.valid = true;
   return true;
}

DescriptorFloatValue*
BufrMessage::
nextFloatValueDescriptor( DescriptorFloatValue &descriptor )
{

   descriptor.descriptor = -1;
   descriptor.index = -1;
   descriptor.value = RVIND;

   if( ! valid )
      return 0;

   if( valueIndex >= kdtexplen )
      return 0;

   descriptor.descriptor = kdtexplist[ valueIndex ];
   descriptor.index = valueIndex;
   descriptor.value = vals[ valueIndex ];
   descriptor.unit = copyToString( cunits[ valueIndex ], 24 );
   descriptor.name = copyToString( cnames[ valueIndex ], 64 );
   valueIndex++;

   return &descriptor;
}

DescriptorFloatValue*
BufrMessage::
peekAtFloatValueDescriptor( DescriptorFloatValue &descriptor, int index )
{
   descriptor.descriptor = -1;
   descriptor.index = -1;
   descriptor.value = RVIND;

   if( ! valid )
      return 0;

   int i = valueIndex + index;

   if( i >= kdtexplen )
      return 0;

   descriptor.descriptor = kdtexplist[ i ];
   descriptor.index = i;
   descriptor.value = vals[ i ];
   descriptor.unit = copyToString( cunits[ i ], 24 );
   descriptor.name = copyToString( cnames[ i ], 64 );

   return &descriptor;
}

std::string
BufrMessage::
getStringValue( int charDescriptorValue )
{
   //The value of a character descriptor code the index
   //and size in the cvals table as follows.
   // val = index*1000 + size
   // Decoding the value to index and size
   //
   // index = int( value/1000)
   // size = value % 1000

   int index;
   if( ! valid )
         return 0;

   if( charDescriptorValue < 1000 )
      return "";

   index = int( charDescriptorValue/1000 );

   cerr << " **** Index: " << index << " length: " << charDescriptorValue % 1000 << " ksup[6]: " << ksup[6] << endl;
   if( index >ksup[6] )
      return "";

   index--;

   return copyToString( cvals[ index ], charDescriptorValue % 1000 );
}

int
BufrMessage::
descriptorTbl()const
{
   if( ! valid )
      return -1;

   if( kdtlen <= 0 )
      return -1;

   return kdtlist[ 0 ];
}
std::ostream&
operator<<( std::ostream &out, const kvalobs::decoder::bufr::BufrMessage &msg )
{
   if( ! msg.valid ) {
      out << " *** Invalid bufr message *** ";
      return out;
   }

   int kdtexplen;
   out << "kelem:       " << msg.kelem << endl;
   out << "kvals:       " << msg.kvals << endl;
   out << "Dim of Array KSEC0: " << msg.ksup[8] << endl;
   out << "Dim of Array KSEC1: " << msg.ksup[0] << endl;
   out << "Dim of Array KSEC2: " << msg.ksup[1] << endl;
   out << "Dim of Array KSEC3: " << msg.ksup[2] << endl;
   out << "Dim of Array KSEC4: " << msg.ksup[3] << endl;
   out << "Number of expanded elements: " << msg.ksup[4] << endl;
   out << "Number of subsets: " << msg.ksup[5] << endl;
   out << "Number of elements in CVALS: " << msg.ksup[6] << endl;
   out << "Bufr message size: " << msg.ksup[7] << endl;
   out << "Descriptos (unexpanded) #: " << msg.kdtlen << endl;
   out << "Descriptors (expanded) #: " << msg.kdtexplen << endl;

   out << "Unexpanded table. " << endl;
   for( int i=0; i<msg.kdtlen; ++i ) {
      out << "   " << setw(3) << i << ":" << msg.kdtlist[i] << endl;
   }
   out << endl;

   if( msg.kdtexplen != msg.ksup[4] )
      out << "**** WARNING: kdtexplen(" << msg.kdtexplen << ") != ksup[4] (" << msg.ksup[4] << ")" << endl;

   kdtexplen = msg.kdtexplen;

   if( kdtexplen > msg.ksup[4] )
      kdtexplen = msg.ksup[4];

   out << "Expnaded table." << endl;

   int prev=0;
   string val;
   ostringstream oval;
   for( int i=0; i<kdtexplen; ++i ) {
      //F XX YYY
      int N = msg.kdtexplist[i];
      int F=int(N/100000);
      int X=int((N-F*100000)/1000);
      int Y=int((N-X*1000));

      oval.str("");
      if( msg.vals[i] != RVIND )
         oval << msg.vals[i] << " [" << copyToString( msg.cunits[i], 24 ) << "]";

      out << "   " << setw(3) << i << ": "
            <<  setw(1) << setfill('0') << F << " "
            << setw(2) << setfill('0') << X << " "
            << setw(3) << setfill('0') << Y << (N==prev?'*':' ') << " : " << oval.str() << endl;
      prev = N;
   }

   out << "Character Values: " << endl;
   for( int i=0; i< msg.ksup[6]; ++i ) {
      out << "   " <<  setw(4)<<setfill('0')<< i << ": " <<  msg.cvals[i] << endl; //"("<<cnames[i]<<" ["<<cunits[i]<<"])" << endl;
   }

   return out;
 /*  out << endl;
   out << "Values: " << endl;
   for( int i=0; i<ksup[4]; ++i ) {
      out << setw(4)<<setfill('0')<< i << ": " <<  vals[i] << endl; //"("<<cnames[i]<<" ["<<cunits[i]<<"])" << endl;
   }
 */
}

std::ostream&
operator<<( std::ostream &out, const DescriptorFloatValue &v )
{
   if( v.descriptor < 0 )
      return out;

   int N = v.descriptor;
   int F=int(N/100000);
   int X=int((N-F*100000)/1000);
   int Y=int((N-X*1000));

   ostringstream oval;

   if( v.value != RVIND )
      oval << v.value;
   else
      oval << "NA";

   out << setw(1) << setfill('0') << F << " "
       << setw(2) << setfill('0') << X << " "
       << setw(3) << setfill('0') << Y << " : " << oval.str();

   return out;
}
}
}
}
