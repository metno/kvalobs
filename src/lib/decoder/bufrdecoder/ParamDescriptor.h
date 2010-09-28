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

#ifndef __PARAMDESCRIPTOR_H__
#define __PARAMDESCRIPTOR_H__

#include <list>

namespace kvalobs {
namespace decoder {
namespace bufr {

struct ParamDescriptor
{
   int bufrDescriptor;
   int bufrTime;
   int bufrReplicationfactor;
   int kvParam;
   int kvLevel;
   int kvSensor;

   ParamDescriptor():bufrDescriptor( -1 ), kvParam( -1 ) {}
   ParamDescriptor( int bufrdescriptor, int time, int replicationfactor, int kvparam, int level=0, int sensor=0 )
      : bufrDescriptor( bufrdescriptor), bufrTime( time ), bufrReplicationfactor( replicationfactor ),
        kvParam(kvparam), kvLevel( level), kvSensor( sensor )
   {}

   ParamDescriptor( int bufrdescriptor, int kvparam )
   : bufrDescriptor( bufrdescriptor), bufrTime( 0 ), bufrReplicationfactor( 0 ),
     kvParam(kvparam), kvLevel(  0 ), kvSensor( 0 )
   { }
};

typedef std::list<ParamDescriptor> ParamDescriptorList;

class ParamDescriptorTbl
{
   ParamDescriptorList paramDescriptors;

   void add( const ParamDescriptor &descriptor );
public:
   ParamDescriptorTbl();

   bool find( int bufrDataDescriptor, int bufrTime, int bufrReplicationFactor, ParamDescriptor &paramDescriptor )const;

};


}
}
}
#endif /* PARAMDESCRIPTOR_H_ */
