/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: dummysqldb.h,v 1.1.2.3 2007/09/27 09:02:26 paule Exp $                                                       

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
#ifndef __dnmi_db_drivers_dummysqldb_h__
#define __dnmi_db_drivers_dummysqldb_h__

#include <db/db.h>
#include <db/dbdrivermgr.h>


namespace dnmi {
	namespace db {
		namespace drivers{
      /**
       * \addtogroup dummy
       *
       * @{
       */

      /**
       * \brief Implements the the abstract DriverBase interface.
       *
       * \see dnmi::db::DriverBase
       */
		class DummyDriver: public dnmi::db::DriverBase {
		public:
	    	DummyDriver();
	    	virtual ~DummyDriver();
	    
	    	virtual std::string name()const { return "DummySQL"; }
	    	virtual Connection* createConnection(const std::string &connect);
	    	virtual bool        releaseConnection(Connection *connect);
		};


		/**
	 	 * \brief Implements the the abstract Result interface.
		 *
	 	 * \see dnmi::db::Result
	 	 */
		class DummyResult :  public dnmi::db::Result{
	    	DummyResult();
	    
	    	virtual void nextImpl();
		public:
	   	~DummyResult();
	    
	    	virtual int         fields()const;
	    	virtual std::string fieldName(int index)const;
	    	virtual int         fieldIndex(const std::string &fieldName)const;
	    	virtual FieldType   fieldType(int index)const;
	    	virtual FieldType   fieldType(const std::string &fieldName)const;
	    	virtual int         fieldSize(int index)const;
	    	virtual int         fieldSize(const std::string &fieldName)const;
	    	virtual int         size()const;
	    	virtual bool        hasNext()const;
		};
	
		/**
	 	 * \brief Implements the the abstract Connection interface.
	 	 *
	 	 * \see dnmi::db::Connection
	 	 */
		class DummyConnection : public dnmi::db::Connection{
	    	DummyConnection();
	    	DummyConnection(const DummyConnection &);
	    	DummyConnection& operator=(const DummyConnection &);
	    
		public:
	    	~DummyConnection();

	    	virtual bool isConnected(); 
	    	virtual bool tryReconnect();
	    
	    	virtual void beginTransaction();
	    	virtual void endTransaction();
	    	virtual void rollBack();
	    
	    	virtual void   exec(const std::string &stmt);
	    	virtual Result *execQuery(const std::string &stmt);
	    
	    	std::string lastError()const;
		};
	
		}
    
   /** @} */
  	}
}

#endif
