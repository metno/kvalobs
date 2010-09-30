/*
 * BufrDecodeBase.h
 *
 *  Created on: Sep 26, 2010
 *      Author: borgem
 */

#ifndef __BUFRDECODEBASE_H__
#define __BUFRDECODEBASE_H__

#include <float.h>
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

class BufrDecodeResultBase
{
public:
   BufrDecodeResultBase(){}
   virtual ~BufrDecodeResultBase(){}

   virtual void setObstime( const miutil::miTime &obstime ) =0;
   virtual miutil::miTime getObstime()const =0;
   virtual void setStationid( int wmono )=0;
   virtual void setLatLong( double latitude, double longitude )=0;
   virtual void add( float value, int kvparamid, int sensor=0, int level=0 )=0;
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

   BufrMessage *bufrMessage;
   BufrToKvUnit_ptr unitConvert;

   /**
    * @exception BufrSequenceException, BufrException, BufrEndException
    * @param descriptor Expected descriptor
    * @param value Set the value from the bufr.
    * @return If mustexist=true an excpetion is thrown if the descriptor
    *         do not exist. If mustexist=false false is returned.
    */
   bool getDescriptor( int descriptor,  double &value, std::string &unit, bool mustexist=true );
   bool getDescriptor( int descriptor,  std::string &value, bool mustexist=true );

   /**
    * @exception BufrSequenceException, BufrException, BufrEndException
    * @param descriptor Expected descriptor.
    * @param kvparam kvalobs parameter id.
    * @param sensor kvalobs  sensor
    * @param level kvalobs level.
    * * @return If mustexist=true an excpetion is thrown if the descriptor
    *         do not exist. If mustexist=false false is returned.
    */
   bool getDescriptor( int descriptor, int kvparam, BufrDecodeResultBase *res, int sensor=0, int level=0, bool mustexist=true );

   bool ignoreDescriptor( int descriptor, bool mustexist=true );

   /**
    * @exception BufrSequenceException, BufrException
    */
   virtual void decode( BufrDecodeResultBase *result )=0;

public:

   BufrDecodeBase( BufrToKvUnit_ptr unitConvert )
      : unitConvert( unitConvert ) {}
   virtual ~BufrDecodeBase(){}
   /**
    * @exception BufrSequenceException, BufrException,BufrEndException
    * @param bufr
    * @return
    */
   virtual void decodeBufrMessage( BufrMessage *bufr, BufrDecodeResultBase *result );

};

}
}
}

#endif /* BUFRDECODEBASE_H_ */
