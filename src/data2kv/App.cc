#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h> 
#include <string.h>
#include <fstream>
#include <sstream>
#include <milog/milog.h>
#include <miconfparser/miconfparser.h>
#include <miutil/replace.h>
#include <miutil/trimstr.h>
#include <kvalobs/kvPath.h>
#include <fileutil/dir.h>
#include <puTools/miTime>
#include "App.h"
#include "CollectData.h"

using namespace std;
using namespace CKvalObs::CDataSource;
using namespace milog;
using miutil::conf::ConfSection;
using miutil::conf::ConfParser;
using miutil::conf::ValElementList;
using miutil::conf::CIValElementList;

namespace{
  FLogStream   *fs;
  StdErrStream *trace;

  ConfSection  *confLoader();
  LogLevel getLogLevel(const char *str);
  bool     setLoglevel(const string &ll, const string &tl);
  volatile sig_atomic_t sigTerm=0;
  void sig_term(int);
  void setSigHandlers();
  void usage();
}

App::
App(int argn, 
	 char **argv)
  : CorbaHelper::CorbaApp(argn, argv),
    test_(false), hasData(false),orbShutdown(false),
    aopath_("AutoObs"), newfileName("data2kv"), hasDataCond(&mutex)
{
	string::size_type i;
	string::size_type i2;
	string            arg;
	string            key;
	string            val;
	struct  stat      sbuf;
	char              *buf;
	string            loglevel;
	string            tracelevel;
	ConfSection       *myConf=confLoader();
	ValElementList    valelem;

	for(int k=1; k<argn; k++){
		arg=argv[k];
		i=arg.find_first_of("=");

		if(i!=string::npos){
			key=arg.substr(0, i);
			val=arg.substr(i+1);
		}else{
			key=arg;
			val.erase();
		}
	
		//LOGDEBUG("Key=" << key << "  val=" << val << endl);

		if(key=="--tracelevel"){
			tracelevel=val;
		}else if(key=="--loglevel"){
			loglevel=val;
		}else if( key=="--logdir"){
			logdir_=val;
		}else if( key=="--piddir"){
			piddir_=val;
		}else if(key=="--help"){
			usage();
		}else if(key=="--test"){
			test_=true;
		}else{
			LOGWARN("Unknown option: " << key << 
					  (!val.empty()?string("="+val):"") 
					  << endl); 
		}
	}

	if(logdir_.empty()){
		if(myConf){
			valelem=myConf->getValue("logdir");
			if(valelem.size()==1)
				logdir_=valelem[0].valAsString();
		}
    
      if(logdir_.empty())
      	usage();
	}

	if(logdir_[0]!='/'){
		LOGFATAL("The 'logdir' directory must " <<
				   "be an absolute path. ex /my/logdir.");
		usage();
	}
    
	if(stat(logdir_.c_str(), &sbuf)<0){
		LOGFATAL("Cant stat the file/path <" << logdir_ << ">\n" <<
				   "-- " << strerror(errno) << endl);
		exit(1);
	}
    
	if(!S_ISDIR(sbuf.st_mode)){
		LOGFATAL("The logdir=<" << logdir_ << "> is not a directory!");
		exit(1);
	}
    
	if(access(logdir_.c_str(), R_OK | W_OK)<0){
		LOGFATAL("autoobs2kv must have read/write access to the 'logdir' directory\n");
		exit(1);
	}
    
	if(logdir_.rbegin()!=logdir_.rend() && *logdir_.rbegin()!='/')
		logdir_.append("/");

	if(datadir_.empty()){
		if(myConf){
			valelem=myConf->getValue("datadir");
	
			if(valelem.size()==1)
				datadir_=valelem[0].valAsString();
		}
		
		if(datadir_.empty())
			usage();
	}

	if(datadir_[0]!='/'){
		LOGFATAL("The 'datadir' directory must " <<
				   "be an absolute path. ex /my/datadir.");
		usage();
	}
    
	if(stat(datadir_.c_str(), &sbuf)<0){
		LOGFATAL("Cant stat the file/path <" << datadir_ << ">\n" <<
				   "-- " << strerror(errno) << endl);
		exit(1);
	}
    
	if(!S_ISDIR(sbuf.st_mode)){
		LOGFATAL("The datadir=<" << datadir_ << "> is not a directory!");
		exit(1);
	}
    
	if(access(datadir_.c_str(), R_OK | W_OK)<0){
		LOGFATAL("autoobs2kv must have read/write access to the 'datadir' directory\n");
		exit(1);
	}
    
	if(datadir_.rbegin()!=datadir_.rend() && *datadir_.rbegin()!='/')
		datadir_.append("/");


	if(piddir_.empty()){
		if(myConf){
			valelem=myConf->getValue("piddir");
			if(valelem.size()==1)
				piddir_=valelem[0].valAsString();
		}
    
      if(piddir_.empty()) 
      	piddir_ = kvPath("localstatedir") + "/run";
	}

	if(piddir_[0]!='/'){
		LOGFATAL("The 'piddir' directory must " <<
		         "be an absolute path. ex /my/piddir.");
		usage();
	}
    
	if(stat(piddir_.c_str(), &sbuf)<0){
		LOGFATAL("Cant stat the file/path <" << piddir_ << ">\n" <<
		         "-- " << strerror(errno) << endl);
		exit(1);
	}
    
	if(!S_ISDIR(sbuf.st_mode)){
		LOGFATAL("The piddir=<" << piddir_ << "> is not a directory!");
		exit(1);
	}
    
	if(access(piddir_.c_str(), R_OK | W_OK)<0){
		LOGFATAL("autoobs2kv must have read/write access to the 'piddir' directory\n");
		exit(1);
	}
    
	if(piddir_.rbegin()!=piddir_.rend() && *piddir_.rbegin()!='/')
		piddir_.append("/");

	if(myConf){
		valelem=myConf->getValue("loglevel");
      
      if(valelem.size()==1)
      	loglevel=valelem[0].valAsString();
      else
      	loglevel="INFO";

      valelem=myConf->getValue("tracelevel");
      
      if(valelem.size()==1)
      	tracelevel=valelem[0].valAsString();
      else
      	tracelevel="DEBUG";
	}
      


	initLogger(loglevel, tracelevel);

	if(myConf){
		valelem=myConf->getValue("corba.nameserver");
      
      CIValElementList it=valelem.begin();
    
      if(it!=valelem.end()){
      	setNameservice(it->valAsString());
      }
	}

	
	
	/**
	 * COMMENT:
	 * The kvalobsservers we shall send observation to is given with the
	 * variables kvservers and kvservers_nopri  in the conf file 
	 * $KVALOBS/etc/autoobs2kv.conf. The nopri servers has no garanti to 
	 * get the data;
	 * The environment variable KVSERVERS is a space separeted list of 
	 * paths in CORBA nameserver where we can find a kvDataInputd. 
	 */
    
	if(myConf){
		valelem=myConf->getValue("corba.kvservers");

      CIValElementList it=valelem.begin();
      
      for(;it!=valelem.end(); it++){
      	KvDataReceiver dr;
      	
      	if( dr.decode( it->valAsString(), getNameservice() ) ) 
      		highpriServers_.push_back( dr );
      }
	}

	if(myConf){
		valelem=myConf->getValue("corba.kvservers_nopri");

      CIValElementList it=valelem.begin();
    
      for(;it!=valelem.end(); it++){
      	KvDataReceiver dr;
            	
      	if( dr.decode( it->valAsString(), getNameservice() ) ) 
      		lowpriServers_.push_back( dr );
      }
	}

	if(myConf){
		valelem=myConf->getValue("corba.path");

      CIValElementList it=valelem.begin();
    
      if(it!=valelem.end()){
      	kvServer_=it->valAsString();
      }else{
      	LOGWARN("No \"path\" is given in the corba section of autoobs2kv.conf"
      			  << endl << " using default path <kvalobs>." << endl
      			  << "Notis this is the path that is the monitored production"
      			  << endl << "server!");
      	kvServer_="kvalobs";
      }
	}

	if(myConf){
		valelem=myConf->getValue("corba.aopath");
      
      CIValElementList it=valelem.begin();
    
      if(it!=valelem.end()){
      	aopath_=it->valAsString();
      }
	}


	if(myConf){
		valelem=myConf->getValue("corba.aonewfile");
      
      CIValElementList it=valelem.begin();
      
      if(it!=valelem.end()){
      	newfileName=it->valAsString();
      }
	}

	FileList mydir;

	if(!getDirList(mydir, datadir_)){
      LOGERROR("Cant get the directory list for <" << datadir_ << "!" 
      			<<endl);
      exit(1);
	}

	//Remove all files directorys in the datadir that is not
	//in highpriServers and backup
	for( IFileList it=mydir.begin();
		  it!=mydir.end();
		  it++)
	{
      if(it->file()=="backup")
      	continue;

      TKvDataReceiverList::const_iterator sit=highpriServers_.begin();
      
      for(;sit!=highpriServers_.end(); sit++){
      	if(sit->dirName==it->file())
      		break;
      }
    
      if(sit==highpriServers_.end()){
      	LOGINFO("Removing directory: " << it->name() << endl);
      	removeDir(it->name());
      }
	}
      

	//Create the needed directorys
	TKvDataReceiverList::const_iterator sit=highpriServers_.begin();
	for(;sit!=highpriServers_.end(); sit++)
	{
		string mypath(datadir_+sit->dirName);

		if(mkdir(mypath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)==-1){
			if(errno!=EEXIST){
				LOGFATAL("Cant create the directory: " << mypath << endl);
				exit(1);
			}else{
				LOGINFO("Allready exist: " << mypath << endl);
			}
		}else{
			LOGINFO("Created directory: " << mypath << endl);
		}
     
		copyToDirList.push_back( sit->dirName );
	}
	
	sit=lowpriServers_.begin();
	for(;sit!=lowpriServers_.end(); sit++)
	{
		string mypath(datadir_+sit->dirName);

		if(mkdir(mypath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)==-1){
			if(errno!=EEXIST){
				LOGFATAL("Cant create the directory: " << mypath << endl);
				exit(1);
			}else{
				LOGINFO("Allready exist: " << mypath << endl);
			}
		}else{
			LOGINFO("Created directory: " << mypath << endl);
      }
      
		copyToDirList.push_back( sit->dirName );
	}

	string lowpri;
	string highpri;

	for( TKvDataReceiverList::const_iterator it=highpriServers_.begin();
		  it!=highpriServers_.end();
		  it++ )
		highpri+=" " + it->confName;


	for( TKvDataReceiverList::const_iterator it=lowpriServers_.begin();
		  it!=lowpriServers_.end();
	     it++)
		lowpri+=" " + it->confName;
    
        
	LOGINFO("Pushing data to kvDataInputd in the following paths in the\n" <<
			  "CORBA nameserver:\n"    
			  "  highpri servers: " << highpri << endl <<
	        "   lowpri servers: " << lowpri << endl << 
	        "datadir:              <" << datadir_    << ">" << endl <<
	        "logdir:               <" << logdir_     << ">" << endl <<
	        "piddir:               <" << piddir_     << ">" << endl <<
	        "kvserver:             <" << kvServer_   << ">" << endl <<
	        "aopath:               <" << aopath_     << ">" << endl <<
	        "newfileInterfaceName: <" << newfileName << ">" <<endl  <<
	        "CORBA nameserver:     <" << getNameservice() << ">" << endl);
       
    setSigHandlers();
}

