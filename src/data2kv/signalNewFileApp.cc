#include <iostream>
#include <string>
#include <unistd.h>
#include <sstream>
#include "signalNewFileApp.h"
#include "datetime.h"
#include <miconfparser/miconfparser.h>
#include <fstream>

#define LOG(a,b)  cerr << (b) 

using namespace std;
using miutil::conf::ConfSection;
using miutil::conf::ConfParser;
using miutil::conf::ValElementList;
using miutil::conf::CIValElementList;



namespace{
  CORBA::Boolean 
  globalTransientHandler(void* cooke,
			 CORBA::ULong retries,
			 const CORBA::TRANSIENT& ex);


  ConfSection  *confLoader();
}


SignalNewFileApp::SignalNewFileApp(int argn, char **argv)
  :CorbaHelper::CorbaApp(argn, argv)
{
  ValElementList   valelem; 
  CIValElementList it;
  ConfSection      *conf=confLoader();

  if(!conf){
    cerr << "Cant load the configuration file!";
    exit(1);
  }

  omniORB::installTransientExceptionHandler(0, globalTransientHandler);
  
  refNewfile=micutil::newfilesignal::_nil();


  valelem=conf->getValue("corba.aopath");
      
  it=valelem.begin();
  
  if(it!=valelem.end()){
    aopath_=it->valAsString();
  }else{
    cerr << "Missing 'corba.aopath' in the configuration file!";
    exit(1);
  }
  

  valelem=conf->getValue("corba.aonewfile");
      
  it=valelem.begin();
    
  if(it!=valelem.end()){
    newfileApp=it->valAsString();
  }else{
    cerr << "Missing 'corba.aonewfile' in the configuration file!";
    exit(1);
  }
  
  valelem=conf->getValue("datadir");
   
  it=valelem.begin();
    
  if(it!=valelem.end()){
    datadir=it->valAsString();
  }else{
    cerr << "Missing 'datadir' in the configuration file!";
    exit(1);
  }

  valelem=conf->getValue("corba.nameserver");
 
  it=valelem.begin();
  
  if(it!=valelem.end()){
    setNameservice(it->valAsString());
  }else{
    cerr << "Missing 'corba.nameserver' in the configuration file!";
    exit(1);
  }
  
  cerr << 
    "Signaling 'newfile' to the following paths in the" << endl <<
    "CORBA nameserver:" << endl << 
    "newfile:          '/" << aopath_ << "/" << newfileApp <<"'"<< endl <<
    "CORBA nameserver: " << getNameservice() << ">" << endl << 
    "datadir:          " << datadir << endl;
}





CORBA::Object_ptr
SignalNewFileApp::getRefInNS(const std::string &name_,
		  const std::string &path_)
{
 
  return getObjFromNS("/"+path_+"/"+name_);
}


/*
 *lookUpKvDataSource will either return the refData or look up kvinput'
 *in the CORBA nameservice.
 */ 
micutil::newfilesignal_ptr 
SignalNewFileApp::lookUpNewfilesignal(bool forceNS, 
				      bool &usedNS)
{
  CORBA::Object_var obj;
  micutil::newfilesignal_ptr ptr;

  usedNS=false;

  while(true){
    if(forceNS){
      usedNS=true;
      
      obj=getRefInNS(newfileApp, aopath_);
      
      if(CORBA::is_nil(obj))
	return micutil::newfilesignal::_nil();
      
      ptr=micutil::newfilesignal::_narrow(obj);
      
      if(CORBA::is_nil(ptr))
	return micutil::newfilesignal::_nil();

      refNewfile=ptr;
      return ptr;
    }

    if(CORBA::is_nil(refNewfile))
      forceNS=true;
    else
      return refNewfile;
  }
}

/**
 * sendDataToKvalobs save the data in the directory <data2kv> and send
 * a signal to autoobs2kv that new data is saved to the directory.
 */
  
bool
SignalNewFileApp::createFile(const std::string &station,
			     const std::string &data, 
			     const std::string &key)
{
  const int UNIQUE=100;
  micutil::newfilesignal_ptr ptrNewfile;
  bool     forceNS=false;
  bool     usedNS;
  char     buf[100];
  FILE     *fd;
  ostringstream ost;
  string   path;
  string   file;
  string   fileInProgres;
  int      unique;

  ost << datadir << "/";
  path=ost.str();
  
  dnmi::DateTime t(dnmi::DateTime::currentDateTime());

  for(unique=0; unique<UNIQUE; unique++){
    sprintf(buf, "-%04d%02d%02d%02d-%d.dat", 
	    t.date().year(), t.date().month(), t.date().day(),
	    t.time().hour(), unique);
    ost.str("");
    ost << path  << station <<buf;
    
    file=ost.str();

    fd=fopen(file.c_str(), "r");
    
    if(fd){
      fclose(fd);
    }else{
      break;
    }
  }

  if(unique>=UNIQUE){
    return false;
  }
   
  fileInProgres=file+".new";

  fd=fopen(fileInProgres.c_str(), "w");
  
  if(fd){
    fprintf(fd, "%s\n%s", key.c_str(), data.c_str());
    fclose(fd);
    if(rename(fileInProgres.c_str(), file.c_str())==-1){
      cerr << "Cant rename file: " << fileInProgres << " -> " << file << "\n";
    }else{
      cerr << "Data written to file <" << file << ">!\n";
    }
  }else{
    cerr << "Cant write data to file <" << fileInProgres << ">!\n";
    return false;
  }

  forceNS=false;
  
  for(int i=0; i<2; i++){
    ptrNewfile=lookUpNewfilesignal(forceNS, usedNS);
    
    if(!CORBA::is_nil(ptrNewfile)){
      try{
	ptrNewfile->newfile(path.c_str());
	cerr << "autoobs2kv notifyed!\n";
	return true;
      }
      //      CORBA_STD_EXCEPTION_TRACE //this hide the catch(...) handler
	catch(...){
	  if(usedNS){
	    cerr << "WARNING: can't send signal to aotoobs2kv!\n";
	    return false;
	  }else{
	    forceNS=true;
	  }
	}
    }
  }

  //Should never be reached
  return false;
}


namespace{
  CORBA::Boolean 
  globalTransientHandler(void* cooke,
			 CORBA::ULong retries,
			 const CORBA::TRANSIENT& ex)
  {
    //  TRACE(0, "GlobalTransientHandler: retries=" << retries << "\n");
    
    if(retries>10)
      return false;
    
    return true;
  }

  ConfSection* 
  confLoader()
  {
    ConfParser parser;
    string     confFile;
    ConfSection *conf;
    ifstream   fis;
    char       *p;
    string     path;

    p=getenv("AUTOOBS");
    
    if(!p){
      cerr << "\n\tThe environment variable AUTOOBS must be set!\n\n";
      exit(1);
    }

    path=p;
    if(path.empty()){
      cerr << "\n\tThe environment variable AUTOOBS is an empty string!\n\n";
      exit(1);
    }

    if(path[path.length()-1]!='/')
      path+='/';

    confFile=path+"etc/autoobs2kv.conf";

    fis.open(confFile.c_str());

    if(!fis){
      cerr << "Cant open the configuration file <" << confFile << ">!" << endl;
      exit(1);
    }
    
    cerr << "Reading configuration from file <" << confFile << ">!" << endl;
    conf=parser.parse(fis);
    
    if(!conf){
      cerr << "Error while reading configuration file: <" << confFile 
	       << ">!" << endl << parser.getError() << endl;
      exit(1);
    }

    cerr << "Configuration file loaded!\n";
    return conf;
      
  }
  
}
