/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: CommandQue.cc,v 1.7.2.2 2007/09/27 09:02:33 paule Exp $                                                       

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
#include <stdlib.h>
#include <boost/thread/xtime.hpp>
#include "CommandQue.h"




bool       
dnmi::thread::
CommandBase::
execute()
{
  return executeImpl();
}


void       
dnmi::thread::
CommandBase::
debugInfo(std::ostream &info)const
{
  info << "No implementation for <CommandBase::debugInfo>.\n"; 
} 

dnmi::thread::
CommandQue::
CommandQue(bool suspended_):
  suspended(suspended_)  
{
}

dnmi::thread::
CommandQue::
~CommandQue()
{
    Lock lock(m);;
    QueIterator it=que.begin();

    for(;it!=que.end(); it++)
	delete *it;
}

void 
dnmi::thread::
CommandQue::
post(CommandBase *command)
{
    Lock lock(m);

    if(suspended)
      throw QueSuspended();

    que.push_back(command);
}

void 
dnmi::thread::
CommandQue::
postAndBrodcast(CommandBase *command)
{
    Lock lock(m);

    if(suspended)
      throw QueSuspended();

    que.push_back(command);

    cond.notify_all();
 
}

dnmi::thread::CommandBase*
dnmi::thread::
CommandQue::
peek(int timeout)
{
  Lock lk(m);
  
  if(que.empty()){
    if(timeout==0){
	
      while(que.empty()){
	cond.wait(lk);
	
	if(suspended)
	  throw QueSuspended();
      }
    }else{
      boost::xtime xt;
      
      xtime_get(&xt, boost::TIME_UTC);
      xt.sec+=timeout;
      
      cond.timed_wait(lk, xt);
      
      if(suspended)
	throw QueSuspended();
      
      if(que.empty())
	return 0;
    }
  }
  CommandBase *tmp=que.front();  
  return tmp;
}

dnmi::thread::CommandBase*
dnmi::thread::
CommandQue::get(int timeout)
{
   Lock lk(m);

   if(que.empty()){
      if(timeout==0){

         while(que.empty()){
            cond.wait(lk);

            if( que.empty() && suspended )
               throw QueSuspended();
         }
      }else{
         boost::xtime xt;

         xtime_get(&xt, boost::TIME_UTC);
         xt.sec+=timeout;

         cond.timed_wait(lk, xt);

         if( que.empty() ) {
            if( suspended )
               throw QueSuspended();

            return 0;
         }
      }
   }

   CommandBase *tmp=que.front();
   que.pop_front();

   return tmp;
}

dnmi::thread::CommandBase*
dnmi::thread::
CommandQue::
remove(CommandBase *com)
{
  Lock lck(m);
  
  if(que.empty())
    return 0;

  QueIterator it=que.begin();
  
  for(; it!=que.end(); it++){
    if(*it==com){
      que.erase(it);
      return com;
    }
  }
  
  return 0;
}

void         
dnmi::thread::
CommandQue::
clear()
{
  Lock lck(m);

  que.clear();
}
 
std::list<dnmi::thread::CommandBase*>*
dnmi::thread::CommandQue::removeAll()
{
  std::list<CommandBase*> *toReturn;
  Lock lck(m);
  
  if(que.empty())
    return 0;

  
  QueIterator it=que.begin();
  
  try{
    toReturn=new std::list<CommandBase*>();
  }
  catch(...){
    return 0;
  }
 

  for(; it!=que.end(); it++){
    toReturn->push_back(*it);
  }
  
  que.clear();
  return toReturn;
}
bool      
dnmi::thread::
CommandQue::
empty()
{
     Lock lck(m);

     return que.empty();
}

int       
dnmi::thread::
CommandQue::
size()
{
    Lock lck(m);
    
    return que.size();
}


void      
dnmi::thread::
CommandQue::
brodcast()
{
    Lock lck(m);
    
    if(!que.empty())
	cond.notify_all();
}


void         
dnmi::thread::
CommandQue::
suspend()
{
  Lock lk(m);

  if(suspended)
    return;

  suspended=true;
  cond.notify_all();
}

void         
dnmi::thread::
CommandQue::
resume()
{
  Lock lk(m);

  if(!suspended)
    return;

  suspended=false;
  cond.notify_all();
}

void
dnmi::thread::
CommandQue::
signal()
{
  Lock lk(m);
  cond.notify_all();
}