App::
~App()
{
}

bool
App::
readFile( const std::string &file, 
	       std::string &content)const
{
	ifstream      fs(file.c_str());
	ostringstream ost;
	char          ch;

	if(!fs){
		LOGERROR("Cant open file <" << file << ">!");
		return false;
	}

	while(fs.get(ch)){
		ost.put(ch);
	}

	if(!fs.eof()){
		LOGERROR("Error while reading file <" << file << ">!");
		fs.close();
		return false;
	}

	fs.close();
	content=ost.str();

	return true;
}


bool
App::
readFile( const std::string &path,
			 const std::string &name, 
			 std::string &content )const
{
	string file(path+"/"+name);

	return readFile(file, content);
}



bool 
App::
writeFile( const std::string &path,
			  const std::string &name, 
			  const std::string &content)const
{
	const int UNIQUE=100;
	char     buf[128];
	FILE     *fd;
	ostringstream ost;
	string   file;
	string   fileInProgres;
	int      unique;

	ost << path <<"/";
  
	miutil::miTime t(miutil::miTime::nowTime() );

	for(unique=0; unique<UNIQUE; unique++){
		sprintf( buf, "-%04d%02d%02d%02d-%d.dat", 
				   t.year(), t.month(), t.day(),
				   t.hour(), unique);
		ost.str("");
		ost << path  << "/" << name << buf;
    
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
		fprintf(fd, "%s", content.c_str());
		fclose(fd);

		if(rename(fileInProgres.c_str(), file.c_str())==-1){
			LOGERROR( "Cant rename file: " << fileInProgres << " -> " 
					    << file << "\n");
		}else{
			LOGDEBUG("Data written to file <" << file << ">!\n");
		}
	}else{
		LOGERROR("Cant write data to file <" << fileInProgres << ">!\n");
		return false;
	}

	return true;
}



