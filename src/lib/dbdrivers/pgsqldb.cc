/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: pgsqldb.cc,v 1.6.2.1 2007/09/27 09:02:26 paule Exp $                                                       

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
#include <unistd.h>
#include <sstream>
#include <time.h>
#include "pgsqldb.h"

using namespace std;

dnmi::db::drivers::PGDriver::PGDriver()
{
}

dnmi::db::drivers::PGDriver::~PGDriver()
{
}

dnmi::db::Connection* 
dnmi::db::drivers::PGDriver::createConnection(const std::string &connect)
{
   PGConnection *con;

   try{
      con=new PGConnection(connect, name());
   }
   catch(...){
      setErrMsg("Out of memmory!");
      return 0;
   }

   if(con->isConnected())
      return con;

   delete con;

   return 0;
}

bool        
dnmi::db::drivers::PGDriver::releaseConnection(Connection *connect)
{
   if(connect->getDriverId()!=name()){
      stringstream ost;
      ost << "ERROR: trying to release a connection with driverId <" <<
            connect->getDriverId() << ">, but this driver har driverId <" <<
            name() << ">!\n";
      setErrMsg(ost.str());
      return false;
   }

   delete connect;

   return true;
}

dnmi::db::drivers::PGConnection::PGConnection(const std::string &connect,
                                              const std::string &driverId)
:Connection(driverId), con(0)
{
   char *msg;

   pimpel=0;


   con=PQconnectdb(connect.c_str());

   if(con){
      if(PQstatus(con)!=CONNECTION_OK){
         msg=PQerrorMessage(con);

         if(msg)
            errMsg=msg;
         else
            errMsg="Can't connect to database, UNKNOWN ERROR!";

         PQfinish(con);
         con=0;
      }
      pimpel = new PGPimpel( this );
   }else
      errMsg="Can't connect to database, OUT OF MEMMORY!";
}

dnmi::db::drivers::PGConnection::~PGConnection()
{
   if( pimpel ) {
      delete static_cast<PGPimpel*>(pimpel);
   }

   if(con){
      std::cerr << "Disconnect from a PostgreSQL database!\n";
      PQfinish(con);
   }
}


bool 
dnmi::db::drivers::PGConnection::isConnected()
{
   if(con==0)
      return false;

   if(PQstatus(con)!=CONNECTION_OK){
      char *msg=PQerrorMessage(con);

      if(msg)
         errMsg=msg;
      else
         errMsg="Can't connect to database, UNKNOWN ERROR!";

      return false;
   }

   return true;
} 

bool 
dnmi::db::drivers::PGConnection::tryReconnect()
{
   if(!con)
      return false;

   PQreset(con);

   if(PQstatus(con)!=CONNECTION_OK){
      char *msg=PQerrorMessage(con);

      if(msg)
         errMsg=msg;
      else
         errMsg="Can't connect to database, UNKNOWN ERROR!";

//      PQfinish(con);
//      con=0;

      return false;
   }

   return true;
}


void
dnmi::db::drivers::PGConnection::beginTransaction()
{
   exec("BEGIN");
}

void 
dnmi::db::drivers::PGConnection::endTransaction()
{
   exec("END");
}

void 
dnmi::db::drivers::PGConnection::rollBack()
{
   exec("ROLLBACK");
}

void
dnmi::db::drivers::PGConnection::exec(const std::string &query)
{
   PGresult *p;
   ExecStatusType status;
   bool           again=false;

   if(!con)
      throw SQLNotConnected("NO CONNECTION, not connected to any database!");

   do {
      if( ! isConnected() ) {
         if( ! tryReconnect() || again ) {
            ostringstream err;
            err << "NO CONNECTION, lost connection to the database!";
            if( !errMsg.empty() )
               err << " Reason(?): " << errMsg;

            throw SQLNotConnected( err.str());
         }

         again = true;
         continue;
      }

      p=PQexec(con, query.c_str());

      status=PQresultStatus(p);
      again = false;

      if( status == PGRES_FATAL_ERROR )
         again = true;

   } while( again );


   if(status==PGRES_COMMAND_OK || status==PGRES_TUPLES_OK ){
      PQclear(p);
      return;
   }

   std::string msg = PQresStatus(status);
   std::string msg2 = PQresultErrorMessage(p);
   std::string errorCode( PQresultErrorField( p, PG_DIAG_SQLSTATE ) );

   PQclear(p);

   std::string::size_type i=msg2.find("duplicate");

   if(i!=std::string::npos){
      i=msg2.find("key");

      if(i!=std::string::npos){

         throw SQLDuplicate(msg2, errorCode);
      }
   }

   if( msg2.empty() ) {
      if( msg.empty() )
         msg = getDriverId() +": Unknow error!";
   } else {
      msg = msg2;
   }

   if( errorCode.length() >= 2 ) {
      std::string errClass = errorCode.substr( 0, 2 );

      if( errClass == "22 ") {
         throw SQLException( msg, errorCode );
      }

      throw SQLAborted( msg, errorCode );
   }

   throw SQLAborted( msg, errorCode );
}


