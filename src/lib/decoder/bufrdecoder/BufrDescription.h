#include <float.h>
#include <limits.h>
#include <vector>

/*
 * BufrDescription.h
 *
 *  Created on: Sep 26, 2010
 *      Author: borgem
 */

#ifndef BUFRDESCRIPTION_H_
#define BUFRDESCRIPTION_H_


namespace kvalobs {
namespace decoder {
namespace bufr {

struct DescriptorType{
   enum { value, loop };
};


struct BufrDescriptor
{
   int descriptor;
   DescriptorType type;
   int val;
   BufrDescriptor( int bufrDescriptor, DescriptorType type )
   : descriptor( bufrDescriptor), type( type), val( -1 )
   {
   }
};

struct KvParam
{
   int paramid;
   int sensor;
   int level;
   float val;

   KvParam( )
         : paramid( INT_MAX ), sensor( INT_MAX ), level( INT_MAX ),
           val( FLT_MAX )
      {
      }
   KvParam( int paramid, int sensor, int level )
      : paramid( paramid ), sensor( sensor ), level( level ),
        val( FLT_MAX )
   {
   }
};

class BufrDescriptorSequence;

class KvParamDecode
{
public:
   KvParam value;
   virtual void decode( const BufrDescriptorSequence &descriptorSequens, float bufrValue )=0;
};

struct KvParamTbl : public std::vector<KvParam>
{
   KvParamTbl();

   void add( const KvParam  &kvParam );
};

class BufrDescriptorSequence : public std::vector<KvParamDecode>
{
public:
   BufrDescriptorSequence( int reserve = 1 );

   void add( int bufrDescriptor, KvParamDecode *kvParamDecode );

};


class BufrDescription
{
public:
   BufrDescription();


};




}
}
}



#endif /* BUFRDESCRIPTION_H_ */
