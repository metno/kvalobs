/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: dbdrivermgr.cc,v 1.6.2.2 2007/09/27 09:02:25 paule Exp $                                                       

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
#include <fileutil/dso.h>
#include "dbdrivermgr.h"
#include <sstream>
#include "kvdb.h"


using namespace std;



                         
dnmi::db::
DriverManager::
DriverManager()
{
}

dnmi::db::
DriverManager::
DriverManager( const std::string &appName_ )
   : appName( appName_ )
{
}


dnmi::db::
DriverManager::
~DriverManager()
{
  IDriverList it=drivers.begin();

  for( ;it!=drivers.end(); it++)
    delete *it;

}

void
dnmi::db::
DriverManager::
setAppName( const std::string &appName_ )
{
   appName = appName_;
}

std::string
dnmi::db::
DriverManager::
getAppName( ) const
{
   return appName;
}

std::string
dnmi::db::
DriverManager::
fixDriverName( const std::string &driver_ )
{
   string driver( driver_ );
   std::string dir( PKGLIB_DBDIR );

   size_t i;
   if( soVersion.empty() ) {
      soVersion=KVALOBSLIBS_SO_VERSION;
       i=soVersion.find_first_of( ":" );

      if( i != string::npos )
        soVersion.erase( i );

      soVersion.insert(0,".so.");
   }

   i=driver.find( ".so" );

   if( i != string::npos ) {
      size_t k=driver.find_first_not_of(".0123456789", i+3 );

      if( k == string::npos ) {
         driver.erase( i );
         driver += soVersion;
      }
   } else {
      driver += soVersion;
   }

   //Add path if needed.
   i = driver.find_first_of( "/" );

   if( i == string::npos ) {
      if( !dir.empty() && *dir.rbegin() != '/' )
         dir += "/";

      driver.insert( 0, dir );
   }

   return driver;
}

bool 
dnmi::db::
DriverManager::
loadDriver(const std::string &driver_,
	   std::string &driverId)
{
  using namespace dnmi::file;
  string driver( fixDriverName( driver_ ) );
  DriverBase *d;
  Driver *drv=0;
  DSO    *dso=0;

  driverId.clear();

  try{
    dso=new DSO( fixDriverName( driver ) );
  }catch(dnmi::file::DSOException &ex){
      err="Can't load driver <";
      err+=driver;
      err+=">\nDSO err: ";
      err+=ex.what();
      
      if(dso)
	delete dso;
      return false;
  }
  catch(...){
    err="Can't load driver <";
    err+=driver;
    err+=">  Out of memmory!";
    
    if(dso)
      delete dso;
      
    return false;
  }

  try{
      dnmi::db::DriverBase* (*createSQLDriver)();
      void (*releaseSQLDriver)(dnmi::db::DriverBase*);
      dnmi::db::DriverBase *p;
      
      releaseSQLDriver=(void(*)(dnmi::db::DriverBase*))(*dso)["releaseSQLDriver"];

      createSQLDriver=(dnmi::db::DriverBase* (*)())(*dso)["createSQLDriver"];
      d=createSQLDriver();
      
      if(!d)
	  throw std::bad_alloc();
      
      drv=new Driver(d, releaseSQLDriver, dso);
  }
  catch(dnmi::file::DSOException &ex){
    err="Can't load driver <";
    err+=driver;
    err+=">, ";
    err+=ex.what();
    
    if(d)
      delete d;
    
    delete dso;

    return false;
  }
  catch(...){
    err="Can't load driver <";
    err+=driver;
    err+=">  Out of memmory!";
     
    if(drv)
      delete drv;
    else if(d){
      delete d;
      delete dso;
    }
  
    return false;
  }

  driverId=drv->driver->name();

  IDriverList it=drivers.begin();

  for( ;it!=drivers.end(); it++){
    if((*it)->driver->name()==driverId){
      delete drv;
      
      return true;
    }
  }
    
  drivers.push_back(drv);
  return true;
}

dnmi::db::Connection* 
dnmi::db::
DriverManager::
connect(const std::string &driverId, 
	const std::string &connect_ )
{
   std::string connect( connect_ );
  dnmi::db::Connection *con;
  
  IDriverList it=drivers.begin();

  for( ;it!=drivers.end(); it++){
      if((*it)->driver->name()==driverId){
      break;
    }
  }

  if(it==drivers.end()){
      //ERROR: Possible race condition.
    err="No driver named <"+driverId+">!";
    return 0;
  }
  
  if( driverId == "PostgreSQL" && !appName.empty() )
     connect += " application_name='" + appName + "'";

  con=(*it)->driver->createConnection(connect);

  if(!con){
      //ERROR: Possible race condition.
    err="Can't connect: "+(*it)->driver->getErr();
    return 0;
  }
  
  return con;
}


bool        
dnmi::db::
DriverManager::
releaseConnection(Connection *con)
{
    IDriverList it=drivers.begin();
    
    for( ;it!=drivers.end(); it++){
	if((*it)->driver->name()==con->getDriverId()){
	    break;
	}
    }
    
    if(it==drivers.end()){
	std::stringstream ost; 
	ost << "ERROR: dnmi::db::DriverManager: No driver named <" 
	    << con->getDriverId() << ">!";
	
	//ERROR: Possible race condition.
	err=ost.str();
	
	return false;
    }
           
    return (*it)->driver->releaseConnection(con);
}

std::list<std::string> 
dnmi::db::
DriverManager::
listDrivers()const
{
    std::list<std::string> list;
    CIDriverList it=drivers.begin();
    
    for( ;it!=drivers.end(); it++)
	list.push_back((*it)->driver->name());
    
    return list;
}
    