dnmi::db::Result*
dnmi::db::drivers::PGConnection::execQuery(const std::string &query)
{
   PGresult *p;
   ExecStatusType status;
   bool again=false;

   if(!con){
      throw SQLNotConnected("NO CONNECTION, not connected to any database!");
   }

   do {
      if( ! isConnected() ) {
         if( ! tryReconnect() || again ) {
            ostringstream err;
            err << "NO CONNECTION, lost connection to the database!";
            if( !errMsg.empty() )
               err << " Reason(?): " << errMsg;

            throw SQLNotConnected( err.str());
         }

         again = true;
         continue;
      }

      p=PQexec(con, query.c_str());

      status=PQresultStatus(p);
      again = false;

      if( status == PGRES_FATAL_ERROR )
         again = true;

   } while( again );

   if(status==PGRES_TUPLES_OK){
      try{
         return new PGResult(p);
      }
      catch(...){
         PQclear(p);
         throw SQLException("OUT OF MEMMORY!");
      }
   }

   if(status==PGRES_COMMAND_OK || status==PGRES_EMPTY_QUERY){
      PQclear(p);
      return 0;
   }

   std::string msg=PQresultErrorMessage(p);
   std::string errorCode( PQresultErrorField( p, PG_DIAG_SQLSTATE ) );

   PQclear(p);

   std::string::size_type i = msg.find("duplicate");

   if( i != std::string::npos){
      i=msg.find("key");

      if( i != std::string::npos)
         throw SQLDuplicate(msg, errorCode );
   }

   if( msg.empty() )
      msg = getDriverId() + ": Unknown error!";

   if( errorCode.length() >= 2 ) {
      std::string errClass = errorCode.substr( 0, 2 );

      if( errClass == "22 ")
         throw SQLException(msg, errorCode );

      throw SQLAborted( msg, errorCode );
   }

   throw SQLAborted(msg, errorCode );
}

std::string 
dnmi::db::drivers::PGConnection::lastError()const
{
   char *msg;

   if(!con)
      return errMsg;

   msg=PQerrorMessage(con);

   if(msg)
      return msg;
   else
      return std::string("UNKNOWN ERROR, cant get error message!");
}


std::string 
dnmi::db::drivers::
PGConnection::
esc( const std::string &stringToEscape )const
{
   if(!con)
      throw SQLException("NO CONNECTION: not connected to a database!");

   char *buf = 0;

   try {
      buf = new char[stringToEscape.length()*2 + 1];

      PQescapeStringConn( con, buf,
                          stringToEscape.c_str(), stringToEscape.length(),
                          0 );

      string ret(buf);
      delete[] buf;
      buf=0;
      return ret;
   }
   catch( ... ) {
      if( buf )
         delete[] buf;

      throw SQLException("NOMEM: Cant escape the string.");
   }
}


dnmi::db::drivers::PGResult::PGResult(PGresult *r):Result(PQnfields(r))
{ 
   res=r;
   nFields=PQnfields(res);
   nTuples=PQntuples(res);
   tupleIndex=0;

   // cerr << "PGResult:ctor:: nFields=" << nFields << " nTuples=" << nTuples <<endl;
}

dnmi::db::drivers::PGResult::~PGResult()
{
   if(res)
      PQclear(res);
}

bool               
dnmi::db::drivers::PGResult::hasResult()const
{
   return res!=0;
}

int                        
dnmi::db::drivers::PGResult::fields()const
{
   return nFields;
}

std::string 
dnmi::db::drivers::PGResult::fieldName(int index)const
{
   if(index>=nFields)
      throw SQLException("index out of range!");

   return PQfname(res, index);

}

