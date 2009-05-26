#ifndef __CollectData_h__
#define __CollectData_h__


#include <omnithread.h>
#include <string>
#include <map>
#include <list>
#include <fileutil/file.h>
#include "kvDataSrcList.h"
#include "App.h"



typedef std::list<dnmi::file::File>                   FileList;
typedef std::list<dnmi::file::File>::iterator        IFileList;
typedef std::list<dnmi::file::File>::const_iterator CIFileList;

class CollectData : public omni_thread
{
	CollectData(CollectData&);
	CollectData& operator=(const CollectData&);
	CollectData();


	App                             &app;
	bool                            lowpri;
	std::string                     datadir;
	KvDataSrc                       server;
	bool                            dataReady_;
	omni_mutex                      dataReadyMutex;
	omni_condition                  dataReadyCond;

	bool tryToSendSavedObservations();

	bool sendMessageToKvalobs( const std::string &msg, 
				                  const std::string &obsType,
				                  bool &kvServerIsUp,
				                  bool &tryToResend);
 
	void removeFiles();
    
public:
	CollectData( App &app, 
			      bool  lowpri,
			      const std::string &datadir,
			      const KvDataReceiver &dataReceiver ); 
	~CollectData();
    
	void notifyData();
	bool dataReady(unsigned long timeoutInSecs, bool &timeout);
    
	void *run_undetached(void*);
};

#endif