bool 
App::
getFileList( FileList &fileList, 
				 const std::string &path_)const
{
	dnmi::file::Dir   dir("*.dat");
	string            file;
	string            path(path_);
	string            filepath;
	struct stat       sbuf;
	dnmi::file::File  f;

	
	fileList.clear();
  
	if(path.empty()){
		LOGERROR("getFileList: path is empty!!!!");
		return false; 
	}

	if(*path.rbegin()!='/')
		path.append("/");
	
	fileList.clear();

	try{
		if(!dir.open(path)){
			LOGERROR("Cant read directory <" << path << ">!");
			return false;
		}

		//LOGDEBUG("Scan the directory: " << path << endl <<
		//	   "Pattern: <" << (pattern.empty()?"*":pattern) 
		//	   << ">");
  
		while( dir.hasNext() ){
			file = dir.next();
		
			if(file==".." || file==".")
				continue;
    
			filepath=path;
			filepath+=file;
    
			if(stat(filepath.c_str(), &sbuf)<0){
				LOGERROR( "Cant get modification time for the file <" << filepath
				    		 << ">!");
				continue;
			}
    
			f=dnmi::file::File(filepath, sbuf);
    
			if(!f.isFile())
				continue;

			fileList.push_back(f);
		}
	}
	catch( const std::exception &ex ) {
		LOGERROR( "EXCEPTION: While scanning the directory <" << path << ">. Reason: " << ex.what() );
		return false;
	}
  
	return !fileList.empty();
}

