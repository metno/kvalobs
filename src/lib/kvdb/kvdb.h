/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: db.h,v 1.1.2.2 2007/09/27 09:02:25 paule Exp $                                                       

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
#ifndef __dnmi_db_db_h__
#define __dnmi_db_db_h__

#include <exception>
#include <string>
#include <vector>
#include <list>
#include <kvdb/transaction.h>


/**
* \namespace dnmi::db
* \brief The namespaece that the database abstraction is in.
*/
namespace dnmi {
namespace db {

/**
* \defgroup dbabstraction Database abstraction
*
* Kvalobs implements a common database abstraction. The abstraction implements
* a plugin technology that is used to implement drivers for a specific database
* server.
*
* @{
*/

/**
* \brief Base class for all exceptions.
*
* SQLException is a base class that all other exceptions in
* in the database abstraction is derived from.
*/
class SQLException : public std::exception{
   std::string reason;
   std::string errorCode_;
public:
   explicit SQLException(const std::string &reason_)
   : reason(reason_){}
   explicit SQLException(const std::string &reason_, const std::string &errorCode )
      : reason(reason_), errorCode_(errorCode){}

   virtual ~SQLException()throw(){};

   std::string errorCode()const { return errorCode_; }

   const char *what()const throw()
	            { return reason.c_str();}
};

/**
* \brief Feature not supported.
*
* SQLNorSupported is thrown if we try to
* do something that is not suported by the driver.
*/
class SQLNotSupported : public SQLException{
public:
   explicit SQLNotSupported(const std::string &reason_)
   :SQLException(reason_){}
   explicit SQLNotSupported(const std::string &reason_, const std::string &errorCode )
      :SQLException(reason_, errorCode){}
   explicit SQLNotSupported()
   :SQLException("Not supported!"){}


   ~SQLNotSupported()throw(){}
};



/**
* \brief Trying to insert a duplicate row.
*
* SQLDuplicate is thrown if we try to insert a row
* into a table that not allows duplicates.
*/
class SQLDuplicate : public SQLException{
public:
   explicit SQLDuplicate(const std::string &reason_)
   :SQLException(reason_){}

   explicit SQLDuplicate(const std::string &reason_, const std::string &errorCode )
      :SQLException(reason_, errorCode ){}
   ~SQLDuplicate()throw(){}
};

/**
* \brief Not connected to the database server.
*
* SQLNotConnected is thrown if we are not connected to the database
* server.
*/
class SQLNotConnected : public SQLException{
public:
   explicit SQLNotConnected(const std::string &reason_)
   :SQLException(reason_){}

   explicit SQLNotConnected(const std::string &reason_, const std::string &errorCode )
   :SQLException(reason_, errorCode ){}

   ~SQLNotConnected()throw(){};
};


/**
* \brief the database server is busy, try again later.
*
*SQLBusy is thrown if the database can't execute
*the function. But it may succeed later if you try again.
*/
class SQLBusy : public SQLException{
public:
   explicit SQLBusy(const std::string &reason_)
   :SQLException(reason_){}

   explicit SQLBusy(const std::string &reason_, const std::string &errorCode )
      :SQLException(reason_, errorCode ){}
   explicit SQLBusy()
   :SQLException("SQLBusy"){}


   ~SQLBusy()throw(){}
};

/**
* \brief A transaction is aborted.
*
* SQLAborted is thrown if a transaction is aborted. The
* transaction must either be rolled back or committed.
*/

class SQLAborted : public SQLException{
public:
   explicit SQLAborted(const std::string &reason_)
   :SQLException(reason_){}
   explicit SQLAborted(const std::string &reason_, const std::string &errorCode )
      :SQLException(reason_, errorCode ){}

   ~SQLAborted()throw(){};
};


/**
* \brief A transaction is aborted due to serialize error.
*
* SQLSerializeError is thrown if a transaction is aborted due to serialize
* error. This may happen if the transaction isolation level is set to
* SERIALIZABLE or an SELECT ... FOR UPDATE is issued. The transaction must
* be rolled back and restarted.
*/

class SQLSerializeError: public SQLException{
   bool deadLockDetected_;
public:
   explicit SQLSerializeError(const std::string &reason_)
   :SQLException(reason_), deadLockDetected_( false ){}
   explicit SQLSerializeError(const std::string &reason_,
                              const std::string &errorCode, bool deadLockDetected=false )
      :SQLException(reason_, errorCode ), deadLockDetected_( deadLockDetected ){}

