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
  }else
    errMsg="Can't connect to database, OUT OF MEMMORY!";
}

dnmi::db::drivers::PGConnection::~PGConnection()
{
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

    PQfinish(con);
    con=0;

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
    bool           connected=false;

    if(!con)
	throw SQLNotConnected("NO CONNECTION, not connected to any database!");

    for(int i=0; i<2 && !connected; i++){
      if(!isConnected()){
	if(tryReconnect())
	  connected=true;
	else
	  sleep(1);
      }else
	connected=true;
    }
    
    if(!connected)
      throw SQLNotConnected("NO CONNECTION, not connected to any database!");

    p=PQexec(con, query.c_str());

    if(!p)
	throw SQLException(lastError());
    
    status=PQresultStatus(p);
    
  

    if(status==PGRES_COMMAND_OK || status==PGRES_TUPLES_OK ){
      PQclear(p);
      return;
    }
 
    char *msg=PQresStatus(status);
    std::string msg2=PQresultErrorMessage(p);
    std::string msgStr;
    std::string::size_type i;

    i=msg2.find("duplicate");
    
    if(i!=std::string::npos){
      i=msg2.find("key");
      
      if(i!=std::string::npos){
	PQclear(p);
	throw SQLDuplicate(msg2);
      }
    }

    i=msg2.find("aborted");
    
    if(i!=std::string::npos){
      i=msg2.find("transaction");
      
      if(i!=std::string::npos){
	PQclear(p);
	throw SQLAborted(msg2);
      }
    }


    if(msg)
      msgStr=msg;
    
    if(!msg2.empty())
      msgStr=msg2;
    

    PQclear(p);

    if(!msgStr.empty())
	throw SQLException(msgStr);
    else
	throw SQLException("UNKNOWN ERROR!");
}


dnmi::db::Result*
dnmi::db::drivers::PGConnection::execQuery(const std::string &query)
{
  PGresult *p;
  ExecStatusType status;
  bool           connected=false;

  if(!con){
    throw SQLNotConnected("NO CONNECTION, not connected to any database!");
  }


  for(int i=0; i<2 && !connected; i++){
    if(!isConnected()){
      if(tryReconnect())
	connected=true;
      else
	sleep(1);
    }else
      connected=true;
  }

  if(!connected)
    throw SQLNotConnected("NO CONNECTION, not connected to any database!");
  
  p=PQexec(con, query.c_str());

  if(!p){
    throw SQLException(lastError());
  }

  status=PQresultStatus(p);
  
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
  std::string::size_type i;
  PQclear(p);

  i=msg.find("duplicate");
  
  if(i!=std::string::npos){
    i=msg.find("key");
    
    if(i!=std::string::npos){
      throw SQLDuplicate(msg);
    }
  }

  if(!msg.empty()){
    throw SQLException(msg);
  }else{
    throw SQLException("UNKNOWN ERROR!");
  }
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