bool 
App::
getDirList( FileList &dirList, 
		      const std::string &path_)const
{
	dnmi::file::Dir   dir;
	string            file;
	string            path(path_);
	string            filepath;
	struct stat       sbuf;
	dnmi::file::File  f;

	dirList.clear();
  
	if(path.empty()){
		LOGERROR("getFileList: path is empty!!!!");
      return false; 
	}

	if(*path.rbegin()!='/')
		path.append("/");

	dirList.clear();

	if(!dir.open(path)){
		LOGERROR("Cant read directory <" << path << ">!");
		return false;
	}

	//LOGDEBUG("Scan the directory: " << path << endl <<
	//	   "Pattern: <" << (pattern.empty()?"*":pattern) 
	//	   << ">");
  
	try {
		while( dir.hasNext() ){
			file = dir.next();

			if(file==".." || file==".")
				continue;
    
			filepath=path;
			filepath+=file;
    
			if(stat(filepath.c_str(), &sbuf)<0){
				LOGERROR( "Cant get modification time for the file <" << filepath
					    << ">!");
				continue;
			}
    
			f=dnmi::file::File(filepath, sbuf);
    
			if(!f.isDir())
				continue;

			dirList.push_back(f);
		}
	}
	catch( const std::exception &ex ) {
		LOGERROR( "EXCEPTION: While scanning the directory <" << path << "> for directorys. Reason: " << ex.what() );
		return false;
	}
  
	return true;
}

