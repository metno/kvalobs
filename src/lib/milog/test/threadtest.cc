/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: threadtest.cc,v 1.3.6.1 2007/09/27 09:02:32 paule Exp $                                                       

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
#include <milog.h>
#include <boost/thread/thread.hpp>
#include <unistd.h>

using namespace milog;

class TestThread{
  std::string name;
public:
  TestThread(const std::string &name_):name(name_){}
  
  void operator()(){
    LogContext ctx(name);
    
    LOGDEBUG("Thread " << "created!");

    func();
  }

  void func(){
    std::stringstream  os;
    LogContext ctx("func");
    int r;

    for(int i=0; i<10; i++)
    {
      os << i;
      LogContext ctx1(os.str());
      os.str("");
      r=rand();
      
      LOGDEBUG("in func: " << i );

      usleep(r%1000000);
    }
  }
};
    

int
main(int argn, char **argv)
{
  LogContext ctx("main");

  LOGWARN("Warn 1");

  
  TestThread t1("T1");
  TestThread t2("T2");
  TestThread t3("T3");
  
  boost::thread bt(t1);
  boost::thread bt2(t2);
  boost::thread bt3(t3);

  bt.join();
  bt2.join();
  bt3.join();
  

  LOGINFO("After join!");

}
