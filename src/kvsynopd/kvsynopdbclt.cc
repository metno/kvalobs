/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvsynopdbclt.cc,v 1.4.6.5 2007/09/27 09:02:23 paule Exp $                                                       

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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <getopt.h>
#include <list>
#include <map>
#include <iostream>
#include <miconfparser/miconfparser.h>
#include <string>
#include <db/dbdrivermgr.h>
#include <fstream>
#include <kvalobs/kvDbGate.h>
#include "tblSynop.h"
#include <puTools/miTime>
#include <sstream>
#include <fstream>
using namespace std;

miutil::conf::ConfSection* 
readConf(const std::string &fname);

void 
use(int exitstatus);


//Initializing the DB system
bool 
getDbInfo(miutil::conf::ConfSection* conf,
	  const std::string &useThisConnect);
dnmi::db::DriverManager dbMgr;      //set by getDbConf
std::string             dbDriver;   //set by getDbConf
std::string             dbConnect;  //set by getDbConf
std::string             dbDriverId; //set by getDbConf


dnmi::db::Connection* 
getNewDbConnection();

void                  
releaseDbConnection(dnmi::db::Connection *con);

miutil::miTime
toStrToTime(const char *s);


typedef map<int, string>                   IntStringMap;
typedef map<int, string>::iterator        IIntStringMap;
typedef map<int, string>::const_iterator CIIntStringMap;

bool
readStationNames(const std::string &file, IntStringMap &map);


struct Options{
  bool           onlySynoptimes;
  bool           allTimes;
  miutil::miTime ftime; //from time
  miutil::miTime ttime; //To time. Empty, use only the ftime. 
  int            tstep; //Timestep in hour, default 3 hour
  std::list<string>  wmoList; //Empty --> all stations
  string         outputdir; //Empty --> stdout
  string         namefile; //Use name from the namefile if it exist
  string         dbconnect;

  Options():tstep(3){}
  Options(const Options &o): 
    ftime(o.ftime), ttime(o.ttime),tstep(o.tstep),
    wmoList(o.wmoList), outputdir(o.outputdir),
    namefile(o.namefile)
  {}
};

void
getOptions(int argn, char **argv, Options &opt);


void
getSynopAndOutput(Options &opt, 
		  const std::string &query,
		  IntStringMap      &nameMap,
		  dnmi::db::Connection *dbCon);




int
main(int argn, char **argv)
{
  char *pKvpath=getenv("KVALOBS");
  std::string kvpath;
  miutil::conf::ConfSection* conf;
  IntStringMap nameMap;
  Options      options;
  ostringstream ost;
  miutil::miTime obt;

  if(!pKvpath){
    cerr << "Missing KVALOBS environment variable!" << endl; 
    kvpath=".";
  }else{
    kvpath=pKvpath;
  }

  cerr << "Configurationfile: " << (kvpath+"/etc/kvsynopd.conf") << endl;

  conf=readConf(kvpath+"/etc/kvsynopd.conf");

  if(!conf){
    return 1;
  }
  
  getOptions(argn, argv, options);


  if(!getDbInfo(conf, options.dbconnect)){
    cerr << "Cant initialize the database system!" << endl;
    return 1;
  }
  
  dnmi::db::Connection *dbCon=getNewDbConnection();

  if(!dbCon){
    return 1;
  }

  if(!options.namefile.empty()){
    if(!readStationNames(options.namefile, nameMap)){
      cerr << "Inavlid format or missingfile: " << options.namefile << endl;
      use(1);
    }
  }


  if(options.wmoList.empty()){
    if(options.allTimes){
        ost << "ORDER BY obstime";
        //cerr << "Query: <" << ost.str() << ">!" << endl;
        getSynopAndOutput(options,  ost.str(), nameMap, dbCon);
    }else{
        obt=options.ftime;
  
        while(obt<=options.ttime){
            ost.str("");
            ost << "WHERE obstime=\'" << obt.isoTime() << "\'";
            //cerr << "Query: <" << ost.str() << ">!" << endl;
            getSynopAndOutput(options,  ost.str(), nameMap, dbCon);
            obt.addHour(options.tstep);
        }
    }
  }else{
    std::list<string>::iterator it=options.wmoList.begin();
    
    for(;it!=options.wmoList.end(); it++){
      obt=options.ftime;
      int wmono=atoi(it->c_str());

      if(wmono<=0)
	continue;
      
      if(wmono<1000){ //Assuming Norway
	wmono+=1000;
      }

      while(obt<=options.ttime){
	ost.str("");
	ost << "WHERE obstime=\'" << obt.isoTime() << "\' AND wmono=" 
	    << wmono;
	getSynopAndOutput(options,  ost.str(), nameMap, dbCon);
	obt.addHour(options.tstep);
      }
    }
  }
}

