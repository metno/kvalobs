/*
 * BufrDecodeBase.h
 *
 *  Created on: Sep 26, 2010
 *      Author: borgem
 */

#ifndef __BUFRDECODEBASE_H__
#define __BUFRDECODEBASE_H__

#include <limits.h>
#include <list>
#include <exception>
#include <string>
#include <kvalobs/kvData.h>
#include "BufrMessage.h"
#include "BufrToKvUnit.h"

namespace kvalobs {
namespace decoder {
namespace bufr {

struct BufrDecodeResult
{
   std::list<kvalobs::kvData> data;
   miutil::miTime obstime;
   miutil::miTime tbtime;
   int stationid;
   int typeid_;

   BufrDecodeResult():
      tbtime( miutil::miTime::nowTime() ), stationid( INT_MAX ), typeid_( 7 ) {}

   void add( float value, int kvparamid, int sensor=0, int level=0 );
};


class BufrException : public std::exception
{
protected:
   std::string what_;

public:
   BufrException(){}
   BufrException( const std::string &what  )
      : what_( what )
   {}
   ~BufrException()throw() {}

   const char *what() const throw() {
      return what_.c_str();
   }

};


class BufrEndException : public BufrException
{
protected:
   std::string what_;

public:
   BufrEndException( const std::string &what=""  )
      : what_( what )
   {
   }
   ~BufrEndException()throw(){}

   const char *what() const throw() {
      if( what_.empty() )
         const_cast<BufrEndException*>(this)->what_="Unexpected end of BUFR.";
      return what_.c_str();
   }
};

class BufrSequenceException : public BufrException
{
public:
   int expectedDescriptor;
   int actualDescriptor;

   BufrSequenceException( int expected, int actual )
      : expectedDescriptor( expected ), actualDescriptor( actual )
   {}

   const char *what() const throw() {
      std::ostringstream out;
      out << "BUFR: SequenceException expected descriptor: "<< expectedDescriptor << " actual descriptor: " << actualDescriptor;
      const_cast<BufrSequenceException*>(this)->what_ = out.str();
      return what_.c_str();
   }
};




class BufrDecodeBase
{
protected:

   BufrDecodeResult* result;
   BufrMessage *bufrMessage;
   BufrToKvUnit_ptr unitConvert;

   /**
    * @exception BufrSequenceException, BufrException, BufrEndException
    * @param descriptor Expected descriptor
    * @param value Set the value from the bufr.
    * @return If mustexist=true an excpetion is thrown if the descriptor
    *         do not exist. If mustexist=false false is returned.
    */
   bool decode( int descriptor,  double &value, std::string &unit, bool mustexist=true );
   bool decode( int descriptor,  std::string &value, bool mustexist=true );

   /**
    * @exception BufrSequenceException, BufrException, BufrEndException
    * @param descriptor Expected descriptor.
    * @param kvparam kvalobs parameter id.
    * @param sensor kvalobs  sensor
    * @param level kvalobs level.
    * * @return If mustexist=true an excpetion is thrown if the descriptor
    *         do not exist. If mustexist=false false is returned.
    */
   bool decode( int descriptor, int kvparam, int sensor=0, int level=0, bool mustexist=true );

   /**
    * @exception BufrSequenceException, BufrException
    */
   virtual void decode()=0;

public:

   BufrDecodeBase( BufrToKvUnit_ptr unitConvert )
      : unitConvert( unitConvert ) {}
   /**
    * @exception BufrSequenceException, BufrException,BufrEndException
    * @param bufr
    * @return
    */
   virtual BufrDecodeResult* decode( BufrMessage *bufr );

};

}
}
}

#endif /* BUFRDECODEBASE_H_ */
