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
#include <signal.h>
#include <regex>
#include <memory>
#include <chrono>
#include "boost/lexical_cast.hpp"
#include "boost/algorithm/string.hpp"
#include "lib/milog/milog.h"
#include "lib/miutil/timeconvert.h"
#include "lib/kvalobs/bitmanip.h"
#include "lib/kvalobs/kvDbGate.h"
#include "lib/kvalobs/kvPath.h"
#include "lib/kvalobs/getLogInfo.h"
#include "lib/kvsubscribe/queue.h"
#include "kvDataInputd/DataSrcApp.h"

using std::list;
using std::string;
using std::endl;
using std::ostringstream;
using std::cerr;
using dnmi::db::Result;
using dnmi::db::DRow;
using dnmi::db::Connection;
using dnmi::db::SQLException;
using kvalobs::decoder::DecoderBase;
using boost::lexical_cast;
using boost::bad_lexical_cast;

volatile sig_atomic_t sigTerm = 0;

boost::regex DataSrcApp::reMessageid(".+(/ *messageid *=([^/]*)).*", boost::regex::perl | boost::regex::icase);

DataSrcApp::DataSrcApp(int argn, char **argv, int nConnections_, miutil::conf::ConfSection *theKvConf)
    : KvBaseApp(argn, argv),
      ok(false),
      shutdown_(false) {
  miutil::conf::ConfSection *conf;
  string logdir(kvPath("logdir"));
  string myPath = kvPath("pkglibdir");
  myPath += "/decode";

  conf = KvBaseApp::getConfiguration();

  if (!conf) {
    LOGFATAL("Cant read configuration file: " << getConfFile() << endl);
    cerr << "Cant read configuration file: " << getConfFile() << endl;
    exit(1);
  }

  httpConfig.port = conf->getValue("kvDataInputd.http.port").valAsInt(httpConfig.port);
  httpConfig.threads = conf->getValue("kvDataInputd.http.threads").valAsInt(httpConfig.threads);
  httpConfig.loglevel = getLoglevelRecursivt(conf, "kvDataInputd.http", httpConfig.loglevel);
  kafkaConfig.brokers = conf->getValue("kafka.brokers").valAsString("localhost");
  kafkaConfig.domain = conf->getValue("kafka.domain").valAsString("");

  if (kafkaConfig.domain.empty()) {
    LOGFATAL("This kvalobs instance must have a name. kafka.domain must be set in the configuration file.");
    exit(1);
  }

  getLogfileInfo(conf, "kvDataInputd.http", httpConfig.logRotate, httpConfig.logSize);

  miutil::conf::ValElementList val;
  val = conf->getValue("database.dbdriver");

  if (val.size() == 1)
    dbDriver = val[0].valAsString();

  connectStr = KvBaseApp::createConnectString();

  if (dbDriver.empty())
    dbDriver = "pgdriver.so";

  decoderMgr.setTheKvConf(theKvConf);
  decoderMgr.setDecoderPath(myPath);

  nConnections = registerDb(nConnections_);

  if (nConnections < 1)
    return;

  LOGINFO("Trying to set decoder pool size " << nConnections << ".");
  decoderExecutor.init(nConnections);
  LOGINFO("Decoder pool size set to " << decoderExecutor.poolSize() << ".");
  if (!registerAllDecoders(theKvConf))
    return;

  milog::createGlobalLogger(logdir, "kvDataInputd", "param_update", milog::DEBUG);

  if (!registerParams())
    return;

  if (!registerTypes())
    return;

  milog::createGlobalLogger(logdir, "kvDataInputd", "http", httpConfig.loglevel, httpConfig.logSize, httpConfig.logRotate);
  milog::createGlobalLogger(logdir, "kvDataInputd", "http_error", milog::ERROR, httpConfig.logSize, httpConfig.logRotate);
  milog::createGlobalLogger(logdir, "kvDataInputd", "http_access", milog::INFO, httpConfig.logSize, httpConfig.logRotate, new milog::StdLayout1());
  milog::createGlobalLogger(logdir, "kvDataInputd", "kafka", milog::DEBUG, httpConfig.logSize, httpConfig.logRotate, new milog::StdLayout1());
  milog::createGlobalLogger(logdir, "kvDataInputd_transaction", "failed", milog::DEBUG);
  milog::createGlobalLogger(logdir, "kvDataInputd_transaction", "duplicates", milog::DEBUG);
  milog::createGlobalLogger(logdir, "kvDataInputd_transaction", "updated", milog::DEBUG);
  milog::createGlobalLogger(logdir, "kvDataInputd_transaction", "retry", milog::DEBUG);
  milog::createGlobalLogger(logdir, "kvDataInputd", "transaction", milog::DEBUG, 200, 1, new milog::StdLayout1());

  try {
    LOGERROR("Starting kafka producer for topic <" << kafkaConfig.getRawTopic() << ">. Brokers <" << kafkaConfig.brokers << ">.");
    std::string name = kafkaRawStream.getName() + "-" + kafkaConfig.getRawTopic();
    kafkaRawStream.setName(name);
    kafkaRawStream.start(kafkaConfig.brokers, kafkaConfig.getRawTopic());
  } catch (const std::exception &ex) {
    LOGERROR(
        "Failed to start the kafka stream for topic <" << kafkaConfig.getRawTopic() << ">.\n" "Brokers: <" << kafkaConfig.brokers << ">\n" "REason: " << ex.what());
    return;
  }
  ok = true;
}