   bool deadLockDetected() const { return deadLockDetected_; }
   ~SQLSerializeError()throw(){};
};



///Is the base class of DRow, that represent a row in the reult set.
typedef std::vector<std::string>                 Row;

/// A iterator to iterate trough the rows in the result set.
typedef std::vector<std::string>::iterator       IDRow;

/// A const iterator to iterate trough the rows in the result set.
typedef std::vector<std::string>::const_iterator CIDRow;

class Result;

/**
* \brief a row in the result set.
*/
class DRow : Row {

   DRow& operator=(const DRow &);
   DRow():Row(){
   }

   //	  friend class DRowP;

   friend class Result;
   Result *result;
   DRow(int size):Row(size){}
   void setResult(Result *r);

public:

   /**
   * \brief get the i'th field in a row.
   *
   * Return the i'th field in a row. i must be in the
   * range [0, fields()>.
   * \exception SQLException on range error.
   */
   std::string& operator[](int i);


   /**
   * \brief get the i'th field in a row.
   *
   * Return the i'th field in a row. i must be in the
   * range [0, fields()>.
   * \exception SQLException on range error.
   */
   std::string& operator[](const std::string &fieldName);

   /**
   * \brief get a iterator to the first field in the row.
   *
   * \return CIDRow, an iterator to the first field in a row.
   */
   CIDRow begin()const;

   /**
   * \brief get a iterator to the end of fields in a row.
   *
   * \return CIDRow, an iterator to the end of the row.
   */
   CIDRow end()const;

   ///Number of fields in a row.
   int      fields()const;

   /**
   * \brief get the field name to the i'th field in a row.
   *
   * The index must be in the range [0, fields()>.
   * \exception  SQLException if i is out of range.
   */
   std::string fieldName(int i)const;

   /**
   * \brief Get a list of the field names in a row.
   *
   * \return a list of field names.
   */
   std::list<std::string> getFieldNames()const;
   int size()const{ return Row::size();}
};

/**
* \brief The field type of a column in the result set.
*/
enum FieldType {Integer, ///An integer.
   Float,   ///An float.
   Text,    ///A text with no length limit.
   VarText, ///A text with a max length.
   Date,    ///A date.
   TimeStamp, ///A timestamp.
   Blob       ///A blob.
};

class Result;

/**
* \brief A connection to the database server.
*
* The connection class represent a connection to the database.
* All communication to a database server is trough a connection.
*/
class Connection{
protected:
   std::string driverId;
   void *pimpel;

   Connection(const std::string &driverId_):driverId(driverId_),pimpel( 0 ){}

public:
   typedef enum{ SERIALIZABLE, REPEATABLE_READ, READ_COMMITTED, READ_UNCOMMITTED } IsolationLevel;

   ///No public constructor
   virtual ~Connection(){};

   ///The driver id for the database engine that we are using.
   std::string getDriverId()const{ return driverId;}

   ///Are we connected to the database server.
   virtual bool isConnected()=0;
   /**
   * tryRecconect will try to reconnect to the database server
   * if have lost the connection.
   *
   * \return true if we the reconnection succeeded. False otherwise.
   */
   virtual bool tryReconnect()=0;

   /**
    * Create a savepoint with the given name.
    *
    * The connection must be in a transaction before a call to savepoint.
    *
    * @param name The name of the savepoint to create.
    * @exception SQLException
    */
   void createSavepoint( const std::string &name );

   /**
    * rollback to a previously created savepoint with name.
    *
    * @param name The name of the savepoint to rollback to.
    * @exception SQLException
    *  @see beginTransaction endTransaction createSavepoint rollbackToSavepoint releaseSavepoint
    */

   void rollbackToSavepoint( const std::string &name );

   /**
    * remove to a previously created savepoint with name.
    *
    * @param name The name of the savepoint to remove.
    * @exception SQLException
    *  @see beginTransaction endTransaction createSavepoint rollbackToSavepoint releaseSavepoint
    */
   void releaseSavepoint( const std::string &name );

   /**
    * beginTransaction starts a database transaction with the given IsolationLevel.
    *
    * @param isolation The transaction isolation level to run at.
    * @exception SQLException if an database error was  encountered.
    * @see beginTransaction endTransaction createSavepoint rollbackToSavepoint releaseSavepoint
    */
   void beginTransaction( IsolationLevel isolation );

