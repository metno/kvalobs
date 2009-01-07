#ifndef __Qc2Thread_h__
#define __Qc2Thread_h__


#include <kvalobs/kvStationInfo.h>
#include <kvalobs/kvStation.h>
#include <string>
#include <stack>


class Qc2App;

/// The main Qc2 thread.

class Qc2Work
{
    Qc2App & app;
    std::string logpath_;

  public:
    Qc2Work( Qc2App &app_, const std::string& logpath = "./log" );
    void operator() ();
  private:

};



#endif