bool DataSrcApp::sendInfoToManager(const kvalobs::kvStationInfoList &info_, const std::list<kvalobs::serialize::KvalobsData> &decodedData) {
  return true;
}

DataSrcApp::~DataSrcApp() {
}

int DataSrcApp::registerDb(int nConn) {
  string driver(kvPath("pkglibdir") + "/db/" + dbDriver);
  string drvId;
  int n = 0;

  LOGINFO("registerDb: loading driver <" << dbMgr.fixDriverName(driver) << ">!\n");

  if (!dbMgr.loadDriver(driver, drvId)) {
    LOGFATAL(dbMgr.getErr());
    return 0;
  }

  if (setAppNameForDb && !appName.empty())
    dbMgr.setAppName(appName);

  LOGINFO("registerDb: Driver <" << drvId<< "> loaded!\n");

  for (int i = 0; i < nConn; i++) {
    Connection *con = 0;

    while (!con && !inShutdown()) {
      con = dbMgr.connect(drvId, connectStr);

      if (!con) {
        LOGFATAL("registerDb: Can't create connection to <" << drvId << ">\n ERROR message: " << dbMgr.getErr());
        sleep(1);
      } else {
        conCache.addConnection(con);
        n++;
      }
    }
  }

  return n;
}

bool DataSrcApp::registerAllDecoders(miutil::conf::ConfSection *theConf) {
  decoderMgr.updateDecoders(theConf);
  if (decoderMgr.numberOfDecoders() < 1) {
    LOGERROR("registerAllDecoders: Can't registers decoders!\n");
    return false;
  }

  return true;
}

