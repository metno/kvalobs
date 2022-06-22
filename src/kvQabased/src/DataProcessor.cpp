/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 Copyright (C) 2010 met.no

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

#include "DataProcessor.h"
#include "db/returntypes/Observation.h"
#include "CheckRunner.h"
#include "QaBaseApp.h"
#include "kvsubscribe/KafkaProducer.h"
#include "decodeutility/kvalobsdataserializer.h"
#include "decodeutility/kvalobsdataparser.h"
#include "decodeutility/kvalobsdata.h"
#include "milog/milog.h"
#include "kvalobs/kvPath.h"
#include "miutil/LogAppender.h"
#include "miutil/makeUniqueFile.h"
#include "miutil/timeconvert.h"
#include <set>
#include <string>
#include <thread>
#include <fstream>

namespace qabase {
namespace {
// Must be protected by mutex if we want to make this thread-safe
std::set<kvalobs::subscribe::KafkaProducer::MessageId> messages;
}

bool DataProcessor::logXml = false;

bool DataProcessor::logTransactions = true;
unsigned DataProcessor::maxKafkaSendErrors=std::numeric_limits<unsigned>::max();

DataProcessor::DataProcessor(std::shared_ptr<qabase::CheckRunner> checkRunner)
    : checkRunner_(checkRunner),
      logCreator_(QaBaseApp::baseLogDir()),
      output_(QaBaseApp::kafkaProducer()) {
}

DataProcessor::~DataProcessor() {
}

qabase::CheckRunner::KvalobsDataPtr DataProcessor::runChecks(const qabase::Observation & obs) const {
  qabase::LogFileCreator::LogStreamPtr logStream = logCreator_.getLogStream(obs.stationInfo());
  qabase::CheckRunner::KvalobsDataPtr modified(checkRunner_->newObservation(obs, logStream.get()));
  return modified;
}

namespace {
  // There is a problem where garbage is received from kafka. This is an attempt to verify
  // that we don't send garbage on the kafka queue. To verify we serialize to xlm and try to 
  // deserialize it again. If we cant deserialize. We log it and try the cycle again. We retry
  // 5 times before we give up, but we send the garabage anyway :-).
  //
  // This does not solve the real problem. What is the cause. Some sort of wild pointer (?), but we 
  // will at least isolate where the problem may be someway.

  void writeToFile(std::string *file, bool ok, const qabase::Observation & obs, const qabase::CheckRunner::KvalobsDataPtr dataList, const std::string &xml, const std::string &ex=""){
    using namespace std;
    using namespace miutil;
    namespace pt=boost::posix_time;
    bool writeData=false;
    string sOk="ok";

    if( ! ok ) {
      sOk="failed";
    }
    
    if( file->empty() ) {
      ostringstream o;
      o << "qabase/" << obs.stationID() << "-" << obs.typeID() << "-" << pt::to_kvalobs_string(obs.obstime(), 'T');
      string prefix=o.str();
      string endsWith=".error";
      writeData=true;
      
      if( ok ) {
       endsWith=".ok";
      } 

      try {
        *file = makeUniqueFile(prefix, endsWith, kvalobs::kvPath(kvalobs::logdir));
      }
      catch( const exception &e) {
        LOGWARN("serialize2xml: Error when creating logfile: " << e.what());
        return;
      }
    }

    ofstream f(file->c_str(), ofstream::out|ofstream::binary|ofstream::app);
    if( !f.is_open() ) {
      LOGWARN("serialize2xml: Failed to open file: '" << *file << "'.");
      return;
    }

    if( writeData) {
      f << "Data to serialize:\n" << *dataList << "\n\n";
    }
    
    if( !ex.empty() ) {
      f << "Exception: " << ex << "\n";
    }

    f << "----- BEGIN XML (" << sOk << ") -----\n" << xml  << "\n----- END XML ------\n";
    f.close();
  }
  
