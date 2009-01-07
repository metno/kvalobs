#ifndef __Distribute_h__
#define __Distribute_h__

#include <kvalobs/kvDbBase.h>
#include <kvalobs/kvDataFlag.h>
#include <kvalobs/kvStation.h>
#include <kvalobs/kvData.h>
#include <vector>
#include <map>


#include "ReadProgramOptions.h"
#include "ProcessControl.h"

//#include "vecmap.h"

///The class manages the redistribution of 24 hour precipitation data. 

class Distribute{

	
private:
  ProcessControl ControlFlag;

public:

  Distribute( const std::list<kvalobs::kvStation> & slist, ReadProgramOptions params );
  ~Distribute(){}
  //~Distribute(){
    //dst_data.clear();                
    //dst_intp.clear();                
    //dst_corr.clear();                
    //dst_newd.clear();                
    //dst_tbtime.clear();                
    //dst_time.clear();                
    //d_sensor.clear();                
    //d_level.clear();                
    //d_typeid.clear();                
    //d_controlinfo.clear();                
    //d_useinfo.clear();                
    //d_cfailed.clear();                
   //}

  ReadProgramOptions params;

  std::map<int, std::vector<float> > dst_data;
  std::map<int, std::vector<float> > dst_intp;
  std::map<int, std::vector<float> > dst_corr;
  std::map<int, std::vector<float> > dst_newd;

  std::map<int, std::vector<miutil::miTime> >    dst_time;
  std::map<int, std::vector<miutil::miTime> >    dst_tbtime;

  std::map<int, std::vector<int> > d_sensor;
  std::map<int, std::vector<int> > d_level;
  
  std::map<int, std::vector<int> >    d_typeid;
  std::map<int, std::vector<kvalobs::kvControlInfo> > d_controlinfo;
  std::map<int, std::vector<kvalobs::kvUseInfo> > d_useinfo;
  std::map<int, std::vector<miutil::miString> > d_cfailed;

  void add_element(int & sid, float & data, float & intp, float & corr, float & newd, miutil::miTime & tbtime, miutil::miTime & time, int & sensor, int & level, int & d_tid, kvalobs::kvControlInfo & d_control, kvalobs::kvUseInfo & d_use, miutil::miString & cfailed);
  void clean_station_entry( int & sid );

  void RedistributeStationData( int & sid , std::list<kvalobs::kvData>& ReturnData );

  void clear_all();
  
};

/** @} */
#endif