bool DataSrcApp::registerParams() {
  Result *res = 0;
  Connection *con;
  ParamList tmpParams;
  ParamList newParams;
  Param param;
  string paramFile;
  ostringstream textParams;
  boost::posix_time::ptime now;

  now = boost::posix_time::second_clock::universal_time();

  if (!nextParamCheckTime.is_special() && now < nextParamCheckTime)
    return true;

  paramFile = kvalobs::kvPath(kvalobs::sysconfdir);

  paramFile += "/stinfosys_params.csv";

  con = conCache.findFreeConnection();

  if (!con) {
    LOGDEBUG("registerParams: SIGNAL: Stopped on signal!\n");
    return false;
  }

  if (!readParamsFromFile(paramFile, tmpParams)) {
    string pfile = paramFile;
    paramFile = kvalobs::kvPath(kvalobs::sysconfdir) + "/stinfosys_params.csv.default";

    IDLOGERROR(
        "param_update",
        "Cant read parameter information from file <" << pfile << ">." << endl << "Trying to load parameter information from default file <" << paramFile << endl << ">. This file may be incomplete. Use 'kv_get_stinfosys_params' to generate the file" << endl << "'" << pfile << "'.");

    LOGERROR(
        "Cant read parameter information from file <" << pfile << ">." << endl << "Trying to load parameter information from default file <" << paramFile << endl << ">. This file may be incomplete. Use 'kv_get_stinfosys_params' to generate the file" << endl << "'" << pfile << "'.");

    if (!readParamsFromFile(paramFile, tmpParams)) {
      IDLOGERROR("param_update", "Cant read parameter information from file <" << paramFile << ">.");
      LOGERROR("Cant read parameter information from file <" << paramFile << ">.");
    }
  }

  if (!tmpParams.empty()) {
    IDLOGINFO("param_update", "Loaded #" << tmpParams.size() << " 'stinfosys' param definitions from file <" << paramFile << ">.");
  }

  try {
    res = con->execQuery("SELECT paramid,name FROM param");

    if (res && res->size() > 0) {
      while (res->hasNext()) {
        DRow row = res->next();

        try {
          int id = lexical_cast<int>(row[0]);
          bool scalar = true;

          if (id >= 1000)
            scalar = false;

          if (findParamInList(tmpParams, row[1], param)) {
            if (!param.isScalar())
              textParams << param.kode() << "(" << param.id() << ") ";
            scalar = param.isScalar();
          }
          newParams.insert(Param(row[1], id, scalar));
        } catch (bad_lexical_cast &) {
          LOGERROR("registerParams: BADNUM: paramid is not a number\n" << "   paramid(" << row[0] << ")\n");
        }
      }

      delete res;
    }
  } catch (SQLException &ex) {
    delete res;
    conCache.freeConnection(con);
    LOGERROR("registerParams: Exception: " << ex.what() << endl);
    return false;
  } catch (...) {
    delete res;
    conCache.freeConnection(con);
    LOGERROR("registerParams: Exception: Unknown!\n");
  }

  IDLOGINFO("param_update", "Defined text parameters: " << textParams.str());
  conCache.freeConnection(con);

  if (!isParamListsEqual(paramList, newParams)) {
    if (!paramList.empty()) {
      IDLOGINFO("param_update", "New, deleted or changed parameter definition in kvalobs.");
    } else {
      IDLOGDEBUG("param_update", "Parameter definitions is initialized.");
    }
    paramList = newParams;
  }

  nextParamCheckTime = now + boost::posix_time::hours(1);
  IDLOGDEBUG("param_update", "nextParamCheckTime: " << nextParamCheckTime << ".");

  return true;
}

bool DataSrcApp::registerTypes() {
  Connection *con = conCache.findFreeConnection();

  if (!con) {
    LOGDEBUG("registerTypes: SIGNAL: Stopped on signal!\n");
    return false;
  }

  kvalobs::kvDbGate gate(con);

  if (!gate.select(typeList)) {
    LOGFATAL("registerTypes failed: " << gate.getErrorStr());
    conCache.freeConnection(con);
    return false;
  }

  conCache.freeConnection(con);

  return true;
}

