/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: sqlite3db.cc,v 1.1.2.2 2007/09/27 09:02:26 paule Exp $                                                       

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
#include <unistd.h>
#include <sstream>
#include "sqlite3db.h"

#define DB_NOTOPEN -1
#define DB_NOMEM -2
#define DB_DRIVERMISMATCH -2

namespace {
int dataCallback(void *pArg, int argc, char **argv, char **columnNames);
}


using namespace std;
using namespace dnmi::db::drivers;

dnmi::db::drivers::SQLiteDriver::SQLiteDriver()
{
}

dnmi::db::drivers::SQLiteDriver::~SQLiteDriver()
{
}

dnmi::db::Connection* 
dnmi::db::drivers::SQLiteDriver::createConnection(const std::string &connect)
{
   SQLiteConnection *con;

   try{
      con=new SQLiteConnection(connect, name());
   }
   catch(...){
      setErrMsg( "SQLite: out of memmory." );
      return 0;
   }

   if(con->isConnected())
      return con;

   delete con;

   return 0;
}

bool        
dnmi::db::drivers::SQLiteDriver::releaseConnection(Connection *connect)
{
   if(connect->getDriverId() != name()){
      stringstream ost;
      ost << "ERROR: trying to release a connection with driverId <"
            << connect->getDriverId() << ">, but this driver har driverId <"
            <<name() << ">!\n";
      setErrMsg(ost.str());
      return false;
   }

   delete connect;
   return true;
}

dnmi::db::drivers::SQLiteConnection::SQLiteConnection(
      const std::string &connect,
      const std::string &driverId)
:Connection(driverId), con(0)
{
   SQLitePimpel *myPimpel=new SQLitePimpel();
   myPimpel->setConnect( connect );
   pimpel = myPimpel;

   int res = sqlite3_open( connect.c_str(), & con );

   if ( SQLITE_OK != res ) {
      if( con ) {
         myPimpel->setErrInfo( res, sqlite3_errmsg(con) );
         sqlite3_close(con);
      } else {
         errMsg = "SQLLite: NOMEM, when trying to connect to the data base '" + connect +"'.";
         myPimpel->setErrInfo( res,  errMsg );

      }

      con = 0;
   }


   if(con) {
      sqlite3_busy_timeout(con, 5000); //sets busy timeout to 5 second.
   }
}

dnmi::db::drivers::SQLiteConnection::~SQLiteConnection()
{
   if(con)
      sqlite3_close(con);

   if( pimpel )
      delete static_cast<SQLitePimpel*>( pimpel );
}


bool 
dnmi::db::drivers::SQLiteConnection::isConnected()
{
   if( ! pimpel ) {
      errMsg = "SQLite: Driver internal error (no pimpel)!";
      return false;
   }

   SQLitePimpel *myPimpel = static_cast<SQLitePimpel*>( pimpel );

   if(con==0) {
      errMsg = myPimpel->setErrInfo( DB_NOTOPEN );
      return false;
   }

   return true;
} 

bool 
dnmi::db::drivers::SQLiteConnection::tryReconnect()
{
   if( ! pimpel ) {
      errMsg = "SQLite: Driver internal error (no pimpel)!";
      return 0;
   }

   int res;
   SQLitePimpel *myPimpel = static_cast<SQLitePimpel*>( pimpel );
   string connect=myPimpel->getConnect();

   if( connect.empty() ) {
      errMsg = myPimpel->setErrInfo( DB_NOTOPEN );
      return false;
   }

   if( con ) {
      res = sqlite3_close( con );

      if( res != SQLITE_OK ) {
         errMsg = myPimpel->setErrInfo( res, sqlite3_errmsg( con ) );
         return false;
      }
   }

   res = sqlite3_open( connect.c_str(), & con );

   if ( res != SQLITE_OK  ) {
      if( con ) {
         errMsg = myPimpel->setErrInfo( res, string("SQLite: tryReconnect: ") + sqlite3_errmsg(con) );
         sqlite3_close( con );
      } else {
         errMsg = myPimpel->setErrInfo(DB_NOMEM,
                                       string("SQLite: NOMEM, when trying to reconnect to the data base '")
                                       + connect +"'." );
      }
      con = 0;
      return false;
   }

   return true;
}


