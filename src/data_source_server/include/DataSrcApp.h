/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: DataSrcApp.h,v 1.19.2.5 2007/09/27 09:02:16 paule Exp $                                                       

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
#ifndef __DataSrcApp_h__
#define __DataSrcApp_h__

#include <exception>
#include <CORBA.h>
#include <boost/thread/thread.hpp>
#include <string>
#include <kvskel/managerInput.hh>
#include <kvalobs/kvapp.h>
#include <dnmithread/CommandQue.h>
#include "DecodeCommand.h"
#include <decoderbase/decodermgr.h>
#include <db/dbdrivermgr.h>
#include "ConnectionCache.h"
#include <kvalobs/paramlist.h>
#include <kvalobs/kvTypes.h>
#include <list>

/**
 * \defgroup kvDatainputd kvDatainputd
 *
 * @{
 */

/**
 * \brief DataSrcApp is a class that encapsulate the main datastructure in the 
 * \em kvDatainputd application. 
 *
 * It contains the message que that consument threads reads
 * the command from. It manages the connection to the database and it
 * manages the deoders.
 */
class DataSrcApp : public KvApp
{
  typedef boost::mutex::scoped_lock    Lock;

  CKvalObs::CManager::ManagerInput_var refMgr;
  dnmi::thread::CommandQue             que;
  kvalobs::decoder::DecoderMgr         decoderMgr;
  dnmi::db::DriverManager              dbMgr;
  bool                                 ok;
  std::string                          connectStr;
  std::string                          dbDriver;
  ConnectionCache                      conCache;
  int                                  nConnections;
  ParamList                            paramList;
  std::list<kvalobs::kvTypes>          typeList;
  bool                                 shutdown_;

  /**
   * \brief registerParams reads parameter information from the table
   * kv_params into paramList.
   */
  bool registerParams();

  /**
   * \brief registerTypes, reads the 'types' from the kvalobs database.
   *
   * This information is often needed by the decoders so we cache
   * this information.
   */
  bool registerTypes();


  /**
   * \brief registerDb looks for the dbDriver.so in the directory $KVALOBS/lib.
   *
   * If the driver is found it is loaded.
   *
   * The function also creates connections to the database. The connections
   * is registred in conCache (connection cache).
   *
   * \param nConn is a hint about how many connection to the databse we 
   *        want to open.
   * \return the number of connections that is created to the database.
   */
  int  registerDb(int nConn);

  /**
   * \brief registerAllDecoders, scans the directory $KVALOBS/lib for decoders.
   *
   * All decoders is managed by decoderMgr.
   *
   * \returns true if we find at least one decoder. false if no decoders is 
   * found.
   */
  bool registerAllDecoders();

  /**
   *\brief lookUpManager, use the CORBA nameserver if necessary, to
   *find the ManagerInput interface.
   *
   * \param forceNS if true use only the nameserver to find
   *                the reference.
   * \param usedNS is set on output to true if the CORBA nameserver
   *               was uesd to find the reference.
   * \return a reference to a ManagerInput object.
   * \exception throws LookUpException if it fails to
   */
  CKvalObs::CManager::ManagerInput_ptr lookUpManager(bool forceNS,
						     bool & usedNS);
  boost::mutex mutex;

 public:
  typedef enum ErrCode {TimeOut, NoDecoder, NoDbConnection, NoMem}; 
  

  /**
   * \brief Constructor that initialize a DataSrcApp instance. 
   *
   * It is supposed* to be only one of this object in the application. 
   * It should have been implemented as a singleton.
   *
   * Use the function isOk() to check if the construction was successfull.
   *
   * \param argn from main(argn, argv)
   * \param argv from main(argn, argv)
   * \param connectStr the string we shall use to connect to
   *        the databse.
   * \param dbdriver which database driver shall we use.
   * \param numberOfConnections how many connections to the database
   *        shall we try to create.
   * \param opt Optional options to omniorb.
   */
  DataSrcApp(int argn, char **argv,
	          int              numberOfConnections,
	          const char      *opt[][2]=0);

  /**
   * \brief Detructor, deletes all connection to the database.
   */
  ~DataSrcApp();
  
  /**
   * \brief creates a DecodeCommand if it find a decoder for
   * the observation type. 
   * 
   * The function will block if no conection
   * to the database is available. The number of working threads
   * mandate the number of connection to the database that will
   * be created. The connections is cached to reduce the time used
   * to establish connetction to the database. If no conection is 
   * made available in the time limit given by timeoutIn_msec, null
   * will be returned and the errCode is set to TimeOut, and a appropriate
   * error message is given in errMsg.
   */

  DecodeCommand *create(const char  *obsType, 
			const char  *obs,
			long        timoutIn_msec,
			ErrCode     &errCode,
			std::string &errMsg);
  
  DecodeCommand *create(CORBA::Long report_id, 
			const char* report_type, 
			const char* obstime, 
			const char* data,
			long        timoutIn_msec,
			ErrCode     &errCode,
			std::string &errMsg);

  void releaseDecodeCommand(DecodeCommand *command);

  bool sendInfoToManager(const kvalobs::kvStationInfoList &info);
  
  /**
   * \brief put is used to send the command to the consument threads.
   *
   * The consuments call execute on the comman and the message 
   * is decoded. If the command can't be posted on the que the function
   * returns false. The que is suspened, this means that the threads
   * is shuting down.
   */
  bool put(dnmi::thread::CommandBase *command);

  /**
   * \brief  remove the command in the que to be proccessed by the consument
   * thread. 
   *
   * If the method return is != 0. The Command is removed from
   * the que. If the method return 0. The command is received by the consument
   * thread and is proccessed.
   */
  dnmi::thread::CommandBase* remove(dnmi::thread::CommandBase *command);


  /**
   * \brief getDbConnections returns the number of connections we have to the 
   * database.
   */
  int getDbConnections()const{ return nConnections;}
  
  /**
   * \brief returns true if this instance of DataSrcApp is valid constructed!
   *
   * \return True if valid. False otherwise.
   */
  virtual bool isOk()const;

  /**
   * \brief returns a pointer to the message Que. Never delete
   * this pointer.
   *
   * \return a pointer to the message que.
   */
  dnmi::thread::CommandQue  *getQue(){ return &que;} 

  /**
   * \brief Request shutdown. Ie. we want to terminate.
   */
  void shutdown(){shutdown_=true;}

  /**
   * \brief Are we in shutdown. 
   *
   * \return true if we are in shutdown. When we are in shutdown all threads
   * shall end the jobs they ar doing and terminate.
   */
  bool inShutdown()const;
};


/** @} */


#endif