CORBA::Object_ptr 
App::
getRefInNS( const std::string &name_, 
			   const std::string &path_, 
			   const CorbaHelper::ServiceHost &nameserver)
{
	std::string name(path_);

	if(name.empty())
		return CORBA::Object::_nil();

	if(*name.rbegin()!='/')
		name.append("/");

	name.append(name_);
    
	return getObjFromNS(name, nameserver );
}

bool 
App::
putRefInNS( CORBA::Object_ptr objref, 
				const std::string &name)
{
	return putObjInNS(objref, name);
}


Result* 
App::
sendDataToKvalobs( const std::string &message, 
						 const std::string &obsType,
						 KvDataSrc   &server)
{
	Data_ptr ptrData;
	bool     forceNS=false;
	bool     usedNS;
	Result   *resToReturn=0;
	Result   *res;
	ostringstream ost;
   
	if(message.empty() || obsType.empty()){
		LOGERROR("INTERNAL: Invalid input message or obstype not given!");
		return 0;
	}

	forceNS=false;
    
	for(int i=0; i<2; i++){
		ptrData=lookUpKvData(forceNS, usedNS, server);
      
		if(CORBA::is_nil(ptrData)){
			server.ref(Data::_nil());
			return 0;
		}
	    
		try{
			res=ptrData->newData(message.c_str(), obsType.c_str());
			return res;
      }
      catch(...){
      	if(usedNS){
      		server.ref(Data::_nil());
      		return 0;
      	}
      	forceNS=true;
      }
	}
    
	//Shall never be reached
	return 0;
}



CKvalObs::CDataSource::Data_ptr 
App::
lookUpKvData( bool forceNS, 
				  bool &usedNS,
				  KvDataSrc &server)

{
	CORBA::Object_var obj;
	Data_ptr ptr;

	usedNS=false;

	while(true){
		if(forceNS){
			usedNS=true;
      
			obj=getRefInNS("kvinput", server.dataReceiver().name, server.dataReceiver().ns );
      
			if(CORBA::is_nil(obj))
				return Data::_nil();
      
			ptr=Data::_narrow(obj);
      
			if(CORBA::is_nil(ptr))
				return Data::_nil();
      
			server.ref(ptr);
      
			return ptr;
		}

		if(CORBA::is_nil(server.ref()))
			forceNS=true;
		else
			return server.ref();
	}
}

