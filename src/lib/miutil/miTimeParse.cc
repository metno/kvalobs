/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: miTimeParse.cc,v 1.1.2.7 2007/09/27 09:02:32 paule Exp $                                                       

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
#include <stdio.h>
#include <string.h>
#include "miTimeParse.h"
#include <sstream>
#include <cstdlib>

using namespace std;
using namespace boost::posix_time;
using namespace boost::gregorian;

namespace {

enum TimeSpec {
  Year = 0,
  Month,
  Day,
  Hour,
  Minute,
  Second,
  TimeSpecEnd
};

const char *validSpecifiers = "YymdHMS";

string::size_type
nextInFormat(string::size_type i, const string &format, char &nextChar,
             bool &space, bool &isSpecifier);
bool
isSpace(char ch);

string::size_type
readFromString(string::size_type ipos, int nChar, const std::string &str,
               string &buf, bool expectNumber, int &number);

void
computeNearestTime(int times[],
                   const boost::posix_time::ptime &nearestToThisTime,
                   boost::posix_time::ptime &time_, std::string::size_type pos);

}

std::string::size_type miutil::miTimeParse(
    const std::string &format, const std::string &stringToParse,
    boost::posix_time::ptime &time_,
    const boost::posix_time::ptime &nearestToThisTime) {
  int times[TimeSpecEnd];
  char nextCh;
  bool space;
  bool specifier;

  for (int i = 0; i < TimeSpecEnd; i++)
    times[i] = -1;

  string::size_type iformat = 0;
  string::size_type i = 0;

  iformat = nextInFormat(0, format, nextCh, space, specifier);

  while (iformat != string::npos) {
    if (space) {
      bool hasSpace = false;

      while (i < stringToParse.length() && isSpace(stringToParse[i])) {
        hasSpace = true;
        i++;
      }

      if (!hasSpace)
        throw miTimeParseException("Expecting space!", i, false);
    } else if (!specifier) {
      if (i < stringToParse.length()) {
        if (stringToParse[i] != nextCh) {
          ostringstream ost;
          ost << "Expecting <" << nextCh << "> at " << i;
          throw miTimeParseException(ost.str(), i, false);
        }
      } else {
        throw miTimeParseException("Unexpected end of time string!", i, false);
      }
      i++;
    } else {  //We have a specifier.
      string buf;
      int n;

      //		cerr << "Format: " << nextCh << endl;

      switch (nextCh) {
        case 'Y':
          i = readFromString(i, 4, stringToParse, buf, true, n);
          times[Year] = n;
          break;
        case 'y':
          i = readFromString(i, 2, stringToParse, buf, true, n);
          times[Year] = n;
          break;
        case 'm':
          i = readFromString(i, 2, stringToParse, buf, true, n);
          times[Month] = n;
          break;
        case 'd':
          i = readFromString(i, 2, stringToParse, buf, true, n);
          times[Day] = n;
          break;
        case 'H':
          i = readFromString(i, 2, stringToParse, buf, true, n);
          times[Hour] = n;
          break;
        case 'M':
          i = readFromString(i, 2, stringToParse, buf, true, n);
          times[Minute] = n;
          break;
        case 'S':
          i = readFromString(i, 2, stringToParse, buf, true, n);
          times[Second] = n;
          break;
        default:
          ostringstream ost;
          ost << "Unexpected specifier <" << nextCh << "> at " << iformat
              << " in format specification!";

          throw miTimeParseException(ost.str(), i, false);
      }
    }

    iformat = nextInFormat(iformat, format, nextCh, space, specifier);
  }

  if (!nearestToThisTime.is_special()) {
    computeNearestTime(times, nearestToThisTime, time_, i);
  } else {
    if (times[Second] == -1)
      times[Second] = 0;

    if (times[Minute] == -1)
      times[Minute] = 0;

    if (times[Hour] == -1)
      times[Hour] = 0;

    if (times[Day] == -1) {
      throw miTimeParseException("No day is given", i, false);
    }

    if (times[Month] == -1) {
      throw miTimeParseException("No month is given", i, false);
    }

    if (times[Year] == -1) {
      throw miTimeParseException("No year is given", i, false);
    }

    if (times[Year] < 100) {
      if (times[Year] > 50)
        times[Year] += 1900;
      else
        times[Year] += 2000;
    }

    time_ = ptime(date(times[Year], times[Month], times[Day]),
                  time_duration(times[Hour], times[Minute], times[Second]));
    if (time_.is_special()) {
      ostringstream ost;
      ost << "Not a valid time: " << times[Year] << "-" << times[Month] << "-"
          << times[Day] << " " << times[Hour] << ":" << times[Minute] << ":"
          << times[Second];

      throw miTimeParseException(ost.str(), i, false);
    }
  }

  return i;
}

