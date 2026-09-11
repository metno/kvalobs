/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 Copyright (C) 2015 met.no

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

#include "Producer.h"
#include <iostream>
#include <algorithm>

namespace kvalobs {
namespace subscribe {


Producer::Producer(const std::string & topic,
                         ErrorHandler onFailedDelivery,
                         SuccessHandler onSuccessfulDelivery):
                         topic_(topic),
                         onFailedDelivery_(onFailedDelivery),
                         onSuccessfulDelivery_(onSuccessfulDelivery) {
  std::cerr << "Creating Producer for topic: " << topic_ << std::endl;
  bool isKvalobsTopic_ = topic_.find("kvalobs.") == 0;
  std::cerr << "Creating Producer for topic: " << topic_ << " isKvalobs: " << (isKvalobsTopic_ ? "true" : "false") << std::endl;
  if(!isKvalobsTopic_) {
    throw std::logic_error("Topic does not start with 'kvalobs.': " + topic_);
  }

  std::size_t i = topic_.find('.');
  if (i != std::string::npos) {
    environment_ = topic_.substr(i + 1);
    i = environment_.find(".");
    if (i != std::string::npos) {
      environment_.erase(i);
    }
  }

  std::cerr << "Environment extracted from topic: '" << environment_ << "'" << std::endl;
  if(environment_!="production" &&
     environment_!="staging" && 
     environment_!="development") {
    throw std::logic_error("Invalid environment in topic name : '" + environment_ + "'");
  }

  validTopics_.clear();
  validTopics_.push_back("kvalobs."+environment_+".raw");
  validTopics_.push_back("kvalobs."+environment_+".checked");
  validTopics_.push_back("kvalobs."+environment_+".checked.lowpri");

  if(std::find(validTopics_.begin(), validTopics_.end(), topic_) == validTopics_.end()) {
    throw std::logic_error("Invalid topic name #: " + topic_);
  }
}

std::string Producer::topic() const {
  return topic_;
}

std::string Producer::topic(QueueType queue) const {
  switch(queue) {
    case raw:
      return "kvalobs." + environment_ + ".raw";
    case checked:
      return "kvalobs." + environment_ + ".checked";
    case checked_lowpri:
      return "kvalobs." + environment_ + ".checked.lowpri";
    default:
      throw std::logic_error("Unknown queue type");
  }
}



} // namespace subscribe
} // namespace kvalobs