void
App::
initLogger(const std::string &ll, const std::string &tl)
{
	char         *name;
	string       filename;
	LogLevel     traceLevel=getLogLevel(tl.c_str());
	LogLevel     logLevel=getLogLevel(ll.c_str());
	string       logname("autoobs2kv");
    

	if(traceLevel==milog::NOTSET || logLevel==milog::NOTSET){
		if(traceLevel==milog::NOTSET){
			LOGFATAL("Unknown tracelevel: " << tl);
      }else{
      	LOGFATAL("Unknown loglevel: " << ll);
      }

      usage();
	}
    
	filename=logdir()+"autoobs2kv.log";
     
	try{
		fs=new FLogStream(4);
      
		if(!fs->open(filename)){
			std::cerr << "FATAL: Can't initialize the Logging system.\n";
			std::cerr << "------ Cant open the Logfile <" << filename << ">\n";
			delete fs;
			exit(1);
		}
	
		trace=new StdErrStream();
    
		if(!LogManager::createLogger(logname, trace)){
			std::cerr << "FATAL: Can't initialize the Logging system.\n";
			std::cerr << "------ Cant create logger\n";
			exit(1);
		}
	
		if(!LogManager::addStream(logname, fs)){
			std::cerr << "FATAL: Can't initialize the Logging system.\n";
			std::cerr << "------ Cant add filelogging to the Logging system\n";
			exit(1);
		}
	
		trace->loglevel(traceLevel);
		fs->loglevel(logLevel);
	
		LogManager::setDefaultLogger(logname);
	}
	catch(...){
		std::cerr << "FATAL: Can't initialize the Logging system.\n";
		std::cerr << "------ OUT OF MEMMORY!!!\n";
		exit(1);
	}

	LOGINFO("Logging to file <" << filename << ">!\n");
}


bool 
App::
dataReady(int timeoutInSecs, bool &timeout)
{
	unsigned long s, ns;

	timeout=false;
	omni_mutex_lock lock(mutex);

	if(hasData){
		hasData=false;
		return true;
	}else{
		omni_thread::get_time(&s, &ns, timeoutInSecs);

		if(!hasDataCond.timedwait(s, ns)){
			timeout=true;
			return false;
		}else{
			bool ret=hasData;
			hasData=false;
			return ret;
		}
	} 
}
 
void
App::
notifyData()
{
	omni_mutex_lock lock(mutex);
  
   hasData=true;
   hasDataCond.broadcast();
}

void
App::
notifyServers()
{
	std::list<CollectData*>::iterator it;
  
	for( it=collectDataThreads.begin();
	     it!=collectDataThreads.end();
	     it++)
	{
		(*it)->notifyData();
	}
}


void 
App::
doShutdown()
{
	omni_mutex_lock lock(mutex);
	sigTerm=1;
}


bool 
App::
inShutdown()
{
	omni_mutex_lock lock(mutex);
	bool ret=sigTerm>0;

	if(ret && !orbShutdown){
		CorbaHelper::CorbaApp::getOrb()->shutdown(false);
		orbShutdown=true;
	}

	return ret;
}


void        
App::
addCollectDataThread(CollectData *data)
{
	collectDataThreads.push_back(data);
}

void        
App::
joinAllCollectDataThread()
{
	std::list<CollectData*>::iterator it;

	for( it=collectDataThreads.begin();
	     it!=collectDataThreads.end();
        it++)
	{
		(*it)->join(0);
	}

	collectDataThreads.clear();
}

/**
 * \brief removeDir removes all files in a directory and the directory.
 */
bool
App::
removeDir(const std::string &path_)const
{
	dnmi::file::Dir   dir;
	string            path(path_);
	string            filepath;
	string            file;
	struct stat       sbuf;
	dnmi::file::File  f;

	if(path.empty()){
		LOGERROR("removedir: path is empty!!!!");
      return false; 
	}

	if(*path.rbegin()!='/')
		path.append("/");

	if(!dir.open(path)){
		LOGERROR("Cant read directory <" << path << ">!");
		return false;
	}

	try {
		while( dir.hasNext() ) {
			file = dir.next();
		
			if(file==".." || file==".")
				continue;
    
			filepath=path;
			filepath+=file;
    
			if(stat(filepath.c_str(), &sbuf)<0){
				LOGERROR("Cant stat the file <" << filepath << ">!");
				continue;
			}
    
			f=dnmi::file::File(filepath, sbuf);
    
			if(!f.isFile())
				continue;

			unlink(filepath.c_str());
		}
	}
	catch( const std::exception &ex ) {
		LOGERROR("EXCEPTION: While removing file in directory <" << path << ">. Reason: " << ex.what() );
	}
  
	if(rmdir(path.c_str())==-1){
		LOGERROR("Cant remove the directory <" << path << ">!" << endl);
		return false;
	}

	LOGINFO("Removed the directory <" << path << endl);
	return true;
}


