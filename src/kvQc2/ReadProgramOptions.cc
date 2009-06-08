/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id$                                                       

  Copyright (C) 2007 met.no

  Contact information:
  Norwegian Meteorological Institute
  Box 43 Blindern
  0313 OSLO
  NORWAY
  email: kvalobs-dev@met.no

  This file is part of KVALOBS

  KVALOBS is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License as 
  published by the Free Software Foundation; either version 2 
  of the License, or (at your option) any later version.
  
  KVALOBS is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.
  
  You should have received a copy of the GNU General Public License along 
  with KVALOBS; if not, write to the Free Software Foundation Inc., 
  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include <iostream>
#include <sstream>
#include <math.h>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>

#include <time.h>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include "ReadProgramOptions.h"

using namespace std;

namespace po = boost::program_options;
namespace fs = boost::filesystem;


ReadProgramOptions::ReadProgramOptions()
{
   newfile=false;
}


///Scans $KVALOBS/Qc2Config and searched for configuration files "*.cfg". 

int 
ReadProgramOptions::
SelectConfigFiles( std::vector<string>& config_files)
{
     
       //std::string filename;
       //filename="gordon";
       //config_files.push_back(filename);

try{
       char *pKv=getenv("KVALOBS"); 
       fs::path config_path(string(pKv)+"/Qc2Config/");
       
       // scan for files
       std::cout << "Scanning For Files" << std::endl;   
       fs::directory_iterator end_iter; // By default this is pass the end of a directory !
       fs::path full_path( fs::initial_path() );
       full_path = fs::system_complete( config_path );
       
       std::string filename;

       std::cout << full_path << std::endl;

       if ( !fs::exists( full_path ) ) {
           //std::cout << "Does not exist: " << full_path.native_file_string() <<std::endl;
           std::cout << "Does not exist: " << full_path.file_string() <<std::endl;
       }
       else {
           for ( fs::directory_iterator dit( full_path ); dit != end_iter; ++dit ) {
              //std::cout << dit->path() << std::endl;
              //std::cout << dit->leaf() << std::endl;
              filename=dit->path().native_file_string();
              if ( filename.substr(filename.length()-3,3) == "cfg") {
                   config_files.push_back(filename);
                   std::cout << "Configuration File Found: " << filename << std::endl; 
              } 
           }
       }
   }
   catch(exception& ecfg) { 
       std::cout << "Problem with configuration files for Qc2" << std::endl;
       std::cout << ecfg.what() << std::endl;
       return 1;
   } 
return 0;
}


