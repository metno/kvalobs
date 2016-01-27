/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id$                                                       

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

#include <stdio.h>
#include <strings.h>
#include <fstream>
#include <puTools/miTime.h>
#include <milog/milog.h>
#include <kvalobs/kvPath.h>
#include "App.h"

using namespace std;
using namespace kvservice;
using namespace CKvalObs::CDataSource;
using namespace CKvalObs;
using namespace miutil;

namespace {
std::string getConfValue(const char *key, miutil::conf::ConfSection *conf,
                         bool exitIfNotFound) {
  string ret;
  miutil::conf::ValElementList valelem;

  valelem = conf->getValue(key);

  if (valelem.size() < 1) {

    if (exitIfNotFound) {
      LOGFATAL("No <" << key << "> in the confile.");
      exit(1);
    } else {
      LOGWARN("No <" << key << "> in the confile.");
    }

    return ret;
  }

  ret = valelem[0].valAsString();

  if (ret.empty()) {

    if (exitIfNotFound) {
      LOGFATAL("No value for key <" << key << "> in the confile.");
      exit(1);
    } else {
      LOGWARN("No value for key <" << key << "> in the confile.");
    }
  }

  return ret;
}

CorbaServerConf getPathConfValue(const char *key,
                                 miutil::conf::ConfSection *conf,
                                 const string &defaultNameserver) {
  string val = getConfValue(key, conf, true);

  CorbaServerConf sc;

  if (!sc.decodeConfspec(val, defaultNameserver)) {
    LOGFATAL(
        "Invalid value <" << val << ">. Expecting" << " a value on the form path[@nameserver[:port]]. " << "where [] specify optional parts.");
    exit(1);
  }

  return sc;
}

milog::LogLevel getLogLevel(miutil::conf::ConfSection *conf) {
  string logLevel = getConfValue("loglevel", conf, false);
  cerr << "loglevel: " << logLevel << endl;
  if (strcasecmp("FATAL", logLevel.c_str()) == 0) {
    return milog::FATAL;
  } else if (strcasecmp("ERROR", logLevel.c_str()) == 0) {
    return milog::ERROR;
  } else if (strcasecmp("WARN", logLevel.c_str()) == 0) {
    return milog::WARN;
  } else if (strcasecmp("DEBUG", logLevel.c_str()) == 0) {
    return milog::DEBUG;
  } else if (strcasecmp("INFO", logLevel.c_str()) == 0) {
    return milog::INFO;
  } else if (strcasecmp("0", logLevel.c_str()) == 0) {
    return milog::FATAL;
  } else if (strcasecmp("1", logLevel.c_str()) == 0) {
    return milog::ERROR;
  } else if (strcasecmp("2", logLevel.c_str()) == 0) {
    return milog::WARN;
  } else if (strcasecmp("3", logLevel.c_str()) == 0) {
    return milog::INFO;
  } else if (strcasecmp("4", logLevel.c_str()) == 0) {
    return milog::DEBUG;
  } else {
    return milog::ERROR;
  }
}

}

App::App(int argn, char **argv, miutil::conf::ConfSection *conf)
    : kvservice::corba::CorbaKvApp(argn, argv, conf),
#warning This must be set in config!
      sendToKvServer("10.99.2.229:8090"),
      dataReceiverThread(0),
      corbaApp(kvservice::corba::CorbaKvApp::getCorbaApp()),
      dataReceiverAlive(false) {
  string defaultNameserver;
  milog::LogLevel loglevel;

  if (!corbaApp) {
    cerr << "Can't initialize CORBA!" << endl;
    exit(1);
  }

  defaultNameserver = getConfValue("corba.nameserver", conf, true);

  receiveFromKvServer = getPathConfValue("corba.path", conf, defaultNameserver);
  setNameservice(receiveFromKvServer.ns.toString());

  subscribeSetup();
  dataReceiverSetup();

  LOGINFO(
      "Receiving data from: " << receiveFromKvServer << endl << "Sending data to:     " << sendToKvServer.host());
}

App::~App() {
  dataReceiverThread.join(true);
}

