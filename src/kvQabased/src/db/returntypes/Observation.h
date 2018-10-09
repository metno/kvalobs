#ifndef OBSERVATION_H_
#define OBSERVATION_H_

// #include <boost/date_time/posix_time/posix_time.hpp>

#include <kvalobs/kvStationInfo.h>
#include <ostream>

namespace qabase {

// typedef kvalobs::kvStationInfo Observation;

class Observation {
public:
    Observation(long long id, int stationid, int type, const boost::posix_time::ptime & obstime, const boost::posix_time::ptime & tbtime)
    : id_(id), stationid_(stationid), type_(type), obstime_(obstime), tbtime_(tbtime)
    {}

    long long id() const { return id_; }
    int stationID() const { return stationid_; }
    int typeID() const { return type_; }
    const boost::posix_time::ptime & obstime() const { return obstime_; }
    const boost::posix_time::ptime & tbtime() const { return tbtime_; }

    kvalobs::kvStationInfo stationInfo() const {
        return kvalobs::kvStationInfo(stationID(), obstime(), typeID());
    }

private:
    long long id_;
    int stationid_;
    int type_;
    boost::posix_time::ptime obstime_;
    boost::posix_time::ptime tbtime_;
};

inline std::ostream & operator<<(std::ostream & s, const Observation & o) {
    return s << o.id() << '(' << o.stationID() << '/' << o.typeID() << '/' << o.obstime() << ')';
}

}


#endif