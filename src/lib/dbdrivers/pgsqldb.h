/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: pgsqldb.h,v 1.2.2.2 2007/09/27 09:02:26 paule Exp $                                                       

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
#ifndef __dnmi_db_drivers_pgsqldb_h__
#define __dnmi_db_drivers_pgsqldb_h__

#include <libpq-fe.h>
#include <kvdb/kvdb.h>
#include <kvdb/Pimpel.h>
#include <kvdb/dbdrivermgr.h>


namespace dnmi {
  namespace db {

    namespace drivers{
      /**
       * \addtogroup dbpgsql
       *
       * @{
       */


      /**
       * \brief Implements the the abstract DriverBase interface.
       *
       * \see dnmi::db::DriverBase
       */
	class PGDriver: public dnmi::db::DriverBase {
	public:
	    PGDriver();
	    virtual ~PGDriver();
	    
	    virtual std::string name()const { return "PostgreSQL"; }
	    virtual Connection* createConnection(const std::string &connect);
	    virtual bool        releaseConnection(Connection *connect);
	};


	/**
	 * \brief Implements the the abstract Result interface.
	 *
	 * \see dnmi::db::Result
	 */
	class PGResult :  public dnmi::db::Result{
	    PGResult();
	    PGResult(const PGResult &);
	    PGResult& operator=(const PGResult &);
	    
	    PGresult *res;
	    int      nTuples;
	    int      tupleIndex;
	    int      nFields;
	    //dnmi::db::DRow data;
	    
	    friend class PGConnection;
	    PGResult(PGresult *res_);
	    
	    virtual void nextImpl();
	public:
	    ~PGResult();
	    
	    
	    bool                hasResult()const; 
	    
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
	class PGConnection : public dnmi::db::Connection{
	    PGConnection();
	    PGConnection(const PGConnection &);
	    PGConnection& operator=(const PGConnection &);
	    
	    friend class PGDriver;
	    PGconn *con;
	    std::string errMsg;
	    
	    PGConnection(const std::string &connect, 
			 const std::string &driverId);
	public:
	    ~PGConnection();

	    virtual bool isConnected(); 
	    virtual bool tryReconnect();
	    
	    virtual void beginTransaction();
	    virtual void endTransaction();
	    virtual void rollBack();
	    
	    virtual void   exec(const std::string &stmt);
	    virtual Result *execQuery(const std::string &stmt);
	    
	    std::string lastError()const;
	    virtual std::string esc( const std::string &stringToEscape )const;
	};
	
	class PGPimpel : public dnmi::db::priv::Pimpel
	{
	   PGConnection *con;

	public:
	   PGPimpel();
	   virtual void createSavepoint( const std::string &name );
	   virtual void rollbackToSavepoint( const std::string &name );
	   virtual void releaseSavepoint( const std::string &name );

	   void beginTransaction(dnmi::db::Connection::IsolationLevel isolation);
	   virtual void perform( dnmi::db::Connection *con,
	                         dnmi::db::Transaction &transaction, int retry,
	                         dnmi::db::Connection::IsolationLevel isolation);
	};

    }
    
    /** @} */
  }
}

#endif
