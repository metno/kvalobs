#include <sstream>
#include "Option.h"
#include <miutil/commastring.h>
#include <miutil/timeconvert.h>
#include <miconfparser/miconfparser.h>
#include <kvalobs/kvPath.h>


namespace pt=boost::posix_time;
namespace gd=boost::gregorian;
namespace mic=miutil::conf;
namespace kv=kvalobs;
using std::endl;
using std::cerr;
using std::cout;
using std::ostringstream;

namespace {
Options::List readIntList(const char *opt);
boost::posix_time::ptime readDate(const char *opt);
std::shared_ptr<miutil::conf::ConfSection> getConfiguration(const std::string &path_);
}

void Use(int exitcode, const std::string &msg) {
  cerr
      << "kvull, a helper program to pulling data from kvalobs"
      << endl
      << "May also subscribers to kvalobs data.!"
      << endl
      << endl
      << "Use"
      << endl
      << "   kvpull [-h] [-q] [-k] [-i typeidlist] [-c confile] -s stationlist -f fromdate [-t todate]"
      << endl << "\t-h            Print this help screen and exit!" << endl
      << "\t-k subscribe on data. To stop enter CTRL-C."
      << "\t-c confile    Use thit confile instead of the defaults "<< endl
      << "\t\t" << kv::kvPath(kv::sysconfdir) << "/kvpull.conf or \n\t\t"<< kv::kvPath(kv::sysconfdir) << "/kvalobs.conf." << endl
      << "\t-q            Trigger a new QA check and a push to subscribers."
      << endl
      << "\t-i typeidlist The typeid(s) of the data to push. This is a list"
      << endl << "\t              on the form typeid1,typeid2,typeid3" << endl
      << "\t              There must be no space between the the commas" << endl
      << "\t              and the typid(s)." << endl
      << "\t              All types is used if the list is empty or if " << endl
      << "\t              -i is omited." << endl
      << "\t-s stationlist A list of stations to push data form. The " << endl
      << "\t              The list must be on the form st1,st2,stN" << endl
      << "\t              There must be now space between the stations." << endl
      << "\t-f fromdate   Push data from this date." << endl
      << "\t-t todate     Push data to this date. If omitted " << endl
      << "\t              current time is used." << endl << endl
      << "Date format: " << endl
      << "   'YYYY-MM-DDThh:mm:ss', mm:ss in the time part can be omitted." << endl
      << "    Fromdate can also be given as number of hours back in time. Ex -f 3 as " << endl
      << "    3 hours back in time from current time."
      << endl << endl;

      if( msg.size() > 0 ) {
        cerr << msg << endl << endl;
      }

  exit(exitcode);
}

void ParseOpt(int argn, char **argv, Options *opt) {
  int ch;

  while ((ch = getopt(argn, argv, "hqki:s:t:f:c:")) > -1) {
    switch (ch) {
      case 'h':
        Use(0);
        break;
      case 'c':
        opt->conf=getConfiguration(optarg);
        break;
      case 'q':
        opt->doQa = true;
        break;
      case 'i':
        opt->typeids=readIntList(optarg);
        break;
      case 's':
        opt->stations=readIntList(optarg);
        break;
      case 't':
        opt->totime=readDate(optarg);
        break;
      case 'f':
        opt->fromtime=readDate(optarg);
        break;
      case 'k':
        opt->doSubscribe=true;
        break;
      default:
        ostringstream ost;

        ost << "Unknown argument: " << ch << ".";
        Use(1, ost.str());
    }
  }

  if( opt->fromtime.is_special() ) {
    opt->fromtime=readDate("1");
  }
}

std::ostream&
operator<<(std::ostream &ost, const Options &opt) {
  ost << "Station(s):";

  if (opt.stations.empty()) {
    ost << " All" << std::endl;
  } else {
    for (Options::CIList it = opt.stations.begin(); it != opt.stations.end();
        it++)
      ost << " " << *it;
    ost << std::endl;
  }

  ost << "Typeid(s):";

  if (opt.typeids.empty()) {
    ost << " All" << std::endl;
  } else {
    for (Options::CIList it = opt.typeids.begin(); it != opt.typeids.end();
        it++)
      ost << " " << *it;
    ost << std::endl;
  }

  ost << "Fromtime: " << opt.fromtime << std::endl;
  ost << "Totime:   " << (opt.totime.is_special()?"(Not specified)":pt::to_kvalobs_string(opt.totime)) << std::endl;

  return ost;
}


namespace {
Options::List readIntList(const char *opt) {
  miutil::CommaString cstr(opt);
  std::string buf;
  Options::List list;
  int n;
  
  for (int i = 0; i < cstr.size(); i++) {
    if (!cstr.get(i, buf))
      continue;

    if (sscanf(buf.c_str(), "%i", &n) != 1) {
      std::ostringstream ost;

      ost << "Not a number <" << buf << ">.";
      Use(1, ost.str());
    }

    list.push_back(n);
  }
  return list;
}

boost::posix_time::ptime readDate(const char *opt) {
  int year, mon, day, hour, min, sec;
  int n;
  pt::ptime date;
  n = sscanf(opt, "%d-%d-%dT%d:%d:%d", &year, &mon, &day, &hour, &min, &sec);

  if (n < 4) {
    if( n == 1 ) {
      //Accept this as numbers of hours back we want to get data.
      n=abs(year);
      date=pt::second_clock::universal_time();
      date -= pt::hours(n);
      auto td=date.time_of_day();
      date=pt::ptime(date.date(), pt::time_duration(td.hours(), 0, 0));
      return date;
    }

    std::ostringstream ost;
    ost << "Invalid timespec: <" << opt << ">.";
    Use(1, ost.str());
  }

  switch (n) {
    case 6:
      date = pt::ptime(gd::date(year, mon, day), pt::time_duration(hour, min, sec));
      break;
    case 5:
      date = pt::ptime(gd::date(year, mon, day), pt::time_duration(hour, min, 0));
      break;
    case 4:
      date = pt::ptime(gd::date(year, mon, day), pt::time_duration(hour, 0, 0));
      break;
  }
  return date;
}

std::shared_ptr<miutil::conf::ConfSection> getConfiguration(const std::string &path_) {
  std::shared_ptr<mic::ConfSection> conf;
  std::string path(path_);
  try {
    conf.reset(mic::ConfParser::parse(path));
  }
  catch( const std::exception &e){
    ostringstream ost;
    ost << "Failed to read file '"<< path <<"'. " << e.what() << ".";
    Use(1, ost.str());
  }
  return conf;
}


}
