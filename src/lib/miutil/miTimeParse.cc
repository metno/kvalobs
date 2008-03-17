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
#include "miTimeParse.h"

using namespace std;

namespace{

typedef enum TimeSpec{Year=0, Month, Day, Hour, Minute, Second, 
	                  TimeSpecEnd};
	
const string validSpecifiers("YymdHMS");

string::size_type 
nextInFormat(string::size_type i, 
		 	 const string &format,
		 	 char         &nextChar,
		 	 bool         &space,
		 	 bool         &isSpecifier );
bool
isSpace(char ch);

string::size_type
readFromString(string::size_type ipos, 
			   int nChar, 
			   const std::string &str,
			   string            &buf,
			   bool              expectNumber,
			   int               &number);
			   
void
computeNearestTime(int times[], 
				   const miutil::miTime &nearestToThisTime, 
				   miutil::miTime &time_,
				   std::string::size_type pos);
			 	 
}


std::string::size_type
miutil::
miTimeParse(const std::string &format, 
		   	const std::string &stringToParse,
		   	miutil::miTime &time_,
			const miutil::miTime &nearestToThisTime)
{
	int  times[TimeSpecEnd];
	char nextCh;
	bool space;
	bool specifier;
	
	for(int i=0; i<TimeSpecEnd; i++)
		times[i]=-1;
	
	string::size_type iformat=0;
	string::size_type i=0;
	
	iformat=nextInFormat(0, format, nextCh, space, specifier);
	
	while(iformat!=string::npos){
		if(space){
			bool hasSpace=false;
			
			while(i<stringToParse.length() && 
				  isSpace(stringToParse[i])){
				hasSpace=true;
				i++;
			}
			
			if(!hasSpace)
				throw miTimeParseException("Expecting space!",
										    i, false);
		}else if(!specifier){
			if(i<stringToParse.length()){
				if(stringToParse[i]!=nextCh){
					ostringstream ost;
					ost << "Expecting <" << nextCh << "> at " << i;
					throw miTimeParseException(ost.str(), i, false);
				}
			}else{
				throw miTimeParseException("Unexpected end of time string!",
										    i, false);
			}
			i++;
		}else{ //We have a specifier.
			string buf;
			int    n;
			
			cerr << "Format: " << nextCh << endl;
			
			switch(nextCh){
			case 'Y':
				i=readFromString(i, 4, stringToParse, buf, true, n);
				times[Year]=n;
				break;
			case 'y':
				i=readFromString(i, 2, stringToParse, buf, true, n);
				times[Year]=n;
				break;
			case 'm':
				i=readFromString(i, 2, stringToParse, buf, true, n);
				times[Month]=n;
				break;
			case 'd':
				i=readFromString(i, 2, stringToParse, buf, true, n);
				times[Day]=n;
				break;
			case 'H':
				i=readFromString(i, 2, stringToParse, buf, true, n);
				times[Hour]=n;
				break;
			case 'M':
				i=readFromString(i, 2, stringToParse, buf, true, n);
				times[Minute]=n;
				break;
			case 'S':
				i=readFromString(i, 2, stringToParse, buf, true, n);
				times[Second]=n;
				break;
			default:
				ostringstream ost;
				ost << "Unexpected specifier <" << nextCh << "> at " << iformat
					<< " in format specification!";
					
				throw miTimeParseException(ost.str(), i, false);
			}
		}
		
		iformat=nextInFormat(iformat, format, nextCh, space, specifier);
	}
	
	
	if(!nearestToThisTime.undef()){
		computeNearestTime(times, nearestToThisTime, time_, i);
	}else{
		if(times[Second]==-1)
			times[Second]=0;
		
		if(times[Minute]==-1)
			times[Minute]=0;
		
		if(times[Hour]==-1)
			times[Hour]=0;

	
		if(times[Day]==-1){
			throw miTimeParseException("No day is given", i, false);
		}
	
		if(times[Month]==-1){
			throw miTimeParseException("No month is given", i, false);
		}
	
		if(times[Year]==-1){
			throw miTimeParseException("No year is given", i, false);
		}
		
		if(times[Year]<100){
			if(times[Year]>50)
				times[Year]+=1900;
			else
				times[Year]+=2000;
		}
		
		if(!miTime::isValid(times[Year], times[Month], times[Day], 
				  	 	   times[Hour], times[Minute], times[Second])){
			ostringstream ost;
			ost << "Not a valid time: "
				<< times[Year] << "-" << times[Month] << "-" << times[Day]
				<< " " << times[Hour] << ":" << times[Minute] << ":" << times[Second];
			
			throw miTimeParseException(ost.str(), i, false);
		}
		
		time_=miTime(times[Year], times[Month], times[Day], 
				  	 times[Hour], times[Minute], times[Second]);
				  	 
	
	}
	
	return i;
}	
		
