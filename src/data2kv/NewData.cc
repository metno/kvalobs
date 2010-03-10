#include <sstream>
#include <puTools/miTime.h>
#include <milog/milog.h>
#include <fstream>
#include <unistd.h>
#include "NewData.h"
 
using namespace std;

 
NewData::NewData(App &app_)
  : app(app_)
{
    start_undetached();
}


  
NewData::~NewData()
{
}

void *
NewData::run_undetached(void*)
{
  list<string> dirList=app.copyDataToDirs();
  list<string>::const_iterator itDirList;
  FileList                     fileList;
  IFileList                    itfl;
  string                       content;
  string                       file;
  bool                         timeout;
  bool                         firstTime=true;
  LOGINFO("Starting thread NewData ....." << endl);

  while(!app.inShutdown()){

    if(!firstTime && !app.dataReady(2,timeout)){
      continue;
    }
    
    firstTime=false;
    
    if(!app.getFileList(fileList, app.datadir())){
      LOGDEBUG("No files !!???" << endl);
      continue;
    }
    
    for(itfl=fileList.begin();
	itfl!=fileList.end();
	itfl++){
   
      if(!itfl->isFile())
	continue;

      if(!app.readFile(itfl->name(), content)){
	LOGERROR("Cant read file <" <<  itfl->name() << ">" << endl);
	continue;
      }
      
      itfl->name();
      unlink(itfl->name().c_str());

      for(itDirList=dirList.begin();
	  itDirList!=dirList.end();
	  itDirList++){
	app.writeFile(string(app.datadir()+*itDirList), 
		  itfl->file(), content); 

      }
    }

    app.notifyServers();
  }

  LOGINFO("Thread NewData terminate ....." << endl);
  return 0;
}