DecodeCommand*
DataSrcApp::create(const char *obsType_, const char *obs, long timoutIn_msec, ErrCode &errCode, std::string &err) {
  DecoderBase *dec;
  DecodeCommand *decCmd;
  Connection *con;
  string myErr;

  // The call to conCache will block until a connection object
  // is ready in the cache.
  con = conCache.findFreeConnection();

  if (!con) {
    errCode = NoDbConnection;
    err = "SIGNAL: Stopped on signal!";
    return 0;
  }

  // lookup the decoder based on obsType.
  {
    Lock lck(mutex);

    // Check for updated params.
    registerParams();

    dec = decoderMgr.findDecoder(*con, paramList, typeList, obsType_, obs, myErr);
  }

  if (!dec) {
    conCache.freeConnection(con);
    errCode = NoDecoder;
    err = myErr;
    return 0;
  }

  try {
    decCmd = new DecodeCommand(dec);
  } catch (...) {
    errCode = NoMem;
    err = "Out of memmory!";

    {
      Lock lck(mutex);
      decoderMgr.releaseDecoder(dec);
    }

    conCache.freeConnection(con);
  }

  return decCmd;
}

void DataSrcApp::releaseDecodeCommand(DecodeCommand *command) {
  DecoderBase *dec;

  if (!command) {
    LOGDEBUG("releaseDecodeCommand: ERROR command==0!");
    return;
  }

  dec = command->getDecoder();

  if (!dec) {
    LOGDEBUG("releaseDecodeCommand: ERROR decoder==0!");
    delete command;
    return;
  }

  conCache.freeConnection(dec->getConnection());

  {
    Lock lck(mutex);
    decoderMgr.releaseDecoder(dec);
  }

  delete command;
}

std::string DataSrcApp::getMessageId(std::string &obstype) {
  boost::smatch match;

  if (!boost::regex_match(obstype, match, reMessageid))
    return "";

  string id = match[2];
  obstype.replace(obstype.find(match[1]), match[1].length(), "");

  return boost::trim_copy(id);
}

DecodeCommand*
DataSrcApp::decode(const char *obsType, const char *data, const std::string &logid, kvalobs::datasource::Result *res) {
  namespace kd = kvalobs::datasource;
  DecodeCommand *decCmd;
  DataSrcApp::ErrCode errCode;
  string errMsg;

  try {
    decCmd = this->create(obsType, data, 60000, errCode, errMsg);
  } catch (const std::exception &ex) {
    IDLOGERROR(logid, "Exception: " << ex.what() << " <" << obsType << ">.\nData <"<< data << ">.");
    decCmd = 0;
  } catch (...) {
    IDLOGERROR(logid, "Exception:  <" << obsType << ">.\nData <"<< data << ">.");
    decCmd = 0;
  }

  if (!decCmd) {
    if (errCode == DataSrcApp::NoDbConnection) {
      res->res = kd::EResult::NOTSAVED;
      res->message = errMsg;
    } else if (errCode == DataSrcApp::NoMem) {
      res->res = kd::EResult::ERROR;
      res->message = "Internal server error.";
    } else if (errCode == DataSrcApp::NoDecoder) {
      res->res = kd::EResult::NODECODER;
      res->message = errMsg;
    } else {
      res->res = kd::EResult::ERROR;
      res->message = errMsg;
    }
    return nullptr;
  }

  return decodeExecute(decCmd, res, logid);
}

DecodeCommand* DataSrcApp::decodeExecute(DecodeCommand *decCmd, kvalobs::datasource::Result *res, const std::string &logid) {
  namespace kd = kvalobs::datasource;
  DecodeCommand *decCmdRet = nullptr;

  try {
    while (!decoderExecutor.execute(decCmd, std::chrono::milliseconds(250))) {
      if (inShutdown())
        decoderExecutor.shutdown();
    }
  } catch (const miutil::concurrent::QueueSuspended &ex) {
    this->releaseDecodeCommand(decCmd);
    res->res = kd::EResult::ERROR;
    res->message = "SHUTDOWN: In the proccess of shuting down!";
    IDLOGINFO(logid, "Shutdown in proccess!");
    return nullptr;
  }

  while (!decCmdRet) {
    decCmdRet = decCmd->wait(250);

    if (decCmdRet)
      break;

    if (this->inShutdown()) {
      // We try to remove the command from the que before it
      // is received by the thread. If we can get it back we
      // release the command and returns to the caller.
      //
      // If we can't remove the command we have to wait for the
      // thread to return the command back before we can procced.

      IDLOGINFO(logid, "Shutdown in proccess!");
      if (decoderExecutor.remove(decCmd)) {
        IDLOGDEBUG(logid, "SHUTDOWN: Removed the command from the que.");
        this->releaseDecodeCommand(decCmd);
        res->res = kd::EResult::ERROR;
        res->message = "SHUTDOWN: In the proccess of shuting down!";
        return nullptr;
      }
    }
  }
  return decCmdRet;
}

