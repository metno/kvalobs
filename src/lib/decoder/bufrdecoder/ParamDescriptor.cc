/*
  Kvalobs - Free Quality Control Software for Meteorological Observations

  $Id: comobsdecode.cc,v 1.7.2.6 2007/09/27 09:02:24 paule Exp $

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

#include "ParamDescriptor.h"
#include "kvParamdefs.h"

namespace kvalobs {
namespace decoder {
namespace bufr {


namespace ParamID=kvalobs::decoder::bufr::paramid;



void
ParamDescriptorTbl::
add( const ParamDescriptor &descriptor )
{
   ParamDescriptor dummy;

   if( find( descriptor.bufrDescriptor, descriptor.bufrTime, descriptor.bufrReplicationfactor, dummy ) )
      return;

   paramDescriptors.push_back( descriptor );
}

ParamDescriptorTbl::
ParamDescriptorTbl()
{
   add( ParamDescriptor( 10004, 0, -1, ParamID::PO, 0, 0 ) );
   add( ParamDescriptor( 10051, 0, -1, ParamID::PR, 0, 0 ) );
   add( ParamDescriptor( 10061, 0, -1, ParamID::PP, 0, 0 ) );
   add( ParamDescriptor( 10063, 0, -1, ParamID::AA, 0, 0 ) );
   add( ParamDescriptor( 12101, 0, -1, ParamID::TA, 0, 0 ) );
   add( ParamDescriptor( 12103, 0, -1, ParamID::TD, 0, 0 ) );
   add( ParamDescriptor( 13003, 0, -1, ParamID::UU, 0, 0 ) );
   add( ParamDescriptor( 20001, 0, -1, ParamID::VV, 0, 0 ) );
   add( ParamDescriptor( 13023, 0, -1, ParamID::RR_24, 0, 0 ) );
   add( ParamDescriptor( 20010, 0, -1, ParamID::NN, 0, 0 ) );
   add( ParamDescriptor( 20011, 0, -1, ParamID::NH, 0, 0 ) );
   add( ParamDescriptor( 20012, 0, -1, ParamID::CL, 0, 0 ) );
   add( ParamDescriptor( 20012, 0, -2, ParamID::CM, 0, 0 ) );
   add( ParamDescriptor( 20012, 0, -3, ParamID::CH, 0, 0 ) );
   add( ParamDescriptor( 20011, 0, -1, ParamID::NH, 0, 0 ) );
   add( ParamDescriptor( 20013, 0, -1, ParamID::HL, 0, 0 ) );

}

bool
ParamDescriptorTbl::
find( int bufrDataDescriptor, int bufrTime, int bufrReplicationFactor, ParamDescriptor &paramDescriptor_ )const
{
   for( ParamDescriptorList::const_iterator it = paramDescriptors.begin();
        it != paramDescriptors.end(); ++it ) {
      if( it->bufrDescriptor == bufrDataDescriptor &&
            it->bufrTime == bufrTime &&
            it->bufrReplicationfactor == bufrReplicationFactor ) {
         paramDescriptor_ = *it;
         return true;
      }
   }

   return false;
}


}
}
}

