/*
 * BufrToKvUnit.h
 *
 *  Created on: Sep 28, 2010
 *      Author: borgem
 */

#include "kvParamdefs.h"
#include "BufrToKvUnit.h"


namespace kvalobs {
namespace decoder {
namespace bufr {

namespace PARAM=kvalobs::decoder::bufr::paramid;

using namespace std;

BufrToKvUnit::UnitList::const_iterator
BufrToKvUnit::find( const std::string &unit )const
{
   return unitList.find( UnitElement( unit ) );
}

BufrToKvUnit::UnitList::iterator
BufrToKvUnit::addUnit( const UnitElement &unit )
{
   pair<UnitList::iterator,bool> ret;

   ret = unitList.insert( pair<UnitElement, set<int> >( unit, set<int>() ) );
   return ret.first;
}

void
BufrToKvUnit::
addKvParam( BufrToKvUnit::UnitList::iterator iter, int kvparamid )
{
   iter->second.insert( kvparamid );
}

bool
BufrToKvUnit::
findUnit( const std::string &unitName, int kvparam, UnitElement &unit )const
{
   UnitList::const_iterator unitIt=find( unitName );
   if( unitIt == unitList.end() )
      return false;

   set<int>::const_iterator paramIt=unitIt->second.find( kvparam );

   if( paramIt == unitIt->second.end() )
      return false;

   unit = unitIt->first;
   return true;
}

BufrToKvUnit::
BufrToKvUnit()
{
   UnitList::iterator unitIt;
   UnitElement unit( "K", 1, -273.15 );

   //Temperature conversions from kelvin to celcius.
   unitIt = addUnit( unit );
   addKvParam( unitIt, PARAM::TA );
   addKvParam( unitIt, PARAM::TD );
   addKvParam( unitIt, PARAM::TAN_12 );
   addKvParam( unitIt, PARAM::TAX_12);
   addKvParam( unitIt, PARAM::TGN_12 );
   addKvParam( unitIt, PARAM::TW );

   //Convert from Pascal to milli pascal.
   unitIt = addUnit( UnitElement( "PA", 0.001, 0 ) );
   addKvParam( unitIt, PARAM::PO );
   addKvParam( unitIt, PARAM::PP );
   addKvParam( unitIt, PARAM::PR );
}

float
BufrToKvUnit::
convert( int kvparamid, double value, const std::string &unitName )const
{
   UnitElement unit;

   if( findUnit( unitName, kvparamid, unit ) )
      return unit.scale()*static_cast<float>(value)+unit.offset();

   return static_cast<float>( value );
}

}
}
}

