#include "ProcessControl.h"

#include <kvalobs/kvDataFlag.h>

#include "algorithms/scone.h"

/// All flag check chars not set unless set explicitely in a configuration file, the values set
/// are carried around in a map with the index pair corresponding  to the nibble position. This
/// is handled by ReadProgramOptions.cc
/// The condition method searches through a controlinfo and see if any of the char/bits match
/// a flag value set in the config file. 

/// Also can test individual claes, e.g. Aflag[3] (under development ???).



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

/// If any of the members of the flag group zflag match the controlinfo TRUE is returned.
bool 
ProcessControl::
condition(kvalobs::kvControlInfo controlinfo, std::map<int, unsigned char> zflag){

  int filter=0;

  for (std::map<int, unsigned char>::const_iterator ik=zflag.begin(); ik != zflag.end(); ++ik) {
       if ( (*ik).second == controlinfo.cflag((*ik).first) ) ++filter;
  }

  if (filter) {
      return true;}
  else{    
      return false;}
}

bool
ProcessControl::
true_nibble( kvalobs::kvControlInfo controlinfo, unsigned char nibble ){
      
      return true;
}

/// Any values set in the flag group zflag are written into Controlinfo
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


