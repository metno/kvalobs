#ifndef __ReadProgramOptions_h__
#define __ReadProgramOptions_h__

#include <boost/program_options.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/tokenizer.hpp>

#include <vector>
#include <map>
#include <puTools/miTime.h>

#include<string>

///Selects and reads the configuration files driving each of the Qc2 algorithms.

class ReadProgramOptions{

public:
ReadProgramOptions();
~ReadProgramOptions(){};
//std::vector<miutil::miTime> UT0;
//std::vector<miutil::miTime> UT1;
miutil::miTime UT0;
miutil::miTime UT1;

int StepD;
int StepH;
int AlgoCode;
int InterpCode;
int LastN;
std::string ControlInfoString;
std::vector<int> ControlInfoVector;

std::string NeighbourFilename;

int RunAtMinute;
int RunAtHour;
int pid;
int tid;
int missing;
int MinimumValue;
float InterpolationLimit;
bool newfile;
std::map<int, unsigned char> zflag;
std::map<int, unsigned char> Rflag;
std::map<int, unsigned char> Iflag;
std::map<int, unsigned char> Aflag;
std::map<int, unsigned char> Wflag;
std::map<int, unsigned char> Sflag;

std::vector<unsigned char> Vfqclevel,Vfr,Vfcc,Vfs,Vfnum,Vfpos,Vfmis,Vftime,Vfw,Vfstat,Vfcp,Vfclim,Vfd,Vfpre,Vfcombi,Vfhqc;

int Parse(std::string filename);
int SelectConfigFiles(std::vector<std::string>& config_files);
int clear();
};


/** @} */
#endif