bool App::subscribeSetup() {

  //Check if we allready has subscribed.
  if (!dataid.empty())
    return true;

//   notifyid = subscribeDataNotify( kvservice::KvDataSubscribeInfoHelper(), eventQue );
//
//   if( notifyid.empty() ){
//      cerr << "Cant subscribe to KvDataNotify!" << endl;
//      return false;
//   }else{
//      cerr << "Subscribe on KvDataNotify!" << endl;
//   }

  dataid = subscribeData(KvDataSubscribeInfoHelper(), eventQue);

  if (dataid.empty()) {
    LOGERROR("Cant subscribe to KvData!");
    return false;
  } else {
    LOGINFO("Subscribe on KvData!");
  }

  hintid = subscribeKvHint(eventQue);

  if (hintid.empty()) {
    LOGERROR("Cant subscribe to KvHint!");
    return false;
  } else {
    LOGINFO("Subscribe on KvHint!");
  }

  return true;
}

bool App::dataReceiverSetup() {
  DataReceiver *dr;

  try {
    dr = new DataReceiver(*this, eventQue);
  } catch (...) {
    return false;
  }

  //The dataReceiverThread take over the responsibility
  //for the (dr) pointer, ie delete it when it is no longer
  //needed.
  dataReceiverThread = DataReceiverThread(dr);

  //Start the thread and return.

  return dataReceiverThread.start();
}

CORBA::Object_ptr App::getRefInNS(const CorbaServerConf &serverSpec,
                                  const std::string &interface) {
  string path(serverSpec.name + "/" + interface);
  return corbaApp->getObjFromNS(path, serverSpec.ns);
}

ParamDefsPtr App::getParamdefs() {
  if (paramdefs && paramdefs->size() > 0)
    return paramdefs;

  LOGDEBUG("App::getParamdefs() getting paramdefs.");

  try {
    std::list<kvalobs::kvParam> kvParam;

    getKvParams(kvParam);

    paramdefs.reset(new ParamDefs());

    for (std::list<kvalobs::kvParam>::iterator it = kvParam.begin();
        it != kvParam.end(); ++it) {
      (*paramdefs)[it->paramID()] = it->name();
    }

    if (paramdefs->size() == 0)
      LOGWARN(
          "No(?) parameter defined on the kvalobs server <" << receiveFromKvServer << ">.");

    return paramdefs;
  } catch (...) {
    LOGWARN(
        "Can't get the parameter definitions from the" << " kvalobs server <" << receiveFromKvServer << ">.");
    return paramdefs;
  }

  //Make the compiler happy.
  return paramdefs;
}