void
dnmi::db::drivers::SQLiteConnection::beginTransaction()
{
   if( ! pimpel ) {
      errMsg = "SQLite: Driver internal error (no pimpel)!";
      throw SQLException( errMsg );
   }

   SQLitePimpel *myPimpel = static_cast<SQLitePimpel*>( pimpel );

   if( ! con ) {
      if( ! tryReconnect() )
         throw SQLNotConnected( errMsg );
   }

   bool reconnect=false;

   do {
      try {
         if( reconnect ) {
            if( ! tryReconnect() ) {
               throw SQLNotConnected( errMsg );
            }
         }

         exec("BEGIN IMMEDIATE");
         return;
      }
      catch( ... ) {
         if( reconnect )
            throw;

         reconnect = true;
      }
   }while( reconnect );
}

void 
dnmi::db::drivers::SQLiteConnection::endTransaction()
{
   exec("COMMIT");
}

void 
dnmi::db::drivers::SQLiteConnection::rollBack()
{
   exec("ROLLBACK");
}

void
dnmi::db::drivers::SQLiteConnection::exec(const std::string &query)
{
   int   sqliteRes;
   char  *msg=0;
   bool busy=false;

   if( ! pimpel ) {
      errMsg = "SQLite: Driver internal error (no pimpel)!";
      throw SQLException( errMsg );
   }

   SQLitePimpel *myPimpel = static_cast<SQLitePimpel*>( pimpel );

   if(!con) {
      if( ! tryReconnect() )
         throw SQLNotConnected( errMsg );
   }

   do {
      sqliteRes=sqlite3_exec(con, query.c_str(), 0, 0, &msg);

      if(msg){
         myPimpel->setErrMsg( msg );
         sqlite3_free(msg);
      }

      if( sqliteRes == SQLITE_OK ) {
         busy = false;
      } else {
         errMsg = myPimpel->setErrInfo( sqliteRes, errMsg );

         if(sqliteRes==SQLITE_CONSTRAINT){
            string::size_type i=errMsg.find("unique");

            if(i!=string::npos){
               errMsg = myPimpel->setErrInfo( sqliteRes, "SQLite: Duplicate (" + errMsg +")");
               throw SQLDuplicate( errMsg, myPimpel->getErrorCode() );
            }else{
               throw SQLException( errMsg, myPimpel->getErrorCode() );
            }
         }else if(sqliteRes==SQLITE_BUSY){
            busy = true;
            //throw SQLBusy("SQLite: " + errMsg, errorCode.str() );
         }else{
            throw SQLException(errMsg, myPimpel->getErrorCode());
         }
      }
   } while( busy );
}


dnmi::db::Result*
dnmi::db::drivers::SQLiteConnection::execQuery(const std::string &query)
{
   int    sqliteRes;
   char   *msg=0;
   SQLite::SQLiteData *data;
   bool busy=false;

   if( ! pimpel ) {
      errMsg = "SQLite: Driver internal error (no pimpel)!";
      return false;
   }

   SQLitePimpel *myPimpel = static_cast<SQLitePimpel*>( pimpel );

   if(!con) {
      if( ! tryReconnect() )
         throw SQLNotConnected( errMsg );
   }

   try{
      data = new SQLite::SQLiteData();
   }
   catch(...){
      errMsg = myPimpel->setErrInfo( DB_NOMEM );
      throw SQLException( errMsg );
   }

   do {
      busy = false;
      sqliteRes=sqlite3_exec(con, query.c_str(), dataCallback, data, &msg);


      if(msg){
         myPimpel->setErrMsg( msg );
         sqlite3_free(msg);
      }else
         myPimpel->setErrMsg( "SQLite: Unknown error!" );

      if(sqliteRes!=SQLITE_OK){
         delete data;
         //cerr << "ERROR: " << sqliteRes << endl;

         if(sqliteRes==SQLITE_BUSY){
            busy = true;
            //throw SQLBusy("SQLiteBusy: " + sMsg, errorCode.str());
         }else{
            errMsg = myPimpel->setErrInfo( sqliteRes );
            throw SQLException( errMsg, myPimpel->getErrorCode() );
         }
      }
   }while( busy );

   try{
      SQLiteResult *res=new SQLiteResult(data);
      return res;
   }
   catch(...){
      delete data;
      errMsg = myPimpel->setErrInfo( DB_NOMEM );
      throw SQLException( errMsg );
   }
}

