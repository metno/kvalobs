/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: HtmlStream.cc,v 1.3.6.2 2007/09/27 09:02:32 paule Exp $                                                       

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
#include <milog/HtmlStream.h>
#include <milog/LogStream.h>
#include <milog/HtmlLayout.h>

namespace milog{

    void 
    HtmlStream::write(const std::string &message)
    {
	if(fd){
	    fwrite(message.c_str(), message.length(), 1, fd);
	}
    }
	
    HtmlStream::HtmlStream():fd(0)
    {
	try{
	    priv::HtmlLayout *l=new priv::HtmlLayout();
	    layout(l);
	}
	catch(...){
	}
    }
    
    HtmlStream::HtmlStream(const std::string &fname):
	fname_(fname), fd(0)
    {
	try{
	    priv::HtmlLayout *l=new priv::HtmlLayout();
	    layout(l);
	}
	catch(...){
	}

	if(!open(fname))
	    fname_.erase();
    }
    
    HtmlStream::~HtmlStream()
    {
       close();
    }

    bool
    HtmlStream::open(const std::string &fname)
    {
	close();

	fd=fopen(fname.c_str(), "w");
	
	if(!fd){
	    fname_.erase();
	    return false;
	}
	
	fname_=fname;

	fputs("<html><body>\n", fd);

	return true;

    }
    
    void 
    HtmlStream::close()
    {
	if(fd){
	    fputs("</body></html>\n", fd);
	    fclose(fd);
	    fd=0;
	}
    }
    
};

