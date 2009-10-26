/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: FDLogStream.cc,v 1.7.6.3 2007/09/27 09:02:31 paule Exp $                                                       

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
#ifdef SMHI_LOG

#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h> 
#include <ctype.h>
#include <milog/FDLogStream.h>
#include <milog/StdLayout.h>
#include <set>
#include <sstream>
#include <iostream>
using namespace std;

namespace{

    /**
     * splitFileName splitter filename opp i en path komponent og
     * et filnavn. Hvis filename ikke er en absolutt sti til filenavnet
     * brukes funksjonen getcwd for � forme path.
     */
    bool 
    splitFileName(const std::string &filename, 
		  std::string       &fname, 
		  std::string       &path);    

    bool 
    checkFormat(const char *buf, const char *name, const char *ext);
    typedef std::set<std::string, std::less<std::string> > TDirSet;
    typedef std::set<std::string, std::less<std::string> >::iterator ITDirSet;
}

void 
milog::FDLogStream::write(const std::string &message)
{
	
	// this should always bee true
	if(timeStamp_< miTime::nowTime())
		logRotate();

    if(fd){
	fwrite(message.c_str(), message.length(), 1, fd);
	fflush(fd);
	timeStamp_=miTime::nowTime();
    }
}
 
milog::FDLogStream::FDLogStream()
: nRotate_(DAY), timeFormat_(DEFAULT_DAY_FORMAT), fd(0), timeStamp_(miTime::nowTime())
{
  try{
    StdLayout *l=new StdLayout();
    layout(l);
  }
  catch(...){
  }
}

milog::FDLogStream::FDLogStream(const int & nRotate,
		   const miString & timeFormat)
    :timeStamp_(miTime::nowTime()), fd(0)
{

    nRotate_=nRotate;
    timeFormat_=timeFormat;
    
    try{
	StdLayout *l=new StdLayout();
	layout(l);
    }
    catch(...){
    }
}


milog::FDLogStream::FDLogStream(Layout *layout, 
			      const int & nRotate,
		   const miString & timeFormat)
  :LogStream(layout), timeStamp_(miTime::nowTime()), fd(0)
{

    nRotate_=nRotate;
    timeFormat_=timeFormat;
}

milog::FDLogStream::FDLogStream(const std::string &fname,   
			      const int & nRotate,
		   const miString & timeFormat)
  :fname_(fname), fd(0), timeStamp_(miTime::nowTime())
{

    nRotate_=nRotate;
    timeFormat_=timeFormat;

    if(open(fname)){
	try{
	    milog::StdLayout *l=new milog::StdLayout();
	    layout(l);
	}
	catch(...){
	    fclose(fd);
	}
    }
}

milog::FDLogStream::~FDLogStream()
{
  if(fd)
    fclose(fd);
}

bool 
milog::FDLogStream::open(const std::string &fname)
{
    string fileName;
    string pathName;
    string::size_type i;
    struct stat   statbuf;
    
    if(fd){
	fname_.erase();
	fclose(fd);
    }
    
    fd=fopen(fname.c_str(), "a");
    
    if(!fd)
	return false;


    if(stat(fname.c_str(), &statbuf)<0){
		// dont change time stamp
    }else{
		// get last time something was logged
		timeStamp_ = miTime((time_t)statbuf.st_mtime);
    }
    
    fname_=fname;
    return true;
}

void 
milog::FDLogStream::close()
{
    if(fd){
	fclose(fd);
	fd=0;
    }
}

bool
milog::FDLogStream::logRotate()
{
    ostringstream ostNew; 
    ostringstream ostOld; 
    std::string   fname(fname_);


    if(!fd)
      return false;
    
    if(fd==stderr)
      return true;

    // Check if its time to rotate

	miTime now = miTime::nowTime();
	
	switch (nRotate_)
	{
	case SECOND:
		if ((timeStamp_.year() == now.year())
			&& (timeStamp_.month() == now.month())
			&& (timeStamp_.day() == now.day()) && (timeStamp_.hour() == now.hour()) && (timeStamp_.min() == now.min()) && (timeStamp_.sec() == now.sec()))
			return false;
		ostNew << fname << "." << timeStamp_.format(timeFormat_);
		break;
	case MINUTE:
		if ((timeStamp_.year() == now.year())
			&& (timeStamp_.month() == now.month()) && (timeStamp_.day() == now.day()) && (timeStamp_.hour() == now.hour()) && (timeStamp_.min() == now.min()))
			return false;
		ostNew << fname << "." << timeStamp_.format(timeFormat_);
		break;
	case HOUR:
		if ((timeStamp_.year() == now.year()) && (timeStamp_.month() == now.month()) && (timeStamp_.day() == now.day()) && (timeStamp_.hour() == now.hour()))
			return false;
		ostNew << fname << "." << timeStamp_.format(timeFormat_);
		break;
	case DAY:
	default:
		if ((timeStamp_.year() == now.year()) && (timeStamp_.month() == now.month()) && (timeStamp_.day() == now.day()))
			return false;
		ostNew << fname << "." << timeStamp_.format(timeFormat_);
		break;
	}
	
	ostOld << fname;

	fclose(fd);
    
    fd=0;

	rename(ostOld.str().c_str(), ostNew.str().c_str());

    open(fname);

    return true;
}




namespace{
    bool 
    checkFormat(const char *buf, const char *name, const char *ext)
    {
	const char *p;
	int  i;
	
	if(!buf || !name || !ext)
	    return false;
	
	if(strncmp(buf, name, strlen(name))!=0){
	    //    std::cout << "1\n";
	    return false;
	}
	
	p=&buf[strlen(name)];
	
	if(*p=='\0' || *p!='_'){
	    // std::cout << "2\n";
	    return false;
	}

	p++;
	
	for(i=0;*p!='\0' && i<10 && isdigit(*p); i++, p++);
	
	if(i!=10){
	    // std::cout << "3\n";
	    return false;
	}
	
	if(strcmp(p, ext)!=0){
	    //std::cout << "4\n";
	    return false;
	}
	
	return true;
    }

    bool 
    splitFileName(const std::string &filename, 
		  std::string       &fname, 
		  std::string       &path)
    {
	std::string::size_type i;
	std::string::size_type start;
	char buf[PATH_MAX+1];
	char *p;
	
	fname.erase();
	path.erase();
	
	if(filename.empty())
	    return false;
	
	i=filename.find_last_of('/');
	
	if(i!=string::npos){
	    fname=filename.substr(i+1);
	    path=filename.substr(0, i);
	    path+="/";
	}else{
	    fname=filename;
	}
	
	if(fname.empty())
	    return false;
	
	if(fname=="." || fname=="..")
	    return false;
	
	if(path.empty() || path[0]!='/'){
	    p=getcwd(buf, PATH_MAX);
	    buf[PATH_MAX]='\0';
	    
	    if(p){
		if(!path.empty())
		    path.insert(0, "/");
		
		path.insert(0, buf);
	    }
	}
	
	return true;
    }
}
#endif