std::string 
dnmi::db::drivers::SQLiteConnection::lastError()const
{
   return errMsg;
}

std::string 
dnmi::db::drivers::
SQLiteConnection::
esc( const std::string &stringToEscape )const
{
   char *buf = sqlite3_mprintf("%q", stringToEscape.c_str() );

   if( ! buf )
      throw SQLException("NOMEM: Cant escape the string.");

   try {
      string ret( buf );
      sqlite3_free( buf );

      return ret;
   }
   catch( ... ) {
      throw SQLException("NOMEM: Cant escape the string.");
   }
}


dnmi::db::drivers::SQLiteResult::SQLiteResult(SQLite::SQLiteData *data_):
        Result(data_->fieldNames.size()),sqlData(data_)
{ 
   nextData=sqlData->data.begin();
}

dnmi::db::drivers::SQLiteResult::~SQLiteResult()
{
   if(sqlData)
      delete sqlData;
}

bool               
dnmi::db::drivers::SQLiteResult::hasResult()const
{
   return sqlData->data.size()>0;
}

int                        
dnmi::db::drivers::SQLiteResult::fields()const
{
   return sqlData->fieldNames.size();
}

std::string 
dnmi::db::drivers::SQLiteResult::fieldName(int index)const
{
   if(index>=sqlData->fieldNames.size())
      throw SQLException("index out of range!");

   return sqlData->fieldNames[index];

}

int         
dnmi::db::drivers::SQLiteResult::fieldIndex(const std::string &fieldName)const
{

   for(int i=0; i<sqlData->fieldNames.size(); i++){
      if(sqlData->fieldNames[i]==fieldName)
         return i;
   }

   throw SQLException("No fields with name: " + fieldName);
}

dnmi::db::FieldType   
dnmi::db::drivers::SQLiteResult::fieldType(int index)const
{
   if(index>=sqlData->fieldNames.size())
      throw SQLException("index out of range!");
}

dnmi::db::FieldType   
dnmi::db::drivers::SQLiteResult::fieldType(const std::string &fieldName)const
{
}

int         
dnmi::db::drivers::SQLiteResult::fieldSize(int index)const
{
   if(index>=sqlData->fieldNames.size())
      throw SQLException("index out of range!");

   throw SQLException("SQLite: fieldSize is not implemented!");
}

int         
dnmi::db::drivers::SQLiteResult::fieldSize(const std::string &fieldName)const
{
   return fieldSize(fieldIndex(fieldName));
}

int         
dnmi::db::drivers::SQLiteResult::size()const
{
   return sqlData->data.size();
}

bool        
dnmi::db::drivers::SQLiteResult::hasNext()const
{
   if(nextData!=sqlData->data.end())
      return true;

   return false;
}

void
dnmi::db::drivers::SQLiteResult::nextImpl()
{
   if(nextData==sqlData->data.end())
      throw SQLException("No more data!");

   if(data.size()!=nextData->size())
      throw SQLException("INTERNAL ERROR: Incompatible data size!");

   for(int i=0; i<nextData->size(); i++){
      data[i]=(*nextData)[i];
   }

   nextData++;
}


void
dnmi::db::drivers::
SQLitePimpel::
beginTransaction(dnmi::db::Connection::IsolationLevel isolation)
{
   using namespace dnmi::db::priv;
   switch( isolation ) {
   case Connection::SERIALIZABLE:
      con->exec("BEGIN EXCLUSIVE");
      break;
   case Connection::READ_COMMITTED:
      con->exec("BEGIN TRANSACTION");
      break;
   case Connection::READ_UNCOMMITTED:
      con->exec("BEGIN TRANSACTION");
      break;
   case Connection::REPEATABLE_READ:
      con->exec("BEGIN TRANSACTION");
      break;
   }

}

dnmi::db::drivers::
SQLitePimpel::
SQLitePimpel() : dnmi::db::priv::Pimpel()
{
}


std::string
dnmi::db::drivers::
SQLitePimpel::
getConnect()const
{
   return connect;
}