  std::string serializeToXml(const qabase::Observation & obs, const qabase::CheckRunner::KvalobsDataPtr dataList, bool logXml) {
    if (!DataProcessor::logTransactions)
      return kvalobs::serialize::KvalobsDataSerializer::serialize(*dataList, "kvqabase");

    std::string xml;
    std::string fname;
    std::ofstream ofs;
    bool errorFileLogged = false;
    miutil::LogAppender log("kvQabased_serialize.log", kvalobs::kvPath(kvalobs::logdir));

    if( !log.isOk() ) {
      LOGWARN("serializeToXml: Failed to create logfile. " << log.lastError());
    }

    if( dataList->empty() ) {
      log.log("Empty datalist.");
      return "";
    }

    if( ! logXml ) {
      return kvalobs::serialize::KvalobsDataSerializer::serialize(*dataList, "kvqabase");
    }

    int i;
    for(  i=0; i<5; ++i) {
      xml=kvalobs::serialize::KvalobsDataSerializer::serialize(*dataList, "kvqabase");
      try {
        kvalobs::serialize::KvalobsData d;
        kvalobs::serialize::KvalobsDataParser::parse(xml, d);
        if( d.size() == 0 ) {
          writeToFile(&fname, false, obs, dataList, xml);
          if( !errorFileLogged) {
            std::ostringstream o;
            o <<   "FAILED Serialize (" << i << "): " << obs << ": " << fname; 
            log.log(o.str());
            errorFileLogged=true;
          }
        } else {
          std::ostringstream o;
          o << "OK Serialize (" << i <<"): "  << obs ;
          writeToFile(&fname, true, obs, dataList, xml);
          o << ": " <<  fname;
          log.log(o.str());
          return xml;
        }
      }
      catch( const std::exception &e) {
        writeToFile(&fname, false, obs, dataList, xml);
        if( !errorFileLogged) {
          std::ostringstream o;
            o << "FAILED Serialize (exception) (" << i << "): " << obs << ": " << fname; 
           log.log(o.str());
           errorFileLogged=true;
        }
      }
    }
    std::ostringstream o;
    o << "FAILED Serialize (" << i <<") to many retry: " << obs << ": " <<  fname;
    log.log(o.str());
    return xml;
  }
}



void DataProcessor::sendToKafka(const qabase::Observation & obs, const qabase::CheckRunner::KvalobsDataPtr dataList, bool * stop) {
  int sendAttempts = 0;

  if( !dataList || dataList->empty()) {
     return;
  }

  do {
    auto xml=serializeToXml(obs, dataList, logXml);
    if( xml.empty()) {
      return;
    }
    auto messageid = output_->send(xml);
    messages.insert(messageid);
    finalizeMessage_();

    if (messages.find(messageid) == messages.end()) {
      if (sendAttempts > 0)
        LOGWARN("Successfully sent data after " << sendAttempts << " retries");
      break;
    } else {
      if ((sendAttempts % 10) == 0) {
        LOGWARN("Could not send data to Kafka. Send queue size=" << messages.size() << ". Retrying (attempts #" << sendAttempts <<") ... ");
      }

      if( sendAttempts > maxKafkaSendErrors ) {
        LOGWARN("Terminating: Could not send data to Kafka. Send queue size=" << messages.size() << ". Tried " << sendAttempts << " times before giving up!");
        exit(16);
      }
      sendAttempts++;
      std::this_thread::sleep_for(std::chrono::seconds(2));
    }
  } while (stop == nullptr || *stop == false);
}

void DataProcessor::process(const kvalobs::kvStationInfo & si) {
  qabase::CheckRunner::KvalobsDataPtr modified(checkRunner_->newObservation(si));

  Observation dummyObs(0, si.stationID(), si.typeID(), si.obstime(), boost::posix_time::second_clock::universal_time());
  sendToKafka(dummyObs, modified);
  finalizeMessage_();
}

// void DataProcessor::process(const std::string & message) {
//   kvalobs::serialize::KvalobsDataSerializer s(message);
//   const kvalobs::serialize::KvalobsData & data = s.toData();
//   auto modified = data.summary();

//   process(modified.begin(), modified.end());
// }

void DataProcessor::onKafkaSendSuccess(kvalobs::subscribe::KafkaProducer::MessageId id, const std::string & data) {
  if (messages.erase(id) == 0) {  // should never happen
    LOGWARN("Got confirmation for invalid message id! Data: <" + data + ">");
    IDLOGINFO("kafka", "Got confirmation for invalid message id! Data: <" + data + ">");
  } else {
    LOGDEBUG("Successfully sent data: <" + data + ">");
    IDLOGINFO("kafka", "Successfully sent data id:" << id );
  }
}

void DataProcessor::onKafkaSendError(kvalobs::subscribe::KafkaProducer::MessageId id, const std::string & data, const std::string & errorMessage) {
  LOGERROR("kafka: Could not send data to Kafka. ("  <<id <<"): " << errorMessage );
  IDLOGINFO("kafka","Could not send data to Kafka. ("  <<id <<"): " << errorMessage );
  LOGDEBUG("kafka: Could not send data to Kafka. ("  <<id <<"): " << errorMessage << "\nData: <" + data + ">");
}

void DataProcessor::finalizeMessage_() {
  output_->catchup(2000);
}

} /* namespace qabase */
