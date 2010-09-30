/*
 * BufrDecodeSequences.h
 *
 *  Created on: Sep 29, 2010
 *      Author: borgem
 */

#ifndef BUFRDECODESEQUENCES_H_
#define BUFRDECODESEQUENCES_H_

#include "BufrDecodeBase.h"
namespace kvalobs {
namespace decoder {
namespace bufr {

class BufrDecodeSequences : virtual public BufrDecodeBase
{
protected:


   void decode301004( BufrDecodeResultBase *result );

   ///Identification of date
   void decode301011( miutil::miDate &date );

   ///Identificaion of time
   void decode301012( miutil::miClock &clock );

   ///Identification of geographic coordinates
   void decode301021( double &latitude, double &longitude );

   ///Fixed surface station identification, time and gegraphic coordinates
   void decode301090( BufrDecodeResultBase *result );

   ///Pressure data
   void decode302001(  BufrDecodeResultBase *result  );

   ///Cloud data
   void decode302004(  BufrDecodeResultBase *result );

   void decode302005( int repeat=0 );

   ///Pressure data
   void decode302031(  BufrDecodeResultBase *result );

   ///Temperature and humidity data
   void decode302032( BufrDecodeResultBase *result );

   ///Visibility data
   void decode302033( BufrDecodeResultBase *result );

   ///Precipitation past 24 hours.
   void decode302034( BufrDecodeResultBase *result );

   ///Basic synoptic "instantaneous" data
   void decode302035( BufrDecodeResultBase *result );
   void decode302036( int repeat=0 );
   void decode302037( int repeat=0 );
   void decode302038( int repeat=0 );
   void decode302039( int repeat=0 );
   void decode302040( int repeat=0 );
   void decode302041( int repeat=0 );
   void decode302042( int repeat=0 );
   void decode302043( int repeat=0 );
   void decode302044( int repeat=0 );
   void decode302045( int repeat=0 );
   void decode302046( int repeat=0 );
   void decode302047( int repeat=0 );
   void decode302048( int repeat=0 );
   void decode307079( BufrDecodeResultBase *result );
   void decode307080( int repeat=0 );
   void decode308009( int repeat=0 );

   virtual void decode( BufrDecodeResultBase *result );

public:
   BufrDecodeSequences( BufrToKvUnit_ptr unitConvert );
   virtual ~BufrDecodeSequences();

};


}
}
}

#endif /* BUFRDECODESEQUENCES_H_ */
