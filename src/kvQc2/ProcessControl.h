#ifndef _PROCESS_CONTROL_
#define _PROCESS_CONTROL_


#include <kvalobs/kvData.h>
#include <vector>
#include <map>

#include "ReadProgramOptions.h"


class ProcessControl{

public:

   ProcessControl();
   bool condition( kvalobs::kvControlInfo controlinfo, std::map<int, unsigned char> vlag );
   int setter( kvalobs::kvControlInfo &controlinfo, std::map<int, unsigned char> vlag );

   bool true_nibble( kvalobs::kvControlInfo controlinfo, std::map<int, unsigned char> vlag, int vindex, bool flagbool );

protected:

private:
   std::map<unsigned char,int> HexToInt;

};


#endif 
