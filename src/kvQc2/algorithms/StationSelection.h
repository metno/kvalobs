/****************************************************************
**
** Definition of Station Selection class
**
****************************************************************/

#ifndef _STATION_SELECTION_
#define _STATION_SELECTION_

#include <kvalobs/kvStation.h>
#include <vector>
#include <list>
#include <map>
#include <milog/milog.h>



class StationSelection
{
public:

    StationSelection();
    StationSelection(std::list<int> StationList);
    ~StationSelection(){};

    std::list<int> Neighbours(int stationid);
    std::map<int, std::list<int> > ReturnMap();

protected:

private:
    std::map<int, std::list<int> > NeighbourMap;

};


#endif 