void
getSynopAndOutput(Options &opt, 
		  const std::string &query,
		  IntStringMap      &nameMap,
		  dnmi::db::Connection *dbCon
		  )
{
  char         buf[30];
  list<TblSynop> synopList;
  IIntStringMap tit;
  kvalobs::kvDbGate gate(dbCon);
  list<TblSynop>::iterator it;
  ostringstream ost;

  if(!gate.select(synopList, query)){
    cerr << "ERROR: " << gate.getErrorStr() << endl;
    return;
  }
   
  it=synopList.begin();

  for(; it!=synopList.end(); it++){
    tit=nameMap.find(it->wmono());
    
    if(tit==nameMap.end()){
      cerr << it->wmono() << "= <NO NAME>" << endl;
      
      //If the namefile is given. Write only synops for 
      //stations in the file.
      if(!opt.namefile.empty())
	continue;
    }
    
    if(opt.outputdir.empty()){
      cout << it->wmomsg() << endl;
      continue;
    }

    ost.str("");
    ost << opt.outputdir << "/";
    
    if(opt.onlySynoptimes && (it->obstime().hour()%3)!=0)
        continue;

    sprintf(buf, "-%04d%02d%02d%02d.syn", 
    	 it->obstime().year(),
    	 it->obstime().month(),
	    it->obstime().day(), 
	    it->obstime().hour());
    
    if(tit!=nameMap.end()){
      ost << tit->second << buf;
    }else{
      ost << it->wmono()<< buf;
    }

    ofstream fst;

    fst.open(ost.str().c_str());
    
    if(!fst.is_open()){
      cerr << "Cant create file: " << ost.str();
      continue;
    }

    fst <<  it->wmomsg() << endl;
    fst.close();
  }
}


bool 
getDbInfo(miutil::conf::ConfSection* conf,
	  const std::string &useThisConnect)
{
  miutil::conf::ValElementList valElem;
  string         val;

  valElem=conf->getValue("database.driver");

  if(valElem.empty()){
    cerr << "No <database.driver> in the configurationfile!" << endl;
    return false;
  }

  dbDriver=valElem[0].valAsString();
  
  cerr << "Loading driver for database engine <" << dbDriver << ">!" << endl;
  
  if(!dbMgr.loadDriver(dbDriver, dbDriverId)){
    cerr << "Can't load driver <" << dbDriver << endl 
	 << dbMgr.getErr() << endl 
	 << "Check if the driver is in the directory $KVALOBS/lib/db???"
	 << endl;

    return false;
  }

  if(useThisConnect.empty()){
    valElem=conf->getValue("database.dbconnect");
    
    if(valElem.empty()){
      cerr << "No <database.dbconnect> in the configurationfile!" << endl;
      return false;
    }
    
    dbConnect=valElem[0].valAsString();
  }else{
    dbConnect=useThisConnect;
  }
  
  cerr << "Connect string <" << dbConnect << ">!" << endl;

  return true;
}
  


miutil::conf::ConfSection* 
readConf(const std::string &fname)
{
  miutil::conf::ConfParser  parser;
  miutil::conf::ConfSection *conf;
  ifstream    fis;
  
  fis.open(fname.c_str());

  if(!fis){
    cerr<< "Cant open the configuration file <" << fname << ">!" << endl;
  }else{
    cerr <<"Reading configuration from file <" << fname << ">!" << endl;
    conf=parser.parse(fis);
      
    if(!conf){
      cerr << "Error while reading configuration file: <" << fname 
	   << ">!" << endl << parser.getError() << endl;
    }else{
      cerr << "Configuration file loaded!\n";
      return conf;
    }
  }
  
  return 0;
}

dnmi::db::Connection*
getNewDbConnection()
{
  dnmi::db::Connection *con;
  
  con=dbMgr.connect(dbDriverId, dbConnect);
  
  if(!con){
    cerr << "Can't create a database connection  (" 
	 << dbDriverId << ")" << endl << "Connect string: <" 
	 << dbConnect << ">!" << endl;
    return 0;
  }
  
  cerr << "New database connection (" << dbDriverId 
       << ") created!" << endl;

  return con;
}

void                  
releaseDbConnection(dnmi::db::Connection *con)
{
  dbMgr.releaseConnection(con);
}

bool
readStationNames(const std::string &file, IntStringMap &map)
{
  ifstream fis;
  string   buf;
  string::size_type i;
  string  name;
  string  no;

  fis.open(file.c_str());

  if(!fis){
    cerr << "Cant open file: " << file << endl;
    return false;
  }

  map.clear();

  while(fis){
    buf.erase();
    getline(fis, buf);
    
    if(buf.empty()){
      continue;
    }
    
    i=buf.find(",");
    
    if(i==string::npos){
      continue;
    }

    name=buf.substr(0, i);
    no=buf.substr(i+1);

    map[atoi(no.c_str())]=name;
  }
  
  return true;
}

