#ifndef __newfileImpl_h__
#define __newfileImpl_h__

#include <kvskel/newfile.hh>
#include "App.h"

class NewFileImpl : public POA_micutil::newfilesignal,
    public PortableServer::RefCountServantBase {
  App &app;

 public:
  // standard constructor
  NewFileImpl(App &app_)
      : app(app_) {
  }

  virtual ~NewFileImpl() {
  }

  // methods corresponding to defined IDL attributes and operations
  void newfile(const char* newfileindir);
};

#endif