//TODO: This code must be fixed to match the documentation.
//miutil::miTime
//miutil::
//isoTimeWithMsec( const std::string &timespec, int &msec )
//{
//   msec = 0;
//
//   if( timespec.empty() )
//      return miutil::miTime();
//
//   std::string::size_type p = timespec.find( '.' );
//
//   if( p != std::string::npos ) {
//      sscanf( timespec.substr(p+1).c_str(), "%d", &msec);
//      return miutil::miTime( timespec.substr(0, p) );
//   } else {
//      return miutil::miTime( timespec );
//   }
//}

namespace {

bool isSpace(char ch) {
  if (ch == ' ' || ch == '\n' || ch == '\t' || ch == '\r')
    return true;

  return false;
}

string::size_type nextInFormat(string::size_type i, const string &format,
                               char &nextChar, bool &space, bool &specifier) {
  space = false;
  specifier = false;

  if (i >= format.length())
    return string::npos;

  nextChar = format[i];

  if (isSpace(nextChar)) {
    while (i < format.length() && isSpace(format[i]))
      i++;

    space = true;
    return i;
  } else
    i++;

  if (nextChar == '%') {
    if (i >= format.length())
      throw miutil::miTimeParseException(
          "Format: Unexpected length of formatstring!", i, true);

    nextChar = format[i];
    i++;

    if (nextChar == '%')
      return i;

    if (index(validSpecifiers, nextChar) == NULL) {
      ostringstream ost;
      ost << "Format: Invalid specifier (" << nextChar << ") at: " << i - 1;
      throw miutil::miTimeParseException(ost.str(), i - 1, true);
    }

    specifier = true;
    return i;
  }

  return i;
}

string::size_type readFromString(string::size_type ipos, int nChar,
                                 const std::string &str, string &buf,
                                 bool expectNumber, int &number) {
  if ((ipos + nChar) > str.length()) {
    ostringstream ost;

    ost << "Unexpected end of time string. Expecting " << nChar << " from pos "
        << ipos;
    throw miutil::miTimeParseException(ost.str(), ipos, false);
  }

  buf = str.substr(ipos, nChar);

  if (expectNumber) {
    string::size_type i = 0;

    for (; i < buf.length() && isdigit(buf[i]); i++)
      ;

    if (i < buf.length()) {
      ostringstream ost;

      ost << "Expecting a number from pos: " << ipos << " to " << ipos + nChar
          << " got (" << buf << ")!";
      throw miutil::miTimeParseException(ost.str(), ipos, false);
    }

    number = atoi(buf.c_str());
  }

  return ipos + nChar;
}

void computeNearestTime(int times[], const boost::posix_time::ptime &nt,
                        boost::posix_time::ptime &time_,
                        std::string::size_type pos) {
  using namespace miutil;
  int i = Year;
  int first;
  int last;

  for (; i < TimeSpecEnd && times[i] == -1; i++)
    ;

  first = i;

  if (i == TimeSpecEnd)  //Should not happend.
    throw miTimeParseException("Unexpected: No time information parsed!", pos,
                               false);

  for (; i < TimeSpecEnd && times[i] != -1; i++)
    last = i;

  if (i < TimeSpecEnd) {
    for (; i < TimeSpecEnd && times[i] == -1; i++)
      ;

    if (i != TimeSpecEnd)
      throw miTimeParseException("Unable to deduce a time to nearest time!",
                                 pos, false);
  }

  if (times[Second] == -1)
    times[Second] = 0;

  if (times[Minute] == -1)
    times[Minute] = 0;

  if (times[Hour] == -1)
    times[Hour] = 0;

  ptime tmp(nt);
  time_ = ptime();  //Undef state

  switch (first) {
    case Second:
      if (times[Second] > tmp.time_of_day().seconds())
        tmp += seconds(-1);

      time_ = ptime(
          tmp.date(),
          time_duration(tmp.time_of_day().hours(), tmp.time_of_day().minutes(),
                        times[Second]));
      break;
    case Minute:
      if (times[Minute] > tmp.time_of_day().minutes()
          || (times[Minute] == tmp.time_of_day().minutes()
              && times[Second] > tmp.time_of_day().seconds()))
        tmp += hours(-1);

      time_ = ptime(
          tmp.date(),
          time_duration(tmp.time_of_day().hours(), times[Minute],
                        times[Second]));
      break;
    case Hour:
      if (times[Hour] > tmp.time_of_day().hours()
          || (times[Hour] == tmp.time_of_day().hours()
              && times[Minute] > tmp.time_of_day().minutes())
          || (times[Hour] == tmp.time_of_day().hours()
              && times[Minute] == tmp.time_of_day().minutes()
              && times[Second] > tmp.time_of_day().seconds()))
        tmp += days(-1);

      time_ = ptime(tmp.date(),
                    time_duration(times[Hour], times[Minute], times[Second]));
      break;
    case Day: {
      int y = tmp.date().year();
      int m = tmp.date().month();

      if (times[Day] > tmp.date().day()
          || (times[Day] == tmp.date().day()
              && times[Hour] > tmp.time_of_day().hours())
          || (times[Day] == tmp.date().day()
              && times[Hour] == tmp.time_of_day().hours()
              && times[Minute] > tmp.time_of_day().minutes())
          || (times[Day] == tmp.date().day()
              && times[Hour] == tmp.time_of_day().hours()
              && times[Minute] == tmp.time_of_day().minutes()
              && times[Second] > tmp.time_of_day().seconds())) {
        m--;
        if (m < 1) {
          y--;
          m = 12;
        }
      }
      time_ = ptime(date(y, m, times[Day]),
                    time_duration(times[Hour], times[Minute], times[Second]));
    }
      break;
    case Month: {
      int y = tmp.date().year();

      if (times[Month] > tmp.date().month()
          || (times[Month] == tmp.date().month()
              && times[Day] > tmp.date().day())
          || (times[Month] == tmp.date().month()
              && times[Day] == tmp.date().day()
              && times[Hour] > tmp.time_of_day().hours())
          || (times[Month] == tmp.date().month()
              && times[Day] == tmp.date().day()
              && times[Hour] == tmp.time_of_day().hours()
              && times[Minute] > tmp.time_of_day().minutes())
          || (times[Month] == tmp.date().month()
              && times[Day] == tmp.date().day()
              && times[Hour] == tmp.time_of_day().hours()
              && times[Minute] == tmp.time_of_day().minutes()
              && times[Second] > tmp.time_of_day().seconds()))
        y--;

      time_ = ptime(date(y, times[Month], times[Day]),
                    time_duration(times[Hour], times[Minute], times[Second]));

    }

      break;
    case Year:
      if (times[Year] < 100) {
        if (times[Year] > 50)
          times[Year] += 1900;
        else
          times[Year] += 2000;
      }

      time_ = ptime(date(times[Year], times[Month], times[Day]),
                    time_duration(times[Hour], times[Minute], times[Second]));

      if (time_.is_special()) {
        ostringstream ost;
        ost << "Not a valid time: " << times[Year] << "-" << times[Month] << "-"
            << times[Day] << " " << times[Hour] << ":" << times[Minute] << ":"
            << times[Second];
        throw miTimeParseException(ost.str(), pos, false);
      }

      break;
    default:
      throw miTimeParseException("Unexpected No \"first\" timeSpec!", pos,
                                 false);
  }
}

}
