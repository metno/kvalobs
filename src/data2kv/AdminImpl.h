#ifndef __AdminImpl_h__
#define __AdminImpl_h__

#include <kvskel/admin.hh>

class App;

class AdminImpl : public virtual POA_micutil::Admin,
    public PortableServer::RefCountServantBase {
  App &app;

 public:
  AdminImpl(App &app_);

  virtual ~AdminImpl();

  CORBA::Boolean ping();

  void statusmessage(CORBA::Short details, CORBA::String_out message);

  CORBA::Boolean shutdown();
};

#endif
