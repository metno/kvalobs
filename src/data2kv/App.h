#ifndef __autoobs2kv_app_h__
#define __autoobs2kv_app_h__

#include <string>
#include <map>
#include <kvskel/datasource.hh>
#include <omnithread.h>
#include <corbahelper/corbaApp.h>
#include <fileutil/file.h>
#include "kvDataSrcList.h"


typedef std::list<dnmi::file::File>                   FileList;
typedef std::list<dnmi::file::File>::iterator        IFileList;
typedef std::list<dnmi::file::File>::const_iterator CIFileList;

class CollectData;

class App : public CorbaHelper::CorbaApp
{
 private:
  CKvalObs::CDataSource::Data_var refData;
  std::string datadir_;
  std::string logdir_;
  bool        test_;
  //  TKvDataSrcList refDataList;
  bool        hasData;
  std::string piddir_;
  bool        orbShutdown;
  std::string kvServer_;
  std::string aopath_;     //Which AutoObs servere shall we register with.
                           //Default is 'AutoObs'.
  std::string newfileName; //What name shall our 'newfile' interface be
                           //known as. Default is 'autoobs2kv'.

  TKvDataReceiverList highpriServers_;
  TKvDataReceiverList lowpriServers_;

  std::list<std::string>  copyToDirList;
  std::list<CollectData*> collectDataThreads;
  
  omni_mutex      mutex;
  omni_condition  hasDataCond;
  void initLogger(const std::string &ll, const std::string &tl);

 public:
  App(int argn, char **argv);
  ~App();

  CORBA::Object_ptr 
    getRefInNS( const std::string &name_, 
   		 		 const std::string &path_,
   		 		 const CorbaHelper::ServiceHost &nameserver );

  bool  putRefInNS(CORBA::Object_ptr objref, 
		   const std::string &name_);

  CKvalObs::CDataSource::Data_ptr 
    lookUpKvData(bool forceNS, 
		 bool &usedNS,
		 KvDataSrc &server);


   CKvalObs::CDataSource::Result 
     *sendDataToKvalobs(const std::string &message, 
			const std::string &obsType,
			KvDataSrc         &server);

   std::string kvServer()const { return kvServer_; }
   TKvDataReceiverList highpriServers()const { return highpriServers_;}
   TKvDataReceiverList lowpriServers()const { return lowpriServers_;}
   bool inShutdown();
   void doShutdown();
   bool dataReady(int timeoutInSecs, bool &timeout);
   void notifyData();
   void notifyServers();
   
   bool getFileList(FileList &fileList,
		    const std::string &path)const;

   bool getDirList(FileList &dirList, 
		   const std::string &path)const;

   bool removeDir(const std::string &path)const;

   bool readFile(const std::string &filepath,
		 std::string &content)const;

   bool readFile(const std::string &path,
		 const std::string &name,  
		 std::string &content)const;
   bool writeFile(const std::string &path,
		  const std::string &name,
		  const std::string &content)const;
   

   std::list<std::string> copyDataToDirs()const { return copyToDirList;} 
   std::string aopath()const{ return aopath_;}
   std::string newfileInterfaceName()const { return newfileName;}
   std::string datadir()const{ return datadir_;}
   std::string logdir()const { return logdir_;}
   bool        test()const{ return test_;}

   
   void        addCollectDataThread(CollectData *data);
   void        joinAllCollectDataThread();


   std::string piddir()const{ return piddir_;}
};


#endif
