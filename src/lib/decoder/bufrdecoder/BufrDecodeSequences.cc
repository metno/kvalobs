
#include <limits.h>
#include <iostream>
#include "kvParamdefs.h"
#include "BufrDecodeSequences.h"


namespace PARAMID=kvalobs::decoder::bufr::paramid;

namespace kvalobs {
namespace decoder {
namespace bufr {

using namespace std;

void
BufrDecodeSequences::
decode301004( BufrDecodeResultBase *result )
{
   string unit;
   string name;
   double block;
   double station;

   int wmono = INT_MAX;

   getDescriptor( 1001, block, unit );
   getDescriptor( 1002, station, unit );
   getDescriptor( 1015, name );
   getDescriptor( 2001, PARAMID::IX, result );

   if( block != DBL_MAX && station != DBL_MAX ) {
      wmono = static_cast<int>(block)*1000+static_cast<int>(station);
      result->setStationid( wmono );
   }

   cerr << "wmono: " << wmono << endl;
   cerr << "name: " << name << endl;
}

void
BufrDecodeSequences::
decode301011( miutil::miDate &date  )
{
   string unit;
   double value;

   date = miutil::miDate(); //Undef
   int year = INT_MAX;
   int month = INT_MAX;
   int day = INT_MAX;

    getDescriptor( 4001, value, unit );

    if( value != FLT_MAX )
       year = static_cast<int>( value );

    getDescriptor( 4002, value, unit );

    if( value != FLT_MAX )
       month = static_cast<int>( value );

    getDescriptor( 4003, value, unit );

    if( value != FLT_MAX )
       day = static_cast<int>( value );

    if( year != INT_MAX && month != INT_MAX && day != INT_MAX )
       date = miutil::miDate( year, month, day );

    cerr << "Obsday: " << date << endl;

}

void
BufrDecodeSequences::
decode301012(  miutil::miClock &clock  )
{
   string unit;
   double value;

   clock = miutil::miClock(); //Undef
   int hour = INT_MAX;
   int min = INT_MAX;

   getDescriptor( 4004, value, unit );

   if( value != FLT_MAX )
      hour = static_cast<int>( value );

   getDescriptor( 4005, value, unit );

   if( value != FLT_MAX )
      min = static_cast<int>( value );

   if( hour != INT_MAX && min != INT_MAX )
       clock = miutil::miClock( hour, min, 0 );

   cerr << "Obsclock: " << clock << endl;
}

void
BufrDecodeSequences::
decode301021( double &latitude, double &longitude )
{
   string unit;

   getDescriptor( 5001, latitude, unit );
   getDescriptor( 6001, longitude, unit );
}

void
BufrDecodeSequences::
decode301090( BufrDecodeResultBase *result )
{
   miutil::miDate date;
   miutil::miClock clock;
   double latitude, longitude;

   decode301004( result );
   decode301011( date );
   decode301012( clock );

   if( !date.undef() && !clock.undef() )
      result->setObstime( miutil::miTime( date, clock ) );

   decode301021( latitude, longitude );

   if( latitude != DBL_MAX || longitude != DBL_MAX )
      result->setLatLong( latitude, longitude );

   ignoreDescriptor( 7030 );
   ignoreDescriptor( 7031 );
}

void
BufrDecodeSequences::
decode302001(  BufrDecodeResultBase *result  )
{
   getDescriptor( 10004, PARAMID::PO, result );
   getDescriptor( 10051, PARAMID::PR, result );
   getDescriptor( 10061, PARAMID::PP, result );
   getDescriptor( 10063, PARAMID::AA, result );
}

void
BufrDecodeSequences::
decode302004(  BufrDecodeResultBase *result  )
{
   struct {
      int vs;
      int NS;
      int CC;
      int HS;
   } Layer[] = { {-1, PARAMID::NS1, PARAMID::CC1, PARAMID::HS1 },
                 {-1, PARAMID::NS2, PARAMID::CC2, PARAMID::HS2 },
                 {-1, PARAMID::NS3, PARAMID::CC3, PARAMID::HS3 },
                 {-1, PARAMID::NS4, PARAMID::CC4, PARAMID::HS4 },
                 { 0, 0, 0, 0 }
               };
   string unit;
   double val;
   int repeat=0;

   getDescriptor(20010, PARAMID::NN, result );
   ignoreDescriptor( 8002 ); //FIXME:
   getDescriptor(20011, PARAMID::NH, result );
   getDescriptor(20013, PARAMID::HL, result );
   getDescriptor(20012, PARAMID::CL, result );
   getDescriptor(20012, PARAMID::CM, result );
   getDescriptor(20012, PARAMID::CH, result );

   getDescriptor( 31001, val, unit );

   if( val != DBL_MAX )
      repeat = static_cast<int>( val );

   for( int i=0; i<repeat && Layer[i].NS != 0; ++i ) {
      ignoreDescriptor(8002);
      getDescriptor( 20011, Layer[i].NS, result );
      getDescriptor( 20012, Layer[i].CC, result );
      getDescriptor( 20013, Layer[i].HS, result );
   }
}

void
BufrDecodeSequences::
decode302005( int repeat )
{
}

void
BufrDecodeSequences::
decode302031( BufrDecodeResultBase *result )
{
   decode302001( result );
   ignoreDescriptor( 10062 );
   ignoreDescriptor( 7004 );
   ignoreDescriptor( 10009 );
}

void
BufrDecodeSequences::
decode302032( BufrDecodeResultBase *result )
{
   ignoreDescriptor( 7032 );
   getDescriptor( 12101, PARAMID::TA, result );
   getDescriptor( 12103, PARAMID::TD, result );
   getDescriptor( 13003, PARAMID::UU, result );
}

void
BufrDecodeSequences::
decode302033( BufrDecodeResultBase *result )
{
   ignoreDescriptor( 7032 );
   getDescriptor( 20001, PARAMID::VV, result );
}

void
BufrDecodeSequences::
decode302034( BufrDecodeResultBase *result )
{
   ignoreDescriptor( 7032 );
   getDescriptor( 13023, PARAMID::RR_24, result );
}

void
BufrDecodeSequences::
decode302035( BufrDecodeResultBase *result )
{
   decode302032( result );
   decode302033( result );
   decode302034( result );
   ignoreDescriptor( 7032 ); //FIXME: What does this do.
   decode302004( result );

}

void
BufrDecodeSequences::
decode302036( int repeat )
{
}

void
BufrDecodeSequences::
decode302037( int repeat )
{
}

void
BufrDecodeSequences::
decode302038( int repeat )
{
}

void
BufrDecodeSequences::
decode302039( int repeat )
{
}

void
BufrDecodeSequences::
decode302040( int repeat )
{
}

void
BufrDecodeSequences::
decode302041( int repeat )
{
}

void
BufrDecodeSequences::
decode302042( int repeat )
{
}

void
BufrDecodeSequences::
decode302043( int repeat )
{
}

void
BufrDecodeSequences::
decode302044( int repeat )
{
}

void
BufrDecodeSequences::
decode302045( int repeat )
{
}

void
BufrDecodeSequences::
decode302046( int repeat )
{
}

void
BufrDecodeSequences::
decode302047( int repeat )
{
}

void
BufrDecodeSequences::
decode302048( int repeat )
{
}

void
BufrDecodeSequences::
decode307079( BufrDecodeResultBase *result )
{
   decode301090( result );
   decode302031( result );
   decode302035( result );
   decode302036();
}

void
BufrDecodeSequences::
decode307080( int repeat )
{
}


void
BufrDecodeSequences::
decode308009( int repeat )
{

}


void
BufrDecodeSequences::
decode( BufrDecodeResultBase *result )
{
   int mainDescriptor=bufrMessage->descriptorTbl();

   cerr << "Decode: " << mainDescriptor << endl;
   if( mainDescriptor == 307079 )
      decode307079( result );
   else if( mainDescriptor == 307080 )
      decode307080();
   else if( mainDescriptor == 308009 )
      decode308009( );
   else
      cerr << "Unknown main decriptor: " << mainDescriptor << endl;
}

BufrDecodeSequences::
BufrDecodeSequences( BufrToKvUnit_ptr unitConvert )
   : BufrDecodeBase( unitConvert )
{
}

BufrDecodeSequences::
~BufrDecodeSequences()
{
}



}
}
}