void
getOptions(int argn, char **argv, Options &opt)
{
    struct option long_options[]={{"help", 0, 0, 0},
                                  {"only-synoptimes", 0, 0, 0},
				                  {0,0,0,0}};
                                  
    int c;
    int index;
    std::string sWmo;
  
    opt.allTimes=false;
    opt.onlySynoptimes=false;
  
    while(true){
        c=getopt_long(argn, argv, "o:f:t:n:i:s:c:", long_options, &index);
    
        if(c==-1)
            break;
    
        switch(c){
        case 'o':
            opt.outputdir=optarg;
            break;
        case 'f':
            if(string(optarg)=="all"){
                opt.ftime=miutil::miTime(1970, 1, 1, 0, 0, 0);
                opt.allTimes=true;
            }else{
                opt.ftime=toStrToTime(optarg);
            }
      
            if(opt.ftime.undef()){
	           cerr << "Invalid from time: " << optarg << endl;
	           use(1);
            }
            break;
        case 't':
            opt.ttime=toStrToTime(optarg);
            if(opt.ttime.undef()){
	           cerr << "Invalid to time: " << optarg << endl;
	           use(1);
            }
            break;
        case 'i':
            opt.namefile=optarg;
            break;
        case 's':
            opt.tstep=atoi(optarg);
            break;

        case 'c':
            opt.dbconnect=optarg;
            break;
      
        case 'n':{
            string numbers(optarg);
            string::size_type i=0;
            string::size_type end;
      
            i=numbers.find_first_not_of(" ,", 0);
      
            while(i!=string::npos){
	           end=numbers.find_first_of(", ", i);
	
                if(end!=string::npos){
                    opt.wmoList.push_back(numbers.substr(i, end-i));
	            }else{
	                opt.wmoList.push_back(numbers.substr(i));
	            }
                i=numbers.find_first_not_of(" ,", end);
            }
            }break;
        case 0:
            if(strcmp(long_options[index].name,"help")==0){
	           use(0);
            }
            if(strcmp(long_options[index].name,"only-synoptimes")==0){
                opt.onlySynoptimes=true;
            }
            break;
        case '?':
            cerr << "Unknown option : <" << (char)optopt << "> unknown!" << endl;
            use(1);
            break;
        case ':':
            cerr << optopt << " missing argument!" << endl;
            use(1);
            break;
        default:
            cerr << "?? option caharcter: <" << (char)optopt << "> unknown!" << endl;
            use(1);
        }
    }
 
  
    if(opt.ftime.undef()){
        cerr << "Missing from time (-f timestamp)!" << endl;
        use(1);
    }

    if(opt.ttime.undef()){
        if(opt.allTimes)
            opt.ttime=miutil::miTime::nowTime();
        else
            opt.ttime=opt.ftime;
    }

    if(opt.tstep<=0){
        opt.tstep=1;
    }

    if(!opt.outputdir.empty()){
        struct stat sbuf;

        if(stat(opt.outputdir.c_str(), &sbuf)<0){
            perror("ERROR: option -o outputdir:");
            use(1);
        }

        if(!S_ISDIR(sbuf.st_mode)){
            cerr << "ERROR: " << opt.outputdir << " is not a directory!" << endl;
            use(1);
        }
    }
}

void 
use(int exitstatus)
{
  cerr << "\n\tuse" << endl
       <<"\t   kvsynopdbclt -f timestamp [-t timestamp] [-s step] " << endl
       << "\t\t[-i stationnames] [-o outdir] [-n wmonolist] [-c dbconnect]" << endl 
       << "\t\t[--only-synoptimes]" << endl << endl
       << "\t-f timestamp  From time. Use 'all' to ignore fromtime!" << endl
       << "\t-t timestamp  To time" << endl
       << "\t-s step       Increment the fromtime with this step until totime"
       << "\n\t          is reached." << endl
       << "\t-o outdir     Write the output to the outdir" << endl
       << "\t-i stationnames Read the file 'stationsnames' to associate" << endl
       << "\t\twmonumbers with names." << endl
       << "\t-n wmonolist   a list of wmonumbers to list."<< endl
       << "\t-c dbconnect  Use this connect string to connect to the cache database!" << endl << endl
       << "\t timestamp format: YYYY-MM-DD hh" << endl
       << "\t wmonolist format: n0 n1 n2 .... nN" << endl << endl;

  exit(exitstatus);
}

miutil::miTime
toStrToTime(const char *s)
{
  int YY, MM, DD, hh;

  if(sscanf(s, "%d-%d-%d %d", &YY, &MM, &DD, &hh)!=4)
    return miutil::miTime();

  return miutil::miTime(YY, MM, DD, hh);
}
