/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

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
#ifndef SRC_KVDATAINPUTD_DATASRCAPP_H_
#define SRC_KVDATAINPUTD_DATASRCAPP_H_

#include <exception>
#include <limits>
#include <string>
#include <list>
#include "boost/regex.hpp"
#include "boost/thread/thread.hpp"
#include "boost/date_time/posix_time/ptime.hpp"
#include "lib/kvalobs/kvbaseapp.h"
#include "lib/kvalobs/paramlist.h"
#include "lib/kvalobs/kvTypes.h"
#include "lib/decoder/decoderbase/decodermgr.h"
#include "lib/kvdb/dbdrivermgr.h"
#include "lib/json/json/json.h"
#include "lib/miconfparser/miconfparser.h"
#include "lib/kvsubscribe/SendData.h"
#include "lib/kvsubscribe/queue.h"
#include "lib/kvsubscribe/ProducerCommand.h"
#include "lib/kvsubscribe/KafkaProducerThread.h"
#include "kvDataInputd/ConnectionCache.h"
#include "kvDataInputd/DecodeCommand.h"
#include "kvDataInputd/DecoderExecutor.h"

/**
 * \defgroup kvDatainputd kvDatainputd
 *
 * @{
 */

struct HttpConfig {
  int port;
  int threads;
  milog::LogLevel loglevel;
  int logRotate;
  int logSize;

  HttpConfig()
      : port(8090),
        threads(5),
        loglevel(milog::ERROR),
        logRotate(0),
        logSize(0) {
  }
};

struct KafkaConfig {
  std::string brokers;
  std::string domain;

  KafkaConfig()
      : brokers("localhost") {
  }

  std::string getRawTopic() {
    return kvalobs::subscribe::queue::raw(domain);
  }
};

/**
 * \brief DataSrcApp is a class that encapsulate the main datastructure in the
 * \em kvDatainputd application.
 *
 * It contains the message que that consument threads reads
 * the command from. It manages the connection to the database and it
 * manages the deoders.
 */
class DataSrcApp : public KvBaseApp {
  typedef boost::mutex::scoped_lock Lock;

  static boost::regex reMessageid;
  kvalobs::decoder::DecoderMgr decoderMgr;
  bool ok;
  std::string connectStr;
  std::string dbDriver;
  ConnectionCache conCache;
  int nConnections;
  ParamList paramList;
  std::list<kvalobs::kvTypes> typeList;
  bool shutdown_;
  boost::posix_time::ptime nextParamCheckTime;
  HttpConfig httpConfig;
  KafkaConfig kafkaConfig;
  DecoderExecutor decoderExecutor;
  kvalobs::service::KafkaProducerThread kafkaRawStream;

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
  int registerDb(int nConn);

  /**
   * \brief registerAllDecoders, scans the directory $KVALOBS/lib for decoders.
   *
   * All decoders is managed by decoderMgr.
   *
   * \returns true if we find at least one decoder. false if no decoders is
   * found.
   */
  bool registerAllDecoders(miutil::conf::ConfSection *theConf);

  DecodeCommand* decodeExecute(DecodeCommand *decCmd, kvalobs::datasource::Result *res, const std::string &logid);
  DecodeCommand* decode(const char *obsType, const char *data, const std::string &logid, kvalobs::datasource::Result *res);

  boost::mutex mutex;

 public:
  typedef enum {
    TimeOut,
    NoDecoder,
    NoDbConnection,
    NoMem
  } ErrCode;

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
  DataSrcApp(int argn, char **argv, int numberOfConnections, miutil::conf::ConfSection *theKvConf);

  /**
   * \brief Detructor, deletes all connection to the database.
   */
  ~DataSrcApp();

  /**
   * \brief creates a DecodeCommand if it find a decoder for
   * the observation type.
   *
   * The function will block if no connection
   * to the database is available. The number of working threads
   * mandate the number of connection to the database that will
   * be created. The connections is cached to reduce the time used
   * to establish connection to the database. If no connection is
   * made available in the time limit given by timeoutIn_msec, null
   * will be returned and the errCode is set to TimeOut, and a appropriate
   * error message is given in errMsg.
   */

  DecodeCommand *create(const char *obsType, const char *obs, long timoutIn_msec, ErrCode &errCode, std::string &errMsg);

  void releaseDecodeCommand(DecodeCommand *command);

  bool sendInfoToManager(const kvalobs::kvStationInfoList &info, const std::list<kvalobs::serialize::KvalobsData> &decodedData);

  HttpConfig getHttpConfig() const {
    return httpConfig;
  }

  static std::string getMessageId(std::string &obstype);
  kvalobs::datasource::Result newObservation(const char *obsType, const char *data, const std::string &logid);

  /**
   * \brief getDbConnections returns the number of connections we have to the
   * database.
   */
  int getDbConnections() const {
    return nConnections;
  }

  /**
   * \brief returns true if this instance of DataSrcApp is valid constructed!
   *
   * \return True if valid. False otherwise.
   */
  virtual bool isOk() const;

  /**
   * \brief returns a pointer to the raw message queue.
   *
   * \return a pointer to the message que.
   */
  kvalobs::service::ProducerQuePtr getRawQueue() {
    return kafkaRawStream.queue;
  }

  /**
   * \brief Request shutdown. Ie. we want to terminate.
   */
  void shutdown();
  /**
   * \brief Are we in shutdown.
   *
   * \return true if we are in shutdown. When we are in shutdown all threads
   * shall end the jobs they ar doing and terminate.
   */
  bool inShutdown() const;
};

/** @} */

#endif  // SRC_KVDATAINPUTD_DATASRCAPP_H_
