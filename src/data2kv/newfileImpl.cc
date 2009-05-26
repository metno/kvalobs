#include <milog/milog.h>
#include "newfileImpl.h"

void
NewFileImpl::newfile(const char* newfileindir)
{
  milog::LogContext ctxt("NewFileImpl");
  LOGDEBUG("New file in dir: " << newfileindir << std::endl);
  
  app.notifyData();
  if(app.datadir()!=newfileindir){
    LOGINFO("Expecting update in dir: " << app.datadir() << std::endl);
  }
}
