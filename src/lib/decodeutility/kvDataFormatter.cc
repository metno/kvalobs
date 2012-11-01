/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvDataFormatter.cc,v 1.6.2.4 2007/09/27 09:02:27 paule Exp $                                                       

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
#include "kvDataFormatter.h"
#include <kvalobs/kvDataFlag.h>
#include <kvalobs/kvexception.h>
#include <miutil/commastring.h>
#include <miutil/timeconvert.h>
#include <milog/milog.h>
#include <boost/lexical_cast.hpp>
#include <sstream>


using namespace std;
using namespace boost;
using namespace kvalobs;
using namespace miutil;
using namespace milog;
//using namespace kvservice;


namespace decodeutility {


/**
 * @brief This namespace conatins methods for creating and
 * parsing string representations of a series of observations.
 */
namespace kvdataformatter {

const string createString(const kvData &d) {
   // bug correction:
   int sensor = d.sensor();
   if ( sensor > 9)
      sensor -= '0';

   stringstream ss;
   // 42.1 and not 42.10099212:
   //ss << setprecision(1) << setiosflags(ios::fixed);
   ss << d.stationID()                << internSeparator
         << to_kvalobs_string(d.obstime()) << internSeparator
         << d.original()                 << internSeparator
         << d.paramID()                  << internSeparator
         << to_kvalobs_string(d.tbtime())<< internSeparator
         << d.typeID()                   << internSeparator
         <<   sensor                     << internSeparator
         << d.level()                    << internSeparator
         << d.corrected()                << internSeparator
         << d.controlinfo().flagstring() << internSeparator
         << d.useinfo().flagstring()     << internSeparator
         << d.cfailed();
   return ss.str();
}


const string createString(kvDataList dl) {

   stringstream s;

   for (kvDataList::const_iterator it = dl.begin();
         it != dl.end();
         it++) {

      if ( it != dl.begin() )
         s << mainSeparator;
      s << createString(*it);
   }

   return s.str();
}


kvDataList getKvData( const string & s ) {

   kvDataList ret;
   CommaString mainCS(s, mainSeparator);
   int noOfElements = mainCS.size();

   for (int i = 0; i < noOfElements; i++) {
      const string &current = mainCS[i];

      if (current.empty())
         continue;

      CommaString internCS(current, internSeparator);

      int x;
      try {
         // Parse values:
         x = 0;
         int pos            = lexical_cast<int>  (internCS[x++]);
         const boost::posix_time::ptime obt = boost::posix_time::time_from_string(internCS[x++]);
         float org          = lexical_cast<float>(internCS[x++].data());
         int par            = lexical_cast<int>  (internCS[x++].data());
         const boost::posix_time::ptime tbt = boost::posix_time::time_from_string(internCS[x++]);
         int typ            = lexical_cast<int>  (internCS[x++].data());
         int sen            = lexical_cast<int>  (internCS[x++].data());
         int lvl            = lexical_cast<int>  (internCS[x++].data());
         float cor          = lexical_cast<float>(internCS[x++].data());
         const kvControlInfo cIn                 (internCS[x++]);
         const kvUseInfo uin                     (internCS[x++]);
         const string fai =                     internCS[x++];

         // Create observation object:
         ret.push_back(kvData(pos, obt, org, par,
                              tbt, typ, sen, lvl,
                              cor, cIn, uin, fai));
      }
      catch(std::exception &e) {
         LOGERROR( "Error during parsing of kvData!:" << endl <<
                   "Line " << i << endl <<
                   current << endl <<
                   e.what()
         );

         /*
      	  LogError( "Error during parsing of kvData!:" );
      	  cerr << i << endl;
      	  cerr << current << endl;
      	  LogError( current );
      	  LogError( e.what() );
          */
         throw InvalidInput();
      }
   }
   return ret;
}
};
};  