namespace{
  void 
  sig_term(int)
  {
    sigTerm=1;
  }
  
  
  void
  setSigHandlers()
  {
    sigset_t         oldmask;
    struct sigaction act, oldact;
    
    
    act.sa_handler=sig_term;
    sigemptyset(&act.sa_mask);
    act.sa_flags=0;
    
    if(sigaction(SIGTERM, &act, &oldact)<0){
      LOGFATAL("Can't install signal handler for SIGTERM\n");
      exit(1);
    }
    
    act.sa_handler=sig_term;
    sigemptyset(&act.sa_mask);
    act.sa_flags=0;
    
    if(sigaction(SIGINT, &act, &oldact)<0){
      LOGFATAL("Can't install signal handler for SIGTERM\n");
      exit(1);
    }
  }
  
  void 
  usage()
  {
    LOGFATAL(" \n\nUSE \n\n" <<
	     "   autoobs2kv   --logdir=path [OPTIONS]\n\n" 
	     "       --logdir=path ex. /var/log \n"
	     "       --piddir=path ex. /var/run \n"
	     "     OPTIONS: \n" 
	     "       --test  [Use this to save additonal files to 'tmpdir'.\n"
	     "                This files can be used for regression testing]\n"
	     "       --traclevel[=level]\n"
	     "       --loglevel[=level]\n"
	     "       --help  print this help and exit.\n\n"
	     "       \nlevel may be one of: \n"
	     "        FATAL or 0\n"
	     "        ERROR or 1\n"
	     "        WARN  or 2\n"
	     "        INFO  or 3\n"
	     "        DEBUG or 4\n\n"
	     "     - Default level is DEBUG, if no level is specified.\n"
	     "     - If no logfile is specified, the log will be written to \n"
	     "       the consol, only.\n"
	     "     - The logfile will be written to the directory\n"
	     "        given with --logfile=absolute_path and the name will \n"
	     "        be norcom2kv.log.\n"
	     "\n\n");
    exit(1);
  }

  bool
  setLoglevel(const std::string &ll, const std::string &tl)
  {
    LogLevel loglevel;
    
    if(!ll.empty()){
      loglevel=getLogLevel(ll.c_str());
      
      if(loglevel==milog::NOTSET){
	return false;
      }
      
      fs->loglevel(loglevel);
    }
  
    if(!tl.empty()){
      loglevel=getLogLevel(tl.c_str());
      
      if(loglevel==milog::NOTSET){
	return false;
      }
      
      trace->loglevel(loglevel);
    }

    return true;
}


  LogLevel getLogLevel(const char *str)
  {
    if(strcmp("FATAL", str)==0){
      return milog::FATAL;
    }else if(strcmp("ERROR", str)==0){
      return milog::ERROR;
    }else if(strcmp("WARN", str)==0){
      return milog::WARN;
    }else if(strcmp("DEBUG", str)==0){
      return milog::DEBUG;
    }else if(strcmp("INFO", str)==0){
      return milog::INFO;
    }else if(strcmp("0", str)==0){
      return milog::FATAL;
    }else if(strcmp("1", str)==0){
      return milog::ERROR;
    }else if(strcmp("2", str)==0){
      return milog::WARN;
    }else if(strcmp("3", str)==0){
      return milog::INFO;
    }else if(strcmp("4", str)==0){
      return milog::DEBUG;
    }else{
      return milog::NOTSET;
    }
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
