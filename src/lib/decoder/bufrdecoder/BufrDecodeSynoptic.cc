/*
  Kvalobs - Free Quality Control Software for Meteorological Observations

  $Id: comobsentry.cc,v 1.1.2.1 2007/09/27 09:02:24 paule Exp $

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

#include <math.h>
#include <limits.h>
#include <list>
#include <iostream>
#include <boost/function.hpp>
#include "kvParamdefs.h"
#include "BufrDecodeSynoptic.h"

using namespace std;
namespace PARAMID=kvalobs::decoder::bufr::paramid;

namespace {
using namespace kvalobs::decoder::bufr;

struct Data {
   bool isFloat;
   float fVal;
   string sVal;
   int paramid;
   int hDiffToObstime;

   Data( int pid, float val, int hDiff=0 )
      : isFloat( true ), fVal( val ), paramid( pid ), hDiffToObstime( hDiff )
   {}
   Data( int pid, const string &val, int hDiff=0  )
         : isFloat( false ), sVal( val ), paramid( pid ), hDiffToObstime( hDiff )
      {}
};

double nullTransform( double val ){
   return static_cast<float>( val );
}

float kelvin2c( double val ) {
   return static_cast<float>( val - 273.15);
}

float cloudCover( double val )
{
   return static_cast<float>( floor( (val*8.0/100.0)+0.5 ) );
}

float Pa2hPa( double val ) {
   return val/100.0;
}
typedef list<Data> DataList;
struct BufrContext
{
   int year;
   int month;
   int day;
   int hour;
   int min;
   int II;
   int iii;
   float latitude;
   float longitude;
   string name;
   miutil::miTime obstime;
   float heightOfPressureSensorAboveMeanSeaLevel;
   float heightOfStationAboveMeanSeaLevel;
   float heightOfSenorAboveLocalGround;
   int   replicate;
   int   nCloudSection;
   int   nCloud;
   int   verticalSignificance;
   int   E;
   int   SA;
   int   tPeriodInHours1;
   int   tPeriodInHours2;
   int   tPeriodInHoursDistance;
   int   timeSignificance;
   int   tPeriodInMinutes;
   int   tPeriodInMinuteDistance;
   int   methodOfWaterMeassurement;
   float depthOfWaterMeassurement;

   int ix;
   DataList data;

   BufrContext()
      : year(INT_MAX), month( INT_MAX ), day( INT_MAX ), hour( INT_MAX ), min( 0 ),
        II(INT_MAX), iii(INT_MAX), latitude( FLT_MAX ), longitude( FLT_MAX ),
        heightOfPressureSensorAboveMeanSeaLevel( FLT_MAX ),
        heightOfStationAboveMeanSeaLevel( FLT_MAX ),
        heightOfSenorAboveLocalGround( FLT_MAX ),
        replicate( INT_MAX ), nCloudSection( 0 ), nCloud( 0 ),
        verticalSignificance( INT_MAX ),
        E( INT_MAX ), SA( INT_MAX ),
        tPeriodInHours1( INT_MAX ), tPeriodInHours2( INT_MAX ),
        tPeriodInHoursDistance( INT_MAX ),
        timeSignificance( INT_MAX ), tPeriodInMinutes( INT_MAX ),
        tPeriodInMinuteDistance( INT_MAX ),
        methodOfWaterMeassurement( INT_MAX ),
        depthOfWaterMeassurement( FLT_MAX ),
        ix( INT_MAX )
      {}

   void addData( int kvparamid, const BufrValue &val, boost::function<float (double val) > transform=nullTransform, int hDiff =0  ) {
      if( val.getType() == BufrValue::Float ) {
         if( val.getValue() != DBL_MAX ) {
            //float fval = unitConvert->convert( kvparamid, transform(val.getValue()), val.getUnit() );
            float fval = transform( val.getValue() );
            data.push_back( Data( kvparamid, fval, hDiff ) );
         }
      } else {
         string s = val.getStrValue();
         if( ! s.empty() )
            data.push_back( Data( kvparamid, s, hDiff ) );
      }
   }

   void addData( int kvparamid, int value, int hDiff =0  ) {
      if( value != INT_MAX )
         data.push_back( Data( kvparamid, static_cast<float>( value ), hDiff ) );
   }

   void addData( int kvparamid, float value, int hDiff =0  ) {
      if( value != FLT_MAX )
         data.push_back( Data( kvparamid,  value, hDiff ) );
   }

   void addData( int kvparamid, double value, int hDiff =0  ) {
         if( value != DBL_MAX )
            data.push_back( Data( kvparamid, static_cast<float>( value ), hDiff ) );
      }

   void setReplicate( const BufrValue &val ){
         if( val.getType() == BufrValue::Float ) {
            if( val.getValue() != DBL_MAX ) {
               replicate = static_cast<int>( val.getValue() );
            }
         }
      }

   void  setVerticalSignificance( const BufrValue &val ){
      verticalSignificance = INT_MAX;
      nCloudSection++;
      nCloud = 0;
      if( val.getType() == BufrValue::Float ) {
         if( val.getValue() != DBL_MAX ) {
            verticalSignificance = static_cast<int>( val.getValue() );
         }
      }
   }

   void setheightOfPressureSensorAboveMeanSeaLevel( const BufrValue &val ){
      if( val.getType() == BufrValue::Float ) {
         if( val.getValue() != DBL_MAX ) {
            heightOfPressureSensorAboveMeanSeaLevel = static_cast<float>( val.getValue() );
         }
      }
   }

   void setheightOfStationAboveMeanSeaLevel( const BufrValue &val ){
      if( val.getType() == BufrValue::Float ) {
            if( val.getValue() != DBL_MAX ) {
               heightOfStationAboveMeanSeaLevel = static_cast<float>( val.getValue() );
            }
         }
      }
   void setHeightOfSenorAboveLocalGround( const BufrValue &val ) {
      {
         heightOfSenorAboveLocalGround = FLT_MAX;  //Cancel previous set value as default
         if( val.getType() == BufrValue::Float ) {
            if( val.getValue() != DBL_MAX ) {
               heightOfSenorAboveLocalGround = static_cast<float>( val.getValue() );
            }
         }
      }
   }

   void decodeCloudAmmount( const BufrValue &val ) {
      ++nCloud;
      int N = INT_MAX;
      if( val.getType() == BufrValue::Float ) {
         if( val.getValue() != DBL_MAX ) {
            N = static_cast<int>( val.getValue() );
         }
      }

      if( N == INT_MAX )
         return;

      if( nCloudSection == 1 )
         addData( PARAMID::NH, N );
      else if( nCloudSection > 1 &&
              verticalSignificance>=1 && verticalSignificance <= 4 ) {
         if( nCloud == 1 )
            addData( PARAMID::NS1, N );
         else if( nCloud == 2 )
            addData( PARAMID::NS2, N );
         else if( nCloud == 3 )
            addData( PARAMID::NS3, N );
         else if( nCloud == 4 )
            addData( PARAMID::NS4, N );
      }
   }

   void decodeCloudType( const BufrValue &val ) {
      int N = INT_MAX;
      if( val.getType() == BufrValue::Float ) {
         if( val.getValue() != DBL_MAX ) {
            N = static_cast<int>( val.getValue() );
         }
      }

      if( N == INT_MAX )
         return;

      if( nCloudSection == 1 ) {
         if( N>=10 && N<20 )
            addData( PARAMID::CH, N-10 );
         else if( N>=20 && N<30 )
            addData( PARAMID::CM, N-20 );
         else if( N>=30 && N<40 )
            addData( PARAMID::CL, N-30 );
      } else if( verticalSignificance>=1 && verticalSignificance <= 4 ) {
         if( N >=0 && N<10 ) {
            if( nCloud == 1 )
               addData( PARAMID::CC1, N );
            else if( nCloud == 2 )
               addData( PARAMID::CC2, N );
            else if( nCloud == 3 )
               addData( PARAMID::CC3, N );
            else if( nCloud == 4 )
               addData( PARAMID::CC4, N );
         }
      }
   }

   void decodeHeightOfBaseCloud( const BufrValue &val ) {
      int N = INT_MAX;
      if( val.getType() == BufrValue::Float ) {
         if( val.getValue() != DBL_MAX ) {
            N = static_cast<int>( val.getValue() );
         }
      }

      if( N == INT_MAX )
         return;

      if( nCloudSection == 1 ) {
         addData( PARAMID::HL, N );
      } else if( verticalSignificance>=1 && verticalSignificance <= 4 ) {
         if( nCloud == 1 )
            addData( PARAMID::HS1, N );
         else if( nCloud == 2 )
            addData( PARAMID::HS2, N );
         else if( nCloud == 3 )
            addData( PARAMID::HS3, N );
         else if( nCloud == 4 )
            addData( PARAMID::HS4, N );
      }
   }

   void stateOfGround( const BufrValue &val ) {
      if( val.getType() == BufrValue::Float ) {
         if( val.getValue() != DBL_MAX ) {
            E = static_cast<int>( val.getValue() );
         }
      }
   }

   void snowDepth( const BufrValue &val ) {
      if( val.getType() == BufrValue::Float ) {
         if( val.getValue() != DBL_MAX ) {
            SA = static_cast<int>( val.getValue() * 100 );
         }
      }
   }

   void presentWeather( const BufrValue &val ) {
      if( val.getType() == BufrValue::Float ) {
         if( val.getValue() != DBL_MAX ) {
            int WW = static_cast<int>( val.getValue() );

            if( WW < 100 )
               addData( PARAMID::WW, WW);
            else if( WW>=100 && WW<200 )
               addData( PARAMID::WAWA, WW-100 );
         }
      }
   }

   void pastWeather( int paramid, const BufrValue &val ) {
      if( val.getType() == BufrValue::Float ) {
         if( val.getValue() != DBL_MAX ) {
            int W = static_cast<int>( val.getValue() );
            addData( paramid, W  );
         }
      }
   }

   void setTPeriodInHours( const BufrValue &val ){
      if( val.getType() == BufrValue::Float ) {
         if( val.getValue() != DBL_MAX ) {
            int h = static_cast<int>( val.getValue() );

            if( tPeriodInHoursDistance == 1 )
               tPeriodInHours2 = h;
            else
               tPeriodInHours1 = h;

            tPeriodInHoursDistance = 0;
         }
      }
   }

   void setTPeriodInMinutes( const BufrValue &val ) {
      tPeriodInMinutes = INT_MAX;
      tPeriodInMinuteDistance = 0;

      if( val.getType() == BufrValue::Float ) {
         if( val.getValue() != DBL_MAX ) {
            tPeriodInMinutes = static_cast<int>( val.getValue() );
         }
      }

   }

   void decodePrecipitation( const BufrValue &val ) {

      if( tPeriodInHoursDistance != 1 || tPeriodInHours1 == INT_MAX )
         return;

      if( val.getType() == BufrValue::Float ) {
         if( val.getValue() != DBL_MAX ) {
            float rr = static_cast<float>( val.getValue() );

            if( rr < 0 )
               rr = 0.0;

            switch( tPeriodInHours1 ) {
            case -1: addData( PARAMID::RR_1, rr ); break;
            case -3: addData( PARAMID::RR_3, rr ); break;
            case -6: addData( PARAMID::RR_6, rr ); break;
            case -12: addData( PARAMID::RR_12, rr ); break;
            case -24: addData( PARAMID::RR_24, rr ); break;
            default:
               break;
            }
         }
      }
   }

   void decodeMinMaxTemperature( bool isMax, const BufrValue &val ) {

         if( tPeriodInHoursDistance != 1 || tPeriodInHours1 == INT_MAX || tPeriodInHours2 == INT_MAX )
            return;

         int hPeriod = tPeriodInHours1 - tPeriodInHours2;

         if( val.getType() == BufrValue::Float ) {
            if( val.getValue() != DBL_MAX ) {
               float tt = kelvin2c( val.getValue() );

               switch( hPeriod ) {
                case -1: addData( (isMax ? PARAMID::TAX : PARAMID::TAN), tt, tPeriodInHours2 ); break;
                case -12: addData( (isMax ? PARAMID::TAX_12 : PARAMID::TAN_12), tt, tPeriodInHours2 ); break;
               default:
                  break;
               }
            }
         }
      }

   void setTimeSignificance( const BufrValue &val ) {
      timeSignificance= INT_MAX;

      if( val.getType() == BufrValue::Float ) {
         if( val.getValue() != DBL_MAX ) {
            timeSignificance = static_cast<int>( val.getValue() );
         }
      }
   }

   void windGust( bool isSpeed, const BufrValue &val ) {
      if( tPeriodInMinutes != INT_MAX && tPeriodInMinuteDistance > 2 )
         return;

      if( val.getType() != BufrValue::Float || val.getValue() == DBL_MAX )
         return;

      float dd = static_cast<float>( val.getValue() );

      switch( tPeriodInMinutes  ) {
      case  -10: addData( ( isSpeed ? PARAMID::FG_010 : PARAMID::DG_010 ), dd ); break;
      case  -60: addData( ( isSpeed ? PARAMID::FG_1 : PARAMID::DG_1), dd ); break;
      case -360: addData( ( isSpeed ? PARAMID::FG_6 : PARAMID::DG_6 ), dd ); break;
      case -720: addData( ( isSpeed ? PARAMID::FG_12 : PARAMID::DG_12 ), dd ); break;
      default:
         break;
      }
   }

   void windMax( bool isSpeed, const BufrValue &val ) {
      if( tPeriodInMinutes != INT_MAX && tPeriodInMinuteDistance > 1 )
         return;

      if( val.getType() != BufrValue::Float || val.getValue() == DBL_MAX )
         return;

      float dd = static_cast<float>( val.getValue() );

      switch( tPeriodInMinutes  ) {
      case  -60: addData( ( isSpeed ? PARAMID::FX_1 : PARAMID::DX_1), dd ); break;
      case -180: addData( ( isSpeed ? PARAMID::FX_3 : PARAMID::DX_3), dd ); break;
      case -360: addData( ( isSpeed ? PARAMID::FX_6 : PARAMID::DX_6 ), dd ); break;
      case -720: addData( ( isSpeed ? PARAMID::FX_12 : PARAMID::DX_12 ), dd ); break;
      default:
         break;
      }
   }

   void setMetodOfWaterMeassurement( const BufrValue &val ) {
      methodOfWaterMeassurement = INT_MAX;

      if( val.getType() == BufrValue::Float ) {
         if( val.getValue() != DBL_MAX ) {
            methodOfWaterMeassurement = static_cast<int>( val.getValue() );
         }
      }
   }

   void setDepthOfWaterMeassurement( const BufrValue &val ) {
         depthOfWaterMeassurement = FLT_MAX;

         if( val.getType() == BufrValue::Float ) {
            if( val.getValue() != DBL_MAX ) {
               depthOfWaterMeassurement = static_cast<float>( val.getValue() );
            }
         }
      }
};

int toInt( const BufrValue &val  ) {
   if( val.getType() == BufrValue::Float && val.getValue() != DBL_MAX )
      return static_cast<int>( val.getValue() );
   else
      return INT_MAX;
}

float toFloat( const BufrValue &val  ) {
   if( val.getType() == BufrValue::Float && val.getValue() != DBL_MAX )
      return static_cast<float>( val.getValue() );
   else
      return FLT_MAX;
}



}

namespace kvalobs {
namespace decoder {
namespace bufr {



void
BufrDecodeSynoptic::
decode( BufrDecodeResultBase *result )
{
   BufrContext cntxt;
   BufrValue bufrVal;
   int descriptor;

   while( nextDescriptor(  descriptor, bufrVal ) ) {
      if( cntxt.tPeriodInHoursDistance != INT_MAX )
         ++cntxt.tPeriodInHoursDistance;

      if( cntxt.tPeriodInMinuteDistance != INT_MAX )
         ++cntxt.  tPeriodInMinuteDistance;

      switch( descriptor ){
      case  1001: cntxt.II = toInt( bufrVal ); break;
      case  1002: cntxt.iii = toInt( bufrVal ); break;
      case  1015: cntxt.name = bufrVal.getStrValue(); break;
      case  4001: cntxt.year = toInt( bufrVal ); break;
      case  4002: cntxt.month = toInt( bufrVal ); break;
      case  4003: cntxt.day = toInt( bufrVal ); break;
      case  4004: cntxt.hour = toInt( bufrVal ); break;
      case  4005: cntxt.min = toInt( bufrVal ); break;
      case  5001: cntxt.latitude = toFloat( bufrVal ); break;
      case  6001: cntxt.longitude = toFloat( bufrVal ); break;
      case  7030: cntxt.setheightOfStationAboveMeanSeaLevel( bufrVal); break; //Height of station above mean sea level
      case  7031: cntxt.setheightOfPressureSensorAboveMeanSeaLevel( bufrVal ); break; //Height of barometer above mean sea level
      case 10004: cntxt.addData( PARAMID::PO, bufrVal, Pa2hPa ); break;
      case 10051: cntxt.addData( PARAMID::PR, bufrVal, Pa2hPa ); break;
      case 10061: cntxt.addData( PARAMID::PP, bufrVal, fabsf ); break; //Must take absolute value of this
      case 10063: cntxt.addData( PARAMID::AA, bufrVal ); break;
      case 10062: break; //24 hour pressure change
      case  7004: break; //Pressure (Standard level ) a3
      case 10009: break; //Geopotential height of standard level
      case 12101: cntxt.addData( PARAMID::TA , bufrVal, kelvin2c ); break;
      case 12103: cntxt.addData( PARAMID::TD , bufrVal, kelvin2c ); break;
      case 13003: cntxt.addData( PARAMID::UU , bufrVal );  break;
      case  7032: cntxt.setHeightOfSenorAboveLocalGround( bufrVal ); break; //Height of sensor above local ground
      case 20001: cntxt.addData( PARAMID::VV , bufrVal ); break;
      case 13023: cntxt.addData( PARAMID::RR_24 , bufrVal ); break;
      case 20010: cntxt.addData( PARAMID::NN, bufrVal, cloudCover ); break;
      case  8002: cntxt.setVerticalSignificance( bufrVal ); break;
      case 20011: cntxt.decodeCloudAmmount( bufrVal ); break;
      case 20012: cntxt.decodeCloudType( bufrVal ); break;
      case 20013: cntxt.decodeHeightOfBaseCloud( bufrVal ); break;
      case 20062: cntxt.stateOfGround( bufrVal ); break;
      case 13013: cntxt.snowDepth( bufrVal ); break;
      case 12113: cntxt.addData( PARAMID::TGN_12, bufrVal, kelvin2c ); break;
      case 20003: cntxt.presentWeather( bufrVal ); break;
      case 20004: cntxt.pastWeather( PARAMID::W1, bufrVal ); break;
      case 20005: cntxt.pastWeather( PARAMID::W1, bufrVal ); break;
      case  4024: cntxt.setTPeriodInHours( bufrVal ); break;
      case 13011: cntxt.decodePrecipitation( bufrVal ); break;
      case 12111: cntxt.decodeMinMaxTemperature( true, bufrVal ); break;
      case 12112: cntxt.decodeMinMaxTemperature( false, bufrVal ); break;
      case  8021: cntxt.setTimeSignificance( bufrVal ); break;
      case  4025: cntxt.setTPeriodInMinutes(  bufrVal ); break;
      case 11001: cntxt.addData( PARAMID::DD, bufrVal ); break;
      case 11002: cntxt.addData( PARAMID::FF, bufrVal ); break;
      case 11043: cntxt.windGust( false,  bufrVal ); break;
      case 11041: cntxt.windGust( true, bufrVal ); break;
      case 22061: cntxt.addData( PARAMID::SG, bufrVal ); break;
      case 22043: cntxt.addData( PARAMID::TW, bufrVal, kelvin2c ); break;
      case  2038: cntxt.setMetodOfWaterMeassurement( bufrVal ); break;
      case  7063: cntxt.setDepthOfWaterMeassurement( bufrVal ); break;
      case 11042: cntxt.windMax( true, bufrVal ); break;
      }
   }
   if( cntxt.year == INT_MAX || cntxt.month == INT_MAX || cntxt.day == INT_MAX ||
         cntxt.hour == INT_MAX || cntxt.min ==INT_MAX ) {
      cerr << "Invalid obstime." << endl;
      throw BufrException("Missing observation time");
   }
   miutil::miTime bufrObstime( cntxt.year, cntxt.month, cntxt.day, cntxt.hour, cntxt.min );

   cerr << "Obstime: " <<  bufrObstime << " wmono: "  << (cntxt.II*1000+cntxt.iii) <<  " name: " << cntxt.name << endl;
   cerr << "Position (latitude, longituide) : " << cntxt.latitude<< ", " << cntxt.longitude << endl;

   cerr << "Data:  " << endl;
   for( DataList::iterator it = cntxt.data.begin(); it != cntxt.data.end(); ++it ) {
      cerr << "   " << PARAMID::paramName( it->paramid ) << " (" << it->paramid << "): " << it->fVal << " h: " << it->hDiffToObstime << endl;
   }

   result->setStationid( cntxt.II*1000+cntxt.iii );
   result->setObstime( bufrObstime );
   result->setLatLong( cntxt.latitude, cntxt.longitude );

   miutil::miTime obstime;

   for( DataList::iterator it = cntxt.data.begin(); it != cntxt.data.end(); ++it ) {
      obstime = bufrObstime;

      if( it->hDiffToObstime > 0 )
         obstime.addHour( it->hDiffToObstime );

      result->add( it->fVal, it->paramid, obstime );
   }
}

BufrDecodeSynoptic::
BufrDecodeSynoptic( )
{
}

BufrDecodeSynoptic::
~BufrDecodeSynoptic()
{
}



}
}
}

