#include <iostream>
#include <mtcout.h>
#include <WorkQue1.h>
#include <mgrApp.h>
#include <NewDataCommand.h>
#include <CommandQue.h>
#include <milog.h>

using namespace std;

template<class CommandType>
WorkQue1<CommandType>::WorkQue1(ManagerApp &app_, 
				dnmi::thread::CommandQue &inputQue_,
				const std::string &idName_)
  :app(app_), inputQue(inputQue_), idName(idName_)
{
}

template <class CommandType> void 
WorkQue1<CommandType>::operator()()
{
  dnmi::thread::CommandBase *cmd; 
  CommandType *newCmd;

  milog::LogContext context(idName);	
  LOGINFO(idName << ": starting!\n");

  while(!app.shutdown()){
    cmd=inputQue.get(1);

    //CERR(idName << ": Command received!\n");

    if(!cmd)
      continue;
    
    newCmd=dynamic_cast<CommandType*>(cmd);

    if(!newCmd){
      LOGERROR(idName << "::operator(): Unknown Command." << endl);
      delete cmd;
    }

    newCmd->execute();
    
    delete newCmd;
  }
  
  LOGINFO(idName <<": thread terminated!\n");

  
}

