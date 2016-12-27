/*
 * kvservicetypes.cc
 *
 *  Created on: Dec 23, 2016
 *      Author: borgem
 */

#include "kvservicetypes.h"

namespace kvservice {

std::ostream& operator<< (std::ostream &o, const KvObsDataList &d){
  o << "---- KvObsDataList: created: "  << d.created << " ----- \n";
  for( auto it = d.begin(); it != d.end(); ++it) {
    o << "---- BEGIN stationid: " << it->stationid() << " -----\n";
    for( auto dit=it->dataList().begin(); dit!=it->dataList().end(); ++dit)
      o << *dit << "\n";

    for( auto dit=it->textDataList().begin(); dit!=it->textDataList().end(); ++dit)
          o << *dit << "\n";

    o << "---- END stationid: " << it->stationid() << " -----\n";
  }
  o << "---- END of: KvObsDataList: created: "  << d.created << " ----- \n";
  return o;
}
}
