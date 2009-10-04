#include "ProcessControl.h"

#include <kvalobs/kvDataFlag.h>

#include "algorithms/scone.h"

/// All flag check chars set to  0x3F unless set explicitely in a configuration file.
/// The condition method searches through a controlinfo and see if any of the char/bits match
/// a flag value set in the config file. Also can test individual claes, e.g. Aflag[3] (under development ???).



ProcessControl::
ProcessControl(){
   HexToInt['0']=0;
   HexToInt['1']=1;
   HexToInt['2']=2;
   HexToInt['3']=3;
   HexToInt['4']=4;
   HexToInt['5']=5;
   HexToInt['6']=6;
   HexToInt['7']=7;
   HexToInt['8']=8;
   HexToInt['9']=9;
   HexToInt['A']=10;
   HexToInt['B']=11;
   HexToInt['C']=12;
   HexToInt['D']=13;
   HexToInt['E']=14;
   HexToInt['F']=15;
}


bool 
ProcessControl::
condition(kvalobs::kvControlInfo controlinfo, std::map<int, unsigned char> zflag){

  int filter=0;

  for (std::map<int, unsigned char>::const_iterator ik=zflag.begin(); ik != zflag.end(); ++ik) {
       if ( (*ik).second != controlinfo.cflag((*ik).first) ) ++filter;
  }

  if (!filter) {
      return true; }
  else{    
      return false; }
}

int 
ProcessControl::
setter( kvalobs::kvControlInfo &controlinfo, std::map<int, unsigned char> zflag ){

 int CC, II;

  for (std::map<int, unsigned char>::const_iterator ik=zflag.begin(); ik != zflag.end(); ++ik) {
      CC=(*ik).first; 
      II=HexToInt[ (*ik).second ];  ///Review this !!!
      controlinfo.set( CC, II ); 
  }
   return 0;
}





// fcc -1 // fclim -1 // fcombi -1 // fcp -1 // fd 7 // fhqc -1 // fmis -1 // fnum -1 // fpos 8 // fpre -1 // fqclevel -1 // fr -1 // fs -1 // fstat -1 // ftime -1 // fw -1 // [0|0|0|0|0|0|0|0|0|0|0|0|0|0|0|0]