int         
dnmi::db::drivers::PGResult::fieldIndex(const std::string &fieldName)const
{
   return PQfnumber(res, fieldName.c_str());
}

dnmi::db::FieldType   
dnmi::db::drivers::PGResult::fieldType(int index)const
{
   if(index>=nFields)
      throw SQLException("index out of range!");
}

dnmi::db::FieldType   
dnmi::db::drivers::PGResult::fieldType(const std::string &fieldName)const
{
}

int         
dnmi::db::drivers::PGResult::fieldSize(int index)const
{
   if(index>=nFields)
      throw SQLException("index out of range!");

   return PQfsize(res, index);
}

int         
dnmi::db::drivers::PGResult::fieldSize(const std::string &fieldName)const
{
   return fieldSize(fieldIndex(fieldName));
}

int         
dnmi::db::drivers::PGResult::size()const
{
   return PQntuples(res);
}

bool        
dnmi::db::drivers::PGResult::hasNext()const
{
   if(tupleIndex<nTuples)
      return true;

   return false;
}

void
dnmi::db::drivers::PGResult::nextImpl()
{
   if(tupleIndex>=nTuples)
      throw SQLException("No more data!");

   for(int i=0; i<fields(); i++){
      data[i]=PQgetvalue(res, tupleIndex, i);
   }

   tupleIndex++;

}



void
dnmi::db::drivers::
PGPimpel::
createSavepoint( const std::string &name )
{
   con->exec("SAVEPOINT " + name);
}

void
dnmi::db::drivers::
PGPimpel::
rollbackToSavepoint( const std::string &name )
{
   con->exec("ROLLBACK TO SAVEPOINT " + name);
}

void
dnmi::db::drivers::
PGPimpel::
releaseSavepoint( const std::string &name )
{
   con->exec("RELEASE SAVEPOINT " + name);
}


void
dnmi::db::drivers::
PGPimpel::
beginTransaction(dnmi::db::Connection::IsolationLevel isolation)
{
   using namespace dnmi::db::priv;
   switch( isolation ) {
   case Connection::SERIALIZABLE:
      con->exec("START TRANSACTION ISOLATION LEVEL SERIALIZABLE");
      break;
   case Connection::READ_COMMITTED:
      con->exec("START TRANSACTION ISOLATION LEVEL READ COMMITTED");
      break;
   case Connection::READ_UNCOMMITTED:
      con->exec("START TRANSACTION ISOLATION LEVEL READ UNCOMMITTED" );
      break;
   case Connection::REPEATABLE_READ:
      con->exec("START TRANSACTION ISOLATION LEVEL REPEATABLE READ");
      break;
   }

}

dnmi::db::drivers::
PGPimpel::
PGPimpel( PGConnection *con_ ) : dnmi::db::priv::Pimpel(), con( con_ )
{
}

void
dnmi::db::drivers::
PGPimpel::
perform( dnmi::db::Connection *con_,
         dnmi::db::Transaction &transaction, int retry,
         dnmi::db::Connection::IsolationLevel isolation)
{
   dnmi::db::Transaction &t( transaction );
   std::string lastError;
   con = static_cast<PGConnection*>( con_ );
   time_t start;
   time_t now;

   time( &start );

   if( retry <=0 )
      retry = 1;

   while( retry > 0 ) {
      try {
         beginTransaction( isolation );
      }
      catch( ... ) {
         if( ! con->tryReconnect() ) {
            lastError=con->lastError();
            break;
         }
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
         if( e.errorCode() != "40001" ) //SERIALIZATION FAILURE
            retry--;
         else { // e.errorCode() == "40001"
            lastError = e.what();
            time( &now );

            //We allow a three minutes periode of retry
            //before we reduce the retry counter.
            if( (now - start) > 180 ) {
               time( &start ); //Reset the timeout counter.
               retry--;
            }
         }
         t.onAbort( con->getDriverId(), e.what(), e.errorCode() );
      }
      catch( const std::exception &ex ){
         retry--;
         lastError = ex.what();
      }
      catch( ... ) {
         retry--;
      }

      try {
         con->rollBack();
      }
      catch( ... ) {
         lastError = con->lastError();
         con->tryReconnect();
      }

      t.onRetry();
   }

   transaction.onMaxRetry( lastError );

}