int App::sendData(const std::string &decoder, const std::string &data) {
  int ret = -1;
  bool retry = true;
  Result *res;
  ostringstream log;

//  do {
//    if (!dataReceiverAlive && !timeLastTryToSendData.is_not_a_date_time()) {
//      boost::posix_time::ptime now =
//          boost::posix_time::microsec_clock::universal_time()
//              - boost::posix_time::minutes(5);
//
//      //If the kvalobs server receiving data is down,
//      //wait 5 minutes before retrying to send data to the server.
//      if (timeLastTryToSendData > now)
//        return -1;
//    }

//    dataReceiverAlive = false;
//    timeLastTryToSendData = boost::posix_time::microsec_clock::universal_time();

    try {
      auto result = sendToKvServer.newData(data, decoder);
      retry = false;
      switch (result.res) {
        case kvalobs::datasource::OK:
          ret = 0;
          break;
        case kvalobs::datasource::NODECODER:
        case kvalobs::datasource::DECODEERROR:
        case kvalobs::datasource::NOTSAVED:
        case kvalobs::datasource::ERROR:
        default:
          throw std::runtime_error(result.message);
      }
    }
    catch (std::exception & e) {
      LOGERROR("Message <" << data << ">, decoder <" << decoder << ">: " << e.what());
    }

//    try {
//      if (CORBA::is_nil(refDataReceiver)) {
//        LOGINFO("Looking up 'kvinput' on <"<<sendToKvServer <<">.");
//        refDataReceiver = Data::_narrow(getRefInNS(sendToKvServer, "kvinput"));
//
//        if (CORBA::is_nil(refDataReceiver)) {
//          LOGERROR("Can't find 'kvinput' on <"<< sendToKvServer <<">.");
//          return -2;
//        }
//        retry = true;
//      }
//
//      res = refDataReceiver->newData(data.c_str(), decoder.c_str());
//      retry = false;
//      dataReceiverAlive = true;
//
//      log << "Sending data to <kvinput> on <" << sendToKvServer
//          << ">. Decoder: " << decoder << ": ";
//      switch (res->res) {
//        case OK:
//          log << "OK";
//          ret = 0;
//          break;
//        case NODECODER:
//          log << "NODECODER: " << res->message;
//          ret = 1;
//          break;
//        case DECODEERROR:
//          log << "DECODEERROR: " << res->message;
//          ret = 2;
//          break;
//        case NOTSAVED:
//          log << "NOTSAVED: " << res->message;
//          ret = 3;
//          break;
//        case ERROR:
//          log << "ERROR: " << res->message;
//          ret = 4;
//          break;
//      }
//      if (res->res != OK) {
//        LOGERROR(log.str());
//      } else {
//        LOGINFO(log.str());
//      }
//    } catch (CORBA::COMM_FAILURE& ex) {
//      if (!retry) {
//        LOGERROR(
//            "Unable to contact the <kvDataInputd> at '" << sendToKvServer <<"'.");
//      }
//      if (retry) {
//        refDataReceiver = Data::_nil();
//        retry = false;
//      }
//    } catch (CORBA::SystemException &ex) {
//      LOGERROR(
//          "Unable to send data to <kvDataInputd>." << endl << "CORBA::SystemException. " << ex._name() << " (" << ex.NP_minorString()<<").");
//    } catch (CORBA::Exception &ex) {
//      LOGERROR(
//          "Unable to send data to <kvDataInputd>." << endl << "CORBA::Exception: " << ex._name());
//    } catch (omniORB::fatalException& fe) {
//      LOGERROR(
//          "Unable to send data to <kvDataInputd>." << endl << "omniORB::fatalException:" << endl << "  file: " << fe.file() << endl << "  line: " << fe.line() << endl << "  mesg: " << fe.errmsg());
//    } catch (...) {
//      LOGERROR(
//          "Unable to send data to <kvDataInputd>." << endl << "Unknown exception.");
//    }
//  } while (retry);

  return ret;
}

void App::createLogger(miutil::conf::ConfSection *conf) {
  using namespace milog;
  string filename;
  LogLevel traceLevel = milog::DEBUG;
  LogLevel logLevel = getLogLevel(conf);
  FLogStream *fs;
  StdErrStream *trace;

  filename = kvalobs::kvPath(kvalobs::logdir) + "/kv2kvDataInputd.log";

  try {
    fs = new FLogStream(4);

    if (!fs->open(filename)) {
      std::cerr << "FATAL: Can't initialize the Logging system.\n";
      std::cerr << "------ Cant open the Logfile <" << filename << ">\n";
      delete fs;
      exit(1);
    }

    trace = new StdErrStream();

    if (!LogManager::createLogger("default", trace)) {
      std::cerr << "FATAL: Can't initialize the Logging system.\n";
      std::cerr << "------ Cant create logger\n";
      exit(1);
    }

    if (!LogManager::addStream("default", fs)) {
      std::cerr << "FATAL: Can't initialize the Logging system.\n";
      std::cerr << "------ Cant add filelogging to the Logging system\n";
      exit(1);
    }

    trace->loglevel(traceLevel);
    fs->loglevel(logLevel);

    LogManager::setDefaultLogger("default");
  } catch (...) {
    std::cerr << "FATAL: Can't initialize the Logging system.\n";
    std::cerr << "------ OUT OF MEMMORY!!!\n";
    exit(1);
  }

  std::cerr << "Logging to file <" << filename << ">!\n";
}

void App::run() {
  if (!subscribeSetup())
    return;

  if (!dataReceiverSetup())
    return;

  kvservice::corba::CorbaKvApp::run();
}

