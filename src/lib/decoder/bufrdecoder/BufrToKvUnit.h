/*
 * BufrToKvUnit.h
 *
 *  Created on: Sep 28, 2010
 *      Author: borgem
 */

#ifndef BUFRTOKVUNIT_H_
#define BUFRTOKVUNIT_H_

#include <boost/shared_ptr.hpp>
#include <string>
#include <set>
#include <map>

namespace kvalobs {
namespace decoder {
namespace bufr {

class UnitElement
{
   std::string fromUnit_;
   float scale_;
   float offset_;
public:
   UnitElement()
      : scale_( 1 ), offset_( 0 )
   {
   }

   UnitElement( const std::string &fromUnit )
      : fromUnit_( fromUnit ), scale_( 1 ), offset_( 0 )
   {
   }

   UnitElement( const std::string &fromUnit, float scale, float offset )
      : fromUnit_( fromUnit ), scale_( scale ), offset_( offset )
   {
   }

   bool operator<(const UnitElement &rhs ) const {
      return fromUnit_<rhs.fromUnit_;
   }

   std::string fromUnit() const { return fromUnit_; }
   float scale()const{ return scale_; }
   float offset()const{ return offset_; }
};

class BufrToKvUnit
{
   typedef std::map<UnitElement, std::set<int> > UnitList;
   UnitList unitList;
   UnitList::const_iterator find( const std::string &unit )const;
   bool findUnit( const std::string &unitName, int kvparam, UnitElement &unit )const;

   UnitList::iterator addUnit( const UnitElement &unit );
   void addKvParam( UnitList::iterator iter, int kvparamid );

public:
   BufrToKvUnit();

   float convert( int kvparamid, double value, const std::string &unit )const;
};

typedef boost::shared_ptr<BufrToKvUnit> BufrToKvUnit_ptr;

}
}
}

#endif /* BUFRTOKVUNIT_H_ */