kvalobs::datasource::Result DataSrcApp::newObservation(const char *obsType_, const char *data_, const std::string &logid) {
  namespace kd = kvalobs::datasource;
  kd::Result res;
  DecodeCommand *decCmdRet;
  const char *data = data_;
  std::string obsType = obsType_;
  string redirectedData;
  string redirectedObsType;
  bool redirect = false;
  boost::shared_ptr<kvalobs::decoder::RedirectInfo> redirected;
  res.messageId = getMessageId(obsType);

  do {
    redirect = false;
    decCmdRet = decode(obsType.c_str(), data, logid, &res);

    if (!decCmdRet)
      return res;

    kvalobs::decoder::DecoderBase::DecodeResult decodeResult = decCmdRet->getResult();
    redirect = false;

    if (decodeResult == kvalobs::decoder::DecoderBase::Redirect) {
      redirected.reset(decCmdRet->getRedirctInfo());
      if (redirected) {
        string redirectedFromDecoder = redirected->decoder();
        redirect = true;
        redirectedObsType = redirected->obsType();
        redirectedData = redirected->data();
        obsType = redirectedObsType.c_str();
        data = redirectedData.c_str();

        this->releaseDecodeCommand(decCmdRet);
        IDLOGDEBUG(logid, "Decode: Redirect <"<< obsType << ">, redirected from: " << redirectedFromDecoder << ".\nData: " << data << ".");
      } else {
        IDLOGDEBUG(logid, "Decode: Redirect, format error.");
        decCmdRet->setMsg("Redirected: Missing data.");
        res.res = kd::EResult::DECODEERROR;
      }

    } else if (decodeResult == kvalobs::decoder::DecoderBase::Ok) {
      IDLOGDEBUG(logid, "Decode: OK.");
      kvalobs::kvStationInfoList infoList = decCmdRet->getInfoList();
      list<kvalobs::serialize::KvalobsData> decodedData = decCmdRet->getDecodedData();
      this->sendInfoToManager(infoList, decodedData);
      res.res = kd::EResult::OK;
    } else if (decodeResult == kvalobs::decoder::DecoderBase::NotSaved) {
      IDLOGDEBUG(logid, "Decode: NotSaved.");
      res.res = kd::EResult::NOTSAVED;
    } else {
      res.res = kd::EResult::DECODEERROR;

      if (decodeResult == kvalobs::decoder::DecoderBase::Error) {
        string::size_type i = decCmdRet->getMsg().find("NODECODER:");
        if (i != string::npos) {
          res.res = kd::EResult::NODECODER;
          IDLOGERROR(logid, "No decoder!");
        } else {
          IDLOGERROR(logid, "Unrecoverable error!");
        }
      } else {
        IDLOGDEBUG(logid, "Rejected.");
      }
    }
  } while (redirect);

  res.message = decCmdRet->getMsg();

  this->releaseDecodeCommand(decCmdRet);
  return res;
}

bool DataSrcApp::isOk() const {
  return ok;
}

bool DataSrcApp::inShutdown() const {
  return sigTerm;
}

void DataSrcApp::shutdown() {
  sigTerm = 1;
  decoderExecutor.shutdown();
  decoderExecutor.waitForTermination(std::chrono::seconds(60));
  kafkaRawStream.shutdown();
  kafkaRawStream.join(std::chrono::seconds(60));
}
