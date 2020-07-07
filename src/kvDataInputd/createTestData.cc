#include <stdio.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <fstream>

using namespace std;

int main(int argc, char **argv) {
  boost::posix_time::ptime nowTime(
      boost::posix_time::microsec_clock::universal_time());
  boost::posix_time::ptime endTime;
  boost::posix_time::ptime itTime;
  char buf[64];
  float ta;
  ofstream ofs;

  ofs.open("18700.txt");

  nowTime = boost::posix_time::ptime(
      nowTime.date(),
      boost::posix_time::time_duration(nowTime.time_of_day().hours(), 0, 0));
  endTime = nowTime;

  endTime -= boost::gregorian::days(3);

  cerr << "Now:     " << nowTime << endl;
  cerr << "endTime: " << endTime << endl;

  ofs << "autoobs/nationalnr=018700/type=3" << endl;
  ofs << "TA" << endl;

  for (ta = -10.0, itTime = endTime; itTime <= nowTime; itTime +=
      boost::posix_time::hours(1), ta += 0.5) {
    sprintf(buf, "%4d%02d%02d%02d%02d%02d", static_cast<int>(itTime.date().year()),
            static_cast<int>(itTime.date().month()), static_cast<int>(itTime.date().day()),
            static_cast<int>(itTime.time_of_day().hours()), static_cast<int>(itTime.time_of_day().minutes()),
            static_cast<int>(itTime.time_of_day().seconds()));

    ofs << buf << "," << ta << endl;
  }

  ofs.close();

}