namespace{

bool
isSpace(char ch){
	if(ch==' ' || ch=='\n' || ch=='\t' || ch=='\r')
		return true;
				
	return false;
}

	
string::size_type 
nextInFormat(string::size_type i, 
			 const string &format,
			 char         &nextChar,
			 bool         &space,
			 bool         &specifier){
	space=false;
	specifier=false;
	
	if(i>=format.length())
		return string::npos;
		
	nextChar=format[i];
		
	if(isSpace(nextChar)){
		while(i<format.length() && isSpace(format[i]))
			i++;
			
		space=true;
		return i;
	}else
		i++;


	if(nextChar=='%'){
		if(i>=format.length())
			throw miutil::miTimeParseException("Format: Unexpected length of formatstring!",
										i, true);
			
		nextChar=format[i];
		i++;
		
		if(nextChar=='%')		
			return i;
			
		if(validSpecifiers.find(nextChar)==string::npos){
			ostringstream ost;
			ost << "Format: Invalid specifier (" << nextChar << ") at: " << i-1;
			throw  miutil::miTimeParseException(ost.str(), i-1, true);
		}
		
		specifier=true;
		return i;
	}	
	
	return i;
}


string::size_type
readFromString(string::size_type ipos, 
			   int nChar, 
			   const std::string &str,
			   string            &buf,
			   bool              expectNumber,
			   int               &number)
{
	if((ipos+nChar)>str.length()){
		ostringstream ost;
		
		ost << "Unexpected end of time string. Expecting " << nChar 
			<< " from pos " << ipos;
		throw miutil::miTimeParseException(ost.str(), ipos, false);
	}
	
	buf=str.substr(ipos, nChar);
	
	if(expectNumber){
		string::size_type i=0;
		
		for(; i<buf.length() && isdigit(buf[i]); i++);
		
		if(i<buf.length()){
			ostringstream ost;
		
			ost << "Expecting a number from pos: " << ipos << " to "
				<< ipos+nChar << " got (" << buf << ")!" ; 
			throw miutil::miTimeParseException(ost.str(), ipos, false);
		}
		
		number=atoi(buf.c_str());
	}
	
	return ipos+nChar;
}

void
computeNearestTime(int times[], 
				   const miutil::miTime &nt, 
				   miutil::miTime &time_,
				   std::string::size_type pos)
{
	using namespace miutil;
	int i=Year;
	int first;
	int last;

	for(;i<TimeSpecEnd && times[i]==-1; i++);
		
	first=i;
		
	if(i==TimeSpecEnd) //Should not happend.
		throw miTimeParseException("Unexpected: No time information parsed!",
									pos, false);
										
	for(;i<TimeSpecEnd && times[i]!=-1; i++)
	last=i;
		
	if(i<TimeSpecEnd){
		for(;i<TimeSpecEnd && times[i]==-1; i++);
			
		if(i!=TimeSpecEnd)
			throw miTimeParseException("Unable to deduce a time to nearest time!",
										pos, false);
	}

	if(times[Second]==-1)
		times[Second]=0;
		
	if(times[Minute]==-1)
		times[Minute]=0;
		
	if(times[Hour]==-1)
		times[Hour]=0;
		
	miutil::miTime tmp(nt);
	time_=miTime(); //Undef state
	
	switch(first){
	case Second:
		if(times[Second]>tmp.sec())
			tmp.addMin(-1);

		time_=miTime(tmp.year(), tmp.month(), tmp.day(),
				     tmp.hour(), tmp.min(), times[Second]);		
		break;
	case Minute:
		if( times[Minute]>tmp.min() ||
		   (times[Minute]==tmp.min() && times[Second]>tmp.sec())
		  )
			tmp.addHour(-1);
			
		time_=miTime(tmp.year(), tmp.month(), tmp.day(),
				     tmp.hour(), times[Minute], times[Second]);		
		break;
	case Hour:
		if( times[Hour]>tmp.hour() ||
		   (times[Hour]==tmp.hour() && times[Minute]>tmp.min()) ||
		   (times[Hour]==tmp.hour() && times[Minute]==tmp.min()
		   	  && times[Second]>tmp.sec())
		  )
			tmp.addDay(-1);
		
		time_=miTime(tmp.year(), tmp.month(), tmp.day(),
				     times[Hour], times[Minute], times[Second]);			
		break;
	case Day:{
		int y=tmp.year();
		int m=tmp.month();
		
		if(times[Day]>tmp.day()   ||
		   (times[Day]==tmp.day() && times[Hour]>tmp.hour()) ||
		   (times[Day]==tmp.day() && times[Hour]==tmp.hour() 
		   	  && times[Minute]>tmp.min())                    ||
		   (times[Day]==tmp.day() && times[Hour]==tmp.hour() 
		   	  && times[Minute]==tmp.min() && times[Second]>tmp.sec())
		  ){
			m--;
			if(m<1){
				y--;
				m=12;
			}
		}
			
		time_=miTime(y, m, times[Day],
				     times[Hour], times[Minute], times[Second]);		
		}
		break;
	case Month:{
		int y=tmp.year();
		
		if( times[Month]>tmp.month()                           || 
		   (times[Month]==tmp.month() && times[Day]>tmp.day()) ||
		   (times[Month]==tmp.month() && times[Day]==tmp.day()
		     && times[Hour]>tmp.hour())                         ||
		   (times[Month]==tmp.month() && times[Day]==tmp.day() 
		   	 && times[Hour]==tmp.hour() && times[Minute]>tmp.min()) ||
		   (times[Month]==tmp.month() && times[Day]==tmp.day() 
		   	 && times[Hour]==tmp.hour() && times[Minute]==tmp.min()
		   	 && times[Second]>tmp.sec())
		   )
			y--;			
			
		time_=miTime(y, times[Month], times[Day],
				     times[Hour], times[Minute], times[Second]);
		}
				
		break;
	case Year:
		if(times[Year]<100){
			if(times[Year]>50)
				times[Year]+=1900;
			else
				times[Year]+=2000;
		}
	
		if(!miTime::isValid(times[Year], times[Month], times[Day], 
				  	 	   times[Hour], times[Minute], times[Second])){
			ostringstream ost;
			ost << "Not a valid time: "
				<< times[Year] << "-" << times[Month] << "-" << times[Day]
				<< " " << times[Hour] << ":" << times[Minute] << ":" << times[Second];
			
			throw miTimeParseException(ost.str(), pos, false);
		}


		time_=miTime(times[Year], times[Month], times[Day],
				     times[Hour], times[Minute], times[Second]);
		break;
	default:
		throw miTimeParseException("Unexpected No \"first\" timeSpec!",
											pos, false);
	}
	
	
			
}

}