void
dnmi::db::drivers::
SQLitePimpel::
setConnect( const std::string &connectString )
{
   connect = connectString;
}


std::string
dnmi::db::drivers::
SQLitePimpel::
getErrorCode()const
{
   return errorCode;
}

void
dnmi::db::drivers::
SQLitePimpel::
setErrorCode( int errCode )
{
   if( errCode < 0 ) {
      if( errCode == DB_NOTOPEN ) {
         errMsg = "SQLite: No database is open!";
         this->errorCode = "DB_NOTOPEN";
      } else if( errCode == DB_NOMEM ) {
         errMsg = "SQLite: out of memmory!";
         errorCode = "DB_NOMEM";
      }else if( errCode == DB_DRIVERMISMATCH ) {
         errMsg = "SQLite: Driver mismatch dected. (Internal error?!).";
         errorCode = "DB_DRIVERMISMATCH";
      } else {
         errMsg = "SQLite: UNKNOWN error!";
         errorCode = "UNKOWN";
      }
   } else {
      ostringstream o;
      o << errCode;
      errorCode = o.str();
   }

}

std::string
dnmi::db::drivers::
SQLitePimpel::
setErrInfo( const int errCode, const char *errMsg )
{
   setErrorCode( errCode );

   if( errMsg )
      setErrMsg( errMsg );

   return this->errMsg;
}

std::string
dnmi::db::drivers::
SQLitePimpel::
setErrInfo( const int errCode, const std::string &errMsg )
{
   setErrorCode( errCode );

   if( ! errMsg.empty() )
      this->errMsg = errMsg;

   return this->errMsg;
}


std::string
dnmi::db::drivers::
SQLitePimpel::
getErrMsg()const
{
   return errMsg;
}

void
dnmi::db::drivers::
SQLitePimpel::
setErrMsg( const char *errmsg )
{
   if( ! errmsg )
      errMsg = "SQLite: UNKNOWN error!";
   else
      errMsg = errmsg;
}


void
dnmi::db::drivers::
SQLitePimpel::
perform( dnmi::db::Connection *con_,
         dnmi::db::Transaction &transaction, int retry,
         dnmi::db::Connection::IsolationLevel isolation)
{
   dnmi::db::Transaction &t( transaction );
   std::string lastError;
   con = static_cast<SQLiteConnection*>( con_ );

   while( retry > 0 ) {
      try {
         beginTransaction( isolation );
      }
      catch( ... ) {
         lastError = con->lastError();
         retry--;
         continue;
      }

      try {
         if( t( con ) ) {
            t.onSuccess();
            con->endTransaction();
            return;
         } else {
            t.onFailure();
            retry--;
         }
      }
      catch (const SQLAborted &e) {
         retry--;
         lastError=e.what();
         t.onAbort( con->getDriverId(), lastError, e.errorCode() );
      }
      catch( const SQLBusy &e) {
         continue;
      }
      catch( const SQLException &e) {
         istringstream is( e.errorCode() );
         int i;
         is >> i;

         if( i == SQLITE_INTERRUPT ) {
            try {
               con->rollBack();
            }
            catch( ... ) {
               lastError=con->lastError();
            }
            t.onAbort( con->getDriverId(), e.what(), e.errorCode() );
            break;
         }
      }
      catch( const std::exception &e ) {
         lastError = e.what();
         retry--;
      }
      catch( ... ) {
         retry--;
      }

      t.onRetry();

      try {
         con->rollBack();
      }
      catch( ... ) {
         lastError = con->lastError();
         con->tryReconnect();
      }
   }

   transaction.onMaxRetry( lastError );
}



namespace {
int
dataCallback(void *pArg, int argc, char **argv, char **columnNames)
{
   SQLite::SQLiteData *data=static_cast<SQLite::SQLiteData*>(pArg);
   SQLite::Row row(argc);

   if(!data){
      cerr << "DEBUG: Error data==0\n";
      return 1;
   }

   if(data->fieldNames.size()!=argc){
      for(int i=0; i<argc; i++){
         data->fieldNames.push_back(columnNames[i]);
      }
   }

   for(int i=0; i<argc; i++){
      row[i]=(argv[i]?argv[i]:"");
   }

   data->data.push_back(row);

   return 0;
}
}