///Parses the configuration files.
int 
ReadProgramOptions::
Parse(std::string filename)
{
int StartYear, StartMonth, StartDay, StartHour, StartMinute, StartSecond;
int EndYear, EndMonth, EndDay, EndHour, EndMinute, EndSecond;
int StepYear, StepMonth, StepDay, StepHour, StepMinute, StepSecond;
int RunMinute;
int RunHour;
int ParamId;
int TypeId;
int MissingValue;
int MinValue;
float InterpolationDistance;
// Test control flag paramters
unsigned char z_fqclevel,z_fr,z_fcc,z_fs,z_fnum,z_fpos,z_fmis,z_ftime,z_fw,z_fstat,z_fcp,z_fclim,z_fd,z_fpre,z_fcombi,z_fhqc;
// CONTROL FLAGS FOR VARIOUS POSSIBLE FILTERS
unsigned char R_fqclevel,R_fr,R_fcc,R_fs,R_fnum,R_fpos,R_fmis,R_ftime,R_fw,R_fstat,R_fcp,R_fclim,R_fd,R_fpre,R_fcombi,R_fhqc; //READ
unsigned char I_fqclevel,I_fr,I_fcc,I_fs,I_fnum,I_fpos,I_fmis,I_ftime,I_fw,I_fstat,I_fcp,I_fclim,I_fd,I_fpre,I_fcombi,I_fhqc; //INTERPOLATE
unsigned char A_fqclevel,A_fr,A_fcc,A_fs,A_fnum,A_fpos,A_fmis,A_ftime,A_fw,A_fstat,A_fcp,A_fclim,A_fd,A_fpre,A_fcombi,A_fhqc; //ALGORITHM
unsigned char W_fqclevel,W_fr,W_fcc,W_fs,W_fnum,W_fpos,W_fmis,W_ftime,W_fw,W_fstat,W_fcp,W_fclim,W_fd,W_fpre,W_fcombi,W_fhqc; //WRITE
// CONTROL FLAGS TO SET 
unsigned char S_fqclevel,S_fr,S_fcc,S_fs,S_fnum,S_fpos,S_fmis,S_ftime,S_fw,S_fstat,S_fcp,S_fclim,S_fd,S_fpre,S_fcombi,S_fhqc; //SET

std::vector<unsigned char> V_fqclevel,V_fr,V_fcc,V_fs,V_fnum,V_fpos,V_fmis,V_ftime,V_fw,V_fstat,V_fcp,V_fclim,V_fd,V_fpre,V_fcombi,V_fhqc; //For algorithms which need multiple control options for the same flag

std::string ControlString;
std::vector<int> ControlVector;

try{
   po::variables_map vm;
   po::options_description config_file_options("Configuration File Parameters");
   // NOTE!! What follows is a very long line of code!!
   config_file_options.add_options()  
        ("RunAtMinute",po::value<int>(&RunMinute)->default_value(0),"Min at which to run (!could just use CRON)")
        ("RunAtHour",po::value<int>(&RunHour)->default_value(2),"Hour at which to run (!!!could just use CRON)")
        ("Start_YYYY",po::value<int>(&StartYear)->default_value(miutil::miTime::nowTime().year()),"Start Year")
        ("Start_MM",po::value<int> (&StartMonth)->default_value(miutil::miTime::nowTime().month()),"Start Month")
        ("Start_DD",po::value<int>  (&StartDay)->default_value(miutil::miTime::nowTime().day()), "Start Day")
        ("Start_hh",po::value<int>  (&StartHour)->default_value(6),  "Start Hour")
        ("Start_mm",po::value<int>  (&StartMinute)->default_value(0),"Start Minute")
        ("Start_ss",po::value<int>  (&StartSecond)->default_value(0),"Start Second")
        ("End_YYYY",po::value<int>  (&EndYear)->default_value(miutil::miTime::nowTime().year()),  "End Year")
        ("End_MM",po::value<int>    (&EndMonth)->default_value(miutil::miTime::nowTime().month()),"End Month")
        ("End_DD",po::value<int>    (&EndDay)->default_value(miutil::miTime::nowTime().day()), "End Day")
        ("End_hh",po::value<int>    (&EndHour)->default_value(6),    "End Hour")
        ("End_mm",po::value<int>    (&EndMinute)->default_value(0),  "End Minute")
        ("End_ss",po::value<int>    (&EndSecond)->default_value(0),  "End Second")

        ("Last_NDays",po::value<int>(&LastN)->default_value(-1),  "Last N Days to Run Algorithm")

        ("Step_YYYY",po::value<int>(&StepYear)->default_value(0),"Step Year")
        ("Step_MM",po::value<int>  (&StepMonth)->default_value(0),  "Step Minute")
        ("Step_DD",po::value<int>  (&StepDay)->default_value(0),  "Step Day")
        ("Step_hh",po::value<int>  (&StepHour)->default_value(0),  "Step Hour")
        ("Step_mm",po::value<int>  (&StepMinute)->default_value(0),  "Step Minute")
        ("Step_ss",po::value<int>  (&StepSecond)->default_value(0),  "Step Second")

        ("ParamId",po::value<int>  (&ParamId),  "Parameter ID")
        ("TypeId",po::value<int>  (&TypeId),  "Type ID")
        ("AlgoCode",po::value<int>  (&AlgoCode)->default_value(-1),  "Algoritham Code")
        ("InterpCode",po::value<int>  (&InterpCode)->default_value(-1),  "Code to determine method of interpolation")
        ("ControlString",po::value<std::string>  (&ControlString),  "Control Info")
        ("ControlVector",po::value<std::vector<int> > (&ControlVector),  "Control Vector")

        ("MissingValue",po::value<int>(&MissingValue)->default_value(-32767),  "Original Missing Data Value") /// could also rely on fmis here !!??
        ("MinValue",po::value<int>(&MinValue)->default_value(-32767),  "Minimum Data Value FOr Some Controls") 
        ("InterpolationDistance",po::value<float>(&InterpolationDistance)->default_value(0),  "Nearest Nieighboue Limiting Distance") 

        ("z_fqclevel",po::value<unsigned char>  (&z_fqclevel)->default_value(0x3F),  "fqclevel")
        ("z_fr",po::value<unsigned char>  (&z_fr)->default_value(0x3F),  "fr")
        ("z_fcc",po::value<unsigned char>  (&z_fcc)->default_value(0x3F),  "fcc")
        ("z_fs",po::value<unsigned char>  (&z_fs)->default_value(0x3F),  "fs")
        ("z_fnum",po::value<unsigned char>  (&z_fnum)->default_value(0x3F),  "fnum")
        ("z_fpos",po::value<unsigned char>  (&z_fpos)->default_value(0x3F),  "fpos")
        ("z_fmis",po::value<unsigned char>  (&z_fmis)->default_value(0x3F),  "fmis")
        ("z_ftime",po::value<unsigned char>  (&z_ftime)->default_value(0x3F),  "ftime")
        ("z_fw",po::value<unsigned char>  (&z_fw)->default_value(0x3F),  "fw")
        ("z_fstat",po::value<unsigned char>  (&z_fstat)->default_value(0x3F),  "fstat")
        ("z_fcp",po::value<unsigned char>  (&z_fcp)->default_value(0x3F),  "fcp")
        ("z_fclim",po::value<unsigned char>  (&z_fclim)->default_value(0x3F),  "fclim")
        ("z_fd",po::value<unsigned char>  (&z_fd)->default_value(0x3F),  "fd")
        ("z_fpre",po::value<unsigned char>  (&z_fpre)->default_value(0x3F),  "fpre")
        ("z_fcombi",po::value<unsigned char>  (&z_fcombi)->default_value(0x3F),  "fcombi")
        ("z_fhqc",po::value<unsigned char>  (&z_fhqc)->default_value(0x3F),  "fhqc")

        ("R_fqclevel",po::value<unsigned char>  (&R_fqclevel)->default_value(0x3F),  "fqclevel")
        ("R_fr",po::value<unsigned char>  (&R_fr)->default_value(0x3F),  "fr")
        ("R_fcc",po::value<unsigned char>  (&R_fcc)->default_value(0x3F),  "fcc")
        ("R_fs",po::value<unsigned char>  (&R_fs)->default_value(0x3F),  "fs")
        ("R_fnum",po::value<unsigned char>  (&R_fnum)->default_value(0x3F),  "fnum")
        ("R_fpos",po::value<unsigned char>  (&R_fpos)->default_value(0x3F),  "fpos")
        ("R_fmis",po::value<unsigned char>  (&R_fmis)->default_value(0x3F),  "fmis")
        ("R_ftime",po::value<unsigned char>  (&R_ftime)->default_value(0x3F),  "ftime")
        ("R_fw",po::value<unsigned char>  (&R_fw)->default_value(0x3F),  "fw")
        ("R_fstat",po::value<unsigned char>  (&R_fstat)->default_value(0x3F),  "fstat")
        ("R_fcp",po::value<unsigned char>  (&R_fcp)->default_value(0x3F),  "fcp")
        ("R_fclim",po::value<unsigned char>  (&R_fclim)->default_value(0x3F),  "fclim")
        ("R_fd",po::value<unsigned char>  (&R_fd)->default_value(0x3F),  "fd")
        ("R_fpre",po::value<unsigned char>  (&R_fpre)->default_value(0x3F),  "fpre")
        ("R_fcombi",po::value<unsigned char>  (&R_fcombi)->default_value(0x3F),  "fcombi")
        ("R_fhqc",po::value<unsigned char>  (&R_fhqc)->default_value(0x3F),  "fhqc")

        ("I_fqclevel",po::value<unsigned char>  (&I_fqclevel)->default_value(0x3F),  "fqclevel")
        ("I_fr",po::value<unsigned char>  (&I_fr)->default_value(0x3F),  "fr")
        ("I_fcc",po::value<unsigned char>  (&I_fcc)->default_value(0x3F),  "fcc")
        ("I_fs",po::value<unsigned char>  (&I_fs)->default_value(0x3F),  "fs")
        ("I_fnum",po::value<unsigned char>  (&I_fnum)->default_value(0x3F),  "fnum")
        ("I_fpos",po::value<unsigned char>  (&I_fpos)->default_value(0x3F),  "fpos")
        ("I_fmis",po::value<unsigned char>  (&I_fmis)->default_value(0x3F),  "fmis")
        ("I_ftime",po::value<unsigned char>  (&I_ftime)->default_value(0x3F),  "ftime")
        ("I_fw",po::value<unsigned char>  (&I_fw)->default_value(0x3F),  "fw")
        ("I_fstat",po::value<unsigned char>  (&I_fstat)->default_value(0x3F),  "fstat")
        ("I_fcp",po::value<unsigned char>  (&I_fcp)->default_value(0x3F),  "fcp")
        ("I_fclim",po::value<unsigned char>  (&I_fclim)->default_value(0x3F),  "fclim")
        ("I_fd",po::value<unsigned char>  (&I_fd)->default_value(0x3F),  "fd")
        ("I_fpre",po::value<unsigned char>  (&I_fpre)->default_value(0x3F),  "fpre")
        ("I_fcombi",po::value<unsigned char>  (&I_fcombi)->default_value(0x3F),  "fcombi")
        ("I_fhqc",po::value<unsigned char>  (&I_fhqc)->default_value(0x3F),  "fhqc")

        ("A_fqclevel",po::value<unsigned char>  (&A_fqclevel)->default_value(0x3F),  "fqclevel")
        ("A_fr",po::value<unsigned char>  (&A_fr)->default_value(0x3F),  "fr")
        ("A_fcc",po::value<unsigned char>  (&A_fcc)->default_value(0x3F),  "fcc")
        ("A_fs",po::value<unsigned char>  (&A_fs)->default_value(0x3F),  "fs")
        ("A_fnum",po::value<unsigned char>  (&A_fnum)->default_value(0x3F),  "fnum")
        ("A_fpos",po::value<unsigned char>  (&A_fpos)->default_value(0x3F),  "fpos")
        ("A_fmis",po::value<unsigned char>  (&A_fmis)->default_value(0x3F),  "fmis")
        ("A_ftime",po::value<unsigned char>  (&A_ftime)->default_value(0x3F),  "ftime")
        ("A_fw",po::value<unsigned char>  (&A_fw)->default_value(0x3F),  "fw")
        ("A_fstat",po::value<unsigned char>  (&A_fstat)->default_value(0x3F),  "fstat")
        ("A_fcp",po::value<unsigned char>  (&A_fcp)->default_value(0x3F),  "fcp")
        ("A_fclim",po::value<unsigned char>  (&A_fclim)->default_value(0x3F),  "fclim")
        ("A_fd",po::value<unsigned char>  (&A_fd)->default_value(0x3F),  "fd")
        ("A_fpre",po::value<unsigned char>  (&A_fpre)->default_value(0x3F),  "fpre")
        ("A_fcombi",po::value<unsigned char>  (&A_fcombi)->default_value(0x3F),  "fcombi")
        ("A_fhqc",po::value<unsigned char>  (&A_fhqc)->default_value(0x3F),  "fhqc")

        ("W_fqclevel",po::value<unsigned char>  (&W_fqclevel)->default_value(0x3F),  "fqclevel")
        ("W_fr",po::value<unsigned char>  (&W_fr)->default_value(0x3F),  "fr")
        ("W_fcc",po::value<unsigned char>  (&W_fcc)->default_value(0x3F),  "fcc")
        ("W_fs",po::value<unsigned char>  (&W_fs)->default_value(0x3F),  "fs")
        ("W_fnum",po::value<unsigned char>  (&W_fnum)->default_value(0x3F),  "fnum")
        ("W_fpos",po::value<unsigned char>  (&W_fpos)->default_value(0x3F),  "fpos")
        ("W_fmis",po::value<unsigned char>  (&W_fmis)->default_value(0x3F),  "fmis")
        ("W_ftime",po::value<unsigned char>  (&W_ftime)->default_value(0x3F),  "ftime")
        ("W_fw",po::value<unsigned char>  (&W_fw)->default_value(0x3F),  "fw")
        ("W_fstat",po::value<unsigned char>  (&W_fstat)->default_value(0x3F),  "fstat")
        ("W_fcp",po::value<unsigned char>  (&W_fcp)->default_value(0x3F),  "fcp")
        ("W_fclim",po::value<unsigned char>  (&W_fclim)->default_value(0x3F),  "fclim")
        ("W_fd",po::value<unsigned char>  (&W_fd)->default_value(0x3F),  "fd")
        ("W_fpre",po::value<unsigned char>  (&W_fpre)->default_value(0x3F),  "fpre")
        ("W_fcombi",po::value<unsigned char>  (&W_fcombi)->default_value(0x3F),  "fcombi")
        ("W_fhqc",po::value<unsigned char>  (&W_fhqc)->default_value(0x3F),  "fhqc")

        ("S_fqclevel",po::value<unsigned char>  (&S_fqclevel)->default_value(0x3F),  "fqclevel")
        ("S_fr",po::value<unsigned char>  (&S_fr)->default_value(0x3F),  "fr")
        ("S_fcc",po::value<unsigned char>  (&S_fcc)->default_value(0x3F),  "fcc")
        ("S_fs",po::value<unsigned char>  (&S_fs)->default_value(0x3F),  "fs")
        ("S_fnum",po::value<unsigned char>  (&S_fnum)->default_value(0x3F),  "fnum")
        ("S_fpos",po::value<unsigned char>  (&S_fpos)->default_value(0x3F),  "fpos")
        ("S_fmis",po::value<unsigned char>  (&S_fmis)->default_value(0x3F),  "fmis")
        ("S_ftime",po::value<unsigned char>  (&S_ftime)->default_value(0x3F),  "ftime")
        ("S_fw",po::value<unsigned char>  (&S_fw)->default_value(0x3F),  "fw")
        ("S_fstat",po::value<unsigned char>  (&S_fstat)->default_value(0x3F),  "fstat")
        ("S_fcp",po::value<unsigned char>  (&S_fcp)->default_value(0x3F),  "fcp")
        ("S_fclim",po::value<unsigned char>  (&S_fclim)->default_value(0x3F),  "fclim")
        ("S_fd",po::value<unsigned char>  (&S_fd)->default_value(0x3F),  "fd")
        ("S_fpre",po::value<unsigned char>  (&S_fpre)->default_value(0x3F),  "fpre")
        ("S_fcombi",po::value<unsigned char>  (&S_fcombi)->default_value(0x3F),  "fcombi")
        ("S_fhqc",po::value<unsigned char>  (&S_fhqc)->default_value(0x3F),  "fhqc")

        ("V_fqclevel",po::value<std::vector<unsigned char> >  (&V_fqclevel),  "fqclevel")
        ("V_fr",po::value<std::vector<unsigned char> >  (&V_fr),  "fr")
        ("V_fcc",po::value<std::vector<unsigned char> >  (&V_fcc),  "fcc")
        ("V_fs",po::value<std::vector<unsigned char> >  (&V_fs),  "fs")
        ("V_fnum",po::value<std::vector<unsigned char> >  (&V_fnum),  "fnum")
        ("V_fpos",po::value<std::vector<unsigned char> >  (&V_fpos),  "fpos")
        ("V_fmis",po::value<std::vector<unsigned char> >  (&V_fmis),  "fmis")
        ("V_ftime",po::value<std::vector<unsigned char> >  (&V_ftime),  "ftime")
        ("V_fw",po::value<std::vector<unsigned char> >  (&V_fw),  "fw")
        ("V_fstat",po::value<std::vector<unsigned char> >  (&V_fstat),  "fstat")
        ("V_fcp",po::value<std::vector<unsigned char> >  (&V_fcp),  "fcp")
        ("V_fclim",po::value<std::vector<unsigned char> >  (&V_fclim),  "fclim")
        ("V_fd",po::value<std::vector<unsigned char> >  (&V_fd),  "fd")
        ("V_fpre",po::value<std::vector<unsigned char> >  (&V_fpre),  "fpre")
        ("V_fcombi",po::value<std::vector<unsigned char> >  (&V_fcombi),  "fcombi")
        ("V_fhqc",po::value<std::vector<unsigned char> >  (&V_fhqc),  "fhqc")

        ; // End of the line is here !!
        ifstream ifs(filename.c_str());   
        store(parse_config_file(ifs, config_file_options), vm);
        notify(vm);
        if (LastN != -1) {       // Ho Ho Ho retain the option to run into the future ... must test this ....
            miutil::miTime TempStartTime=miutil::miTime::nowTime();
            TempStartTime.addDay(-LastN);
            StartDay=TempStartTime.day();
            StartMonth=TempStartTime.month();
            StartYear=TempStartTime.year();
         }
         miutil::miTime StartTime(StartYear,StartMonth,StartDay,StartHour,StartMinute,StartSecond);
         miutil::miTime EndTime(EndYear,EndMonth,EndDay,EndHour,EndMinute,EndSecond);
         //std::cout << config_file_options << std::endl;  // This prints all the conifg options !!!!!!!!!!!!!
         UT0=StartTime;
         UT1=EndTime;
         pid=ParamId;
         tid=TypeId;
         RunAtMinute=RunMinute;
         RunAtHour=RunHour;
         StepD=StepDay;
         StepH=StepHour;
         ControlInfoString=ControlString;
         ControlInfoVector=ControlVector;
         InterpolationLimit=InterpolationDistance;
         missing=MissingValue;
         MinimumValue=MinValue;
         std::cout << miutil::miTime::nowTime() << ": " << UT0 << " -> " << UT1 << "  " << filename << std::endl;


         //std::map<std::string, unsigned char> zflag; // initially wantd to implement this like this!!! But string description of flags not supported in
                                            // the library

         if (z_fqclevel != 0x3F) zflag[0]= z_fqclevel;
         if (z_fr != 0x3F) zflag[1]= z_fr;
         if (z_fcc != 0x3F) zflag[2]= z_fcc;
         if (z_fs != 0x3F) zflag[3]= z_fs;
         if (z_fnum != 0x3F) zflag[4]= z_fnum;
         if (z_fpos != 0x3F) zflag[5]= z_fpos;
         if (z_fmis != 0x3F) zflag[6]= z_fmis;
         if (z_ftime != 0x3F) zflag[7]= z_ftime;
         if (z_fw != 0x3F) zflag[8]= z_fw;
         if (z_fstat != 0x3F) zflag[9]= z_fstat;
         if (z_fcp != 0x3F) zflag[10]= z_fcp;
         if (z_fclim != 0x3F) zflag[11]= z_fclim;
         if (z_fd != 0x3F) zflag[12]= z_fd;
         if (z_fpre != 0x3F) zflag[13]= z_fpre;
         if (z_fcombi != 0x3F) zflag[14]= z_fcombi;
         if (z_fhqc != 0x3F) zflag[15]= z_fhqc;

         if (R_fqclevel != 0x3F) Rflag[0]= R_fqclevel;
         if (R_fr != 0x3F) Rflag[1]= R_fr;
         if (R_fcc != 0x3F) Rflag[2]= R_fcc;
         if (R_fs != 0x3F) Rflag[3]= R_fs;
         if (R_fnum != 0x3F) Rflag[4]= R_fnum;
         if (R_fpos != 0x3F) Rflag[5]= R_fpos;
         if (R_fmis != 0x3F) Rflag[6]= R_fmis;
         if (R_ftime != 0x3F) Rflag[7]= R_ftime;
         if (R_fw != 0x3F) Rflag[8]= R_fw;
         if (R_fstat != 0x3F) Rflag[9]= R_fstat;
         if (R_fcp != 0x3F) Rflag[10]= R_fcp;
         if (R_fclim != 0x3F) Rflag[11]= R_fclim;
         if (R_fd != 0x3F) Rflag[12]= R_fd;
         if (R_fpre != 0x3F) Rflag[13]= R_fpre;
         if (R_fcombi != 0x3F) Rflag[14]= R_fcombi;
         if (R_fhqc != 0x3F) Rflag[15]= R_fhqc;

         if (I_fqclevel != 0x3F) Iflag[0]= I_fqclevel;
         if (I_fr != 0x3F) Iflag[1]= I_fr;
         if (I_fcc != 0x3F) Iflag[2]= I_fcc;
         if (I_fs != 0x3F) Iflag[3]= I_fs;
         if (I_fnum != 0x3F) Iflag[4]= I_fnum;
         if (I_fpos != 0x3F) Iflag[5]= I_fpos;
         if (I_fmis != 0x3F) Iflag[6]= I_fmis;
         if (I_ftime != 0x3F) Iflag[7]= I_ftime;
         if (I_fw != 0x3F) Iflag[8]= I_fw;
         if (I_fstat != 0x3F) Iflag[9]= I_fstat;
         if (I_fcp != 0x3F) Iflag[10]= I_fcp;
         if (I_fclim != 0x3F) Iflag[11]= I_fclim;
         if (I_fd != 0x3F) Iflag[12]= I_fd;
         if (I_fpre != 0x3F) Iflag[13]= I_fpre;
         if (I_fcombi != 0x3F) Iflag[14]= I_fcombi;
         if (I_fhqc != 0x3F) Iflag[15]= I_fhqc;

         if (A_fqclevel != 0x3F) Aflag[0]= A_fqclevel;
         if (A_fr != 0x3F) Aflag[1]= A_fr;
         if (A_fcc != 0x3F) Aflag[2]= A_fcc;
         if (A_fs != 0x3F) Aflag[3]= A_fs;
         if (A_fnum != 0x3F) Aflag[4]= A_fnum;
         if (A_fpos != 0x3F) Aflag[5]= A_fpos;
         if (A_fmis != 0x3F) Aflag[6]= A_fmis;
         if (A_ftime != 0x3F) Aflag[7]= A_ftime;
         if (A_fw != 0x3F) Aflag[8]= A_fw;
         if (A_fstat != 0x3F) Aflag[9]= A_fstat;
         if (A_fcp != 0x3F) Aflag[10]= A_fcp;
         if (A_fclim != 0x3F) Aflag[11]= A_fclim;
         if (A_fd != 0x3F) Aflag[12]= A_fd;
         if (A_fpre != 0x3F) Aflag[13]= A_fpre;
         if (A_fcombi != 0x3F) Aflag[14]= A_fcombi;
         if (A_fhqc != 0x3F) Aflag[15]= A_fhqc;

         if (W_fqclevel != 0x3F) Wflag[0]= W_fqclevel;
         if (W_fr != 0x3F) Wflag[1]= W_fr;
         if (W_fcc != 0x3F) Wflag[2]= W_fcc;
         if (W_fs != 0x3F) Wflag[3]= W_fs;
         if (W_fnum != 0x3F) Wflag[4]= W_fnum;
         if (W_fpos != 0x3F) Wflag[5]= W_fpos;
         if (W_fmis != 0x3F) Wflag[6]= W_fmis;
         if (W_ftime != 0x3F) Wflag[7]= W_ftime;
         if (W_fw != 0x3F) Wflag[8]= W_fw;
         if (W_fstat != 0x3F) Wflag[9]= W_fstat;
         if (W_fcp != 0x3F) Wflag[10]= W_fcp;
         if (W_fclim != 0x3F) Wflag[11]= W_fclim;
         if (W_fd != 0x3F) Wflag[12]= W_fd;
         if (W_fpre != 0x3F) Wflag[13]= W_fpre;
         if (W_fcombi != 0x3F) Wflag[14]= W_fcombi;
         if (W_fhqc != 0x3F) Wflag[15]= W_fhqc;

         if (S_fqclevel != 0x3F) Sflag[0]= S_fqclevel;
         if (S_fr != 0x3F) Sflag[1]= S_fr;
         if (S_fcc != 0x3F) Sflag[2]= S_fcc;
         if (S_fs != 0x3F) Sflag[3]= S_fs;
         if (S_fnum != 0x3F) Sflag[4]= S_fnum;
         if (S_fpos != 0x3F) Sflag[5]= S_fpos;
         if (S_fmis != 0x3F) Sflag[6]= S_fmis;
         if (S_ftime != 0x3F) Sflag[7]= S_ftime;
         if (S_fw != 0x3F) Sflag[8]= S_fw;
         if (S_fstat != 0x3F) Sflag[9]= S_fstat;
         if (S_fcp != 0x3F) Sflag[10]= S_fcp;
         if (S_fclim != 0x3F) Sflag[11]= S_fclim;
         if (S_fd != 0x3F) Sflag[12]= S_fd;
         if (S_fpre != 0x3F) Sflag[13]= S_fpre;
         if (S_fcombi != 0x3F) Sflag[14]= S_fcombi;
         if (S_fhqc != 0x3F) Sflag[15]= S_fhqc;

         
         Vfqclevel = V_fqclevel;
         Vfr = V_fr;
         Vfcc = V_fcc;
         Vfs = V_fs;
         Vfnum = V_fnum;
         Vfpos = V_fpos;
         Vfmis = V_fmis;
         Vftime = V_ftime;
         Vfw = V_fw;
         Vfstat = V_fstat;
         Vfcp = V_fcp;
         Vfclim = V_fclim;
         Vfd = V_fd;
         Vfpre = V_fpre;
         Vfcombi = V_fcombi;
         Vfhqc = V_fhqc;

   }
   catch(exception& e) {
       std::cout << e.what() << std::endl;
       return 1;
   }
 return 0;
}

int
ReadProgramOptions::
clear() {

zflag.clear();
Rflag.clear();
Iflag.clear();
Aflag.clear();
Wflag.clear();
Sflag.clear();

miutil::miTime UT0(1900,1,1,0,0,0);
miutil::miTime UT1(1900,1,1,0,0,0);

StepD=0;
StepH=0;
AlgoCode=-1;
InterpCode=-1;
LastN=0;
std::string ControlInfoString;       ///Check these are cleared correctly
std::vector<int> ControlInfoVector;  ///TBD

RunAtMinute=0;
RunAtHour=2;
pid=0;
tid=0;

return 0;
}
//miutil::miTime UT0;
//miutil::miTime UT1;
//int StepD;
//int StepH;
//int AlgoCode;
//int LastN;
//std::string ControlInfoString;
//std::vector<int> ControlInfoVector;
//int RunAtMinute;
//int RunAtHour;
//int pid;
//int tid;
//bool newfile;



