/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: dbdrivermgr.h,v 1.1.2.2 2007/09/27 09:02:25 paule Exp $                                                       

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
#ifndef __dnmi__db_dbdrivermgr_h__
#define __dnmi__db_dbdrivermgr_h__

#include <fileutil/dso.h>
#include <db/db.h>

namespace dnmi {
  namespace db {
    
    /**
     * \addtogroup dbabstraction
     *
     * @{
     */
    

    /**
     * \brief An abstact base class to be implemented by a specific plugin
     *        for a database server.
     *
     * DriverBase is a abstract class that must be implemented by
     * the driver for a given database engine.
     *
     */
     class DriverBase {
	  std::string errMsg;
	  
      protected:
	  void setErrMsg(const std::string &msg){ errMsg=msg; }
	  
      public:
	  DriverBase(){};
	  virtual ~DriverBase(){};
	  
	  /**
	   * \brief the driver id for database server.
	   *
	   * name returns a the driverId for this driver.
	   *
	   * \return driverId.
	   */
	  virtual std::string name()const=0;

	  /**
	   * \brief creates a connection to a the database server.
	   *
	   * creates a connection to the database. The returned conection must
	   * be released with releaseConnection.
	   *
	   * \return If a connection can't be created 0 is returned. Use getErr()
	   *         too get a error message.
	   */
	  virtual Connection* createConnection(const std::string &connect)=0;
	  
	  /**
	   * \brief relese the connection to the database server.
	   *
	   * release a connection that was created with createConnection. 
	   * The connection is NOT released if the driverId given with 
	   * 'name' don't match.
	   *
	   * \return false if the connection is not released, true otherwise.
	   */
	  virtual bool        releaseConnection(Connection *connect)=0;
	  
	  /**
	   * \brief An error message for the last error that has occured.
	   *
	   * \return An error message to the last error. 
	   */
	  std::string getErr()const{ return errMsg;}
      };
      
	
     /**
      * \brief DriverManager loads a plugin for a specific database server.
      *
      * DriverManager contains the information of all drivers that is in use 
      * by the application. It is used to load a driver for a given database 
      * engine so that it can be used by the application.
      *
      * It is also used to create connections to the databases and release
      * the connections after use. 
      */
      class DriverManager{
	  struct Driver{
	      DriverBase *driver;
	      void (*releaseDriverFunc)(dnmi::db::DriverBase*);
	      dnmi::file::DSO  *dso;
	      
	      Driver(DriverBase *driver_, 
		     void (*pf)(dnmi::db::DriverBase*),
		     dnmi::file::DSO *dso_)
		  :driver(driver_), releaseDriverFunc(pf), dso(dso_)
		  {
		  }
	      ~Driver(){
		  releaseDriverFunc(driver);
		  delete dso;
	      }
	  };
		  
	  typedef std::list<Driver*>                 DriverList;
	  typedef std::list<Driver*>::iterator       IDriverList;
	  typedef std::list<Driver*>::const_iterator CIDriverList;
	  
	  DriverList drivers;
	  std::string err;

      public:
	  DriverManager();
	  ~DriverManager();
	  
	  bool loadDriver(const std::string &driver, std::string &driverId);

	  /**
	   * Create a connection to a database through the driver given with 
	   * 'driverId'. The connect information is given in the string
	   * 'connect'.
	   *
	   * \param driverId Which driver shal we connect trough.
	   * \param connect  connect information. ie. database, host, passord etc.
	   * \return 0 if a connection could'nt be created, otherwise
	   *         the connection.
	   */
	  Connection *connect(const std::string &driverId, 
			      const std::string &connect);
	  

	  /**
	   * release a connection previously created with connect.
	   * 
	   * \return true if the connection is released, false otherwise.
	   */
	  bool        releaseConnection(Connection *con);

	  
	  /**
	   * get a list of all drivers that is known by the driverManager.
	   */
	  std::list<std::string> listDrivers()const;

	  /**
	   * getErr can  be used to get a error message if a method fails.
	   */
	  std::string getErr()const { return err;}
      };
      
      /** @} */ 
  }
}
#endif
