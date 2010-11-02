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
    setErrMsg("Out of memory!");
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
    
dnmi::db::drivers::SQLiteConnection::SQLiteConnection(
					       const std::string &connect,
					      const std::string &driverId)
  :Connection(driverId), con(0)
{
  int res = sqlite3_open( connect.c_str(), & con );
  if ( SQLITE_OK != res )
  {
	  errMsg = sqlite3_errmsg(con);
	  con = 0;
  }

  if(con)
    sqlite3_busy_timeout(con, 5000); //sets busy timeout to 5 second.
}

dnmi::db::drivers::SQLiteConnection::~SQLiteConnection()
{
  if(con)
    sqlite3_close(con);
}


bool 
dnmi::db::drivers::SQLiteConnection::isConnected()
{
  if(con==0)
    return false;

  return true;
} 

bool 
dnmi::db::drivers::SQLiteConnection::tryReconnect()
{
  if(!con)
    return false;

  return true;
}


void
dnmi::db::drivers::SQLiteConnection::beginTransaction()
{
  exec("BEGIN");
}

void 
dnmi::db::drivers::SQLiteConnection::endTransaction()
{
  exec("END");
}

void 
dnmi::db::drivers::SQLiteConnection::rollBack()
{
  exec("ROLLBACK");
}

void
dnmi::db::drivers::SQLiteConnection::exec(const std::string &query)
{
   //  string sMsg;
   int   sqliteRes;
   char  *msg=0;

   errMsg.erase();

   if(!con)
      throw SQLNotConnected("NO CONNECTION, not connected to any database!");

   sqliteRes=sqlite3_exec(con, query.c_str(), 0, 0, &msg);

   if(msg){
      errMsg=msg;
      sqlite3_free(msg);
   }

   if(sqliteRes!=SQLITE_OK){
      ostringstream emsg;
      //cerr << "ERROR: " << sqliteRes << endl;
      if(sqliteRes==SQLITE_CONSTRAINT){
         string::size_type i=errMsg.find("unique");

         if(i!=string::npos){
            throw SQLDuplicate("SQLite: Duplicate (" + errMsg +")");
         }else{
            emsg << "SQLite: sqliteres(" << sqliteRes << "): " + errMsg;
            throw SQLException( emsg.str() );
         }
      }else if(sqliteRes==SQLITE_BUSY){
         throw SQLBusy("SQLite: " + errMsg);
      }else{
         emsg << "SQLite: sqliteres(" << sqliteRes << "): " + errMsg;
         throw SQLException( emsg.str() );
      }
   }
}


dnmi::db::Result*
dnmi::db::drivers::SQLiteConnection::execQuery(const std::string &query)
{
  string sMsg;
  int    sqliteRes;
  char   *msg=0;
  SQLite::SQLiteData *data;
  
  if(!con)
    throw SQLNotConnected("SQLite: NO CONNECTION, not connected to any database!");
  
  try{
    data = new SQLite::SQLiteData();
  }
  catch(...){
    throw SQLException("NOMEM: cant create 'SQLiteData'");
  }  

  sqliteRes=sqlite3_exec(con, query.c_str(), dataCallback, data, &msg);


  if(msg){
    sMsg=msg;
    sqlite3_free(msg);
  }else
    sMsg="SQLite: Unknown error!";


  if(sqliteRes!=SQLITE_OK){
    delete data;
    //cerr << "ERROR: " << sqliteRes << endl;

    if(sqliteRes==SQLITE_BUSY){
      throw SQLBusy("SQLiteBusy: " + sMsg);
    }else{
      ostringstream  emsg;
      emsg << "SQLite: sqliteres(" << sqliteRes << "): " + errMsg;
      throw SQLException( emsg.str() );
    }
  }
  
  try{
    SQLiteResult *res=new SQLiteResult(data);
    return res;
  }
  catch(...){
    delete data;
    throw SQLException("NOMEM: cant create 'SQLiteResult'");
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
      throw SQLException("INTERNAL ERROR: Incompatible datasize!");
    
    for(int i=0; i<nextData->size(); i++){
	data[i]=(*nextData)[i];
    }

    nextData++;
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