   /**
   * beginTransaction starts a database transaction.
   * \exception SQLException if an database error was  encountered.
   * \see endTransaction
   * \see rollBack beginTransaction endTransaction createSavepoint rollbackToSavepoint releaseSavepoint
   */

   virtual void beginTransaction()=0;

   /**
   * endTransaction  database transaction  that was previous started
   * with beginTransaction.
   *
   * \exception SQLException if an database error was  encountered.
   * \see beginTransaction.
   * \see rollBack beginTransaction endTransaction createSavepoint rollbackToSavepoint releaseSavepoint
   */
   virtual void endTransaction()=0;

   /**
   * rollBack undo all changes made to the database since the start of
   * the transaction.
   *
   * \exception SQLException if an database error was  encountered.
   * @see beginTransaction endTransaction createSavepoint rollbackToSavepoint releaseSavepoint.
   */
   virtual void rollBack()=0;

   /**
   * execQuery sends a SQL query to the database server.
   * The function return the result of the query.
   * execQuery throws  SQLException on error. The caller
   * must delete the result when not needed any longer.
   *
   * \param SQLstmt the SQL statement.
   * \return The  result set.
   *
   * \exception SQLNotConnected, SQLAborted, SQLBusy, SQLDuplicate,
   * SQLSerializeError or SQLExcetion on error.
   */
   virtual Result *execQuery(const std::string &SQLstmt)=0;

   /**
   * Use exec to send an SQL statement to the server that NOT returns
   * a result.
   * exec throws SQLException on error.
   *
   * \param SQLstmt The SQL statement.
   *
   * \exception SQLNotConnected, SQLAborted, SQLBusy,
   * SQLDuplicate, SQLSerializeError or SQLExcetion on error.
   */
   virtual void exec(const std::string &SQLstmt)=0;

   virtual std::string lastError()const=0;

   /**
   * Correctly escape strings to insert into the database.
   *
   * \exception SQLException if an error occured.
   */
   virtual std::string esc( const std::string &stringToEscape )const=0;

   /**
    * Performs an transaction.
    *
    * If an transaction is attempted retry times without success the
    * transactions onMaxRetry is called. The default action to
    * onMaxRetry is to throw SQLException with a message of the last
    * error.
    *
    * @param transaction
    * @param retry Number of times to retry a transaction before giving up.
    * @exception SQLException. May throw a sub class of SQLException.
    */
   void perform( dnmi::db::Transaction &transaction, int retry = 3,
                 IsolationLevel isolation=Connection::SERIALIZABLE );
};



/**
* \brief The result set from a query.
*
* If you issue a command to the database that returns a result as
* rows, an instance of this class is returned.
*/
class Result{
   friend class DRow;
protected:
   DRow data;

   virtual void nextImpl()=0;

public:
   Result(int size):data(size){}
   virtual ~Result(){};
   ///no public constructor

   ///Number of fields in a row.
   virtual int         fields()const=0;

   ///The name of the field with index i. The indices start with 0.
   ///\exception SQLException if i is out of range. ie i<0 or i>=fields().
   virtual std::string fieldName(int i)const=0;


   virtual std::list<std::string> getFieldNames()const;


   ///Return the index of the field with name fieldName.
   ///\exception SQLException if the fieldName does'nt exist.
   virtual int         fieldIndex(const std::string &fieldName)const=0;

   ///The type of the field.
   virtual FieldType   fieldType(int index)const=0;
   virtual FieldType   fieldType(const std::string &fieldName)const=0;

   /**
   * Returns the size in bytes of the field associated with the given
   * field index i the table.
   *
   * \param index the field index. Indices start with 0.
   * \return the size in bytes. -1 if the field is variable length.
   * \exception SQLException with error.
   */
   virtual int fieldSize(int index)const=0;


   /**
   * Returns the size in bytes of the field associated with the given
   * fieldName i the table.
   *
   * \param fieldName the name of the field. Indices start with 0.
   * \return the size in bytes. -1 if the field is variable length.
   * \exception SQLException with error.
   */
   virtual int fieldSize(const std::string &fieldName)const=0;

   ///Number of rows in the result set.
   virtual int  size()const=0;

   /**
   * \brief Is there more rows in the result set.
   * \return true if there is more rows in the result set and
   *         false if we are at the end of the result set.
   */
   virtual bool        hasNext()const =0;


   /**
   * \brief the next row in the result set.
   *
   * \return DRow the next row.
   * \exception SQLException with error. It is an error to call
   *            next after hasNext has returned false.
   */
   DRow &next();
};

/** @} */
}


}




#endif
