#include "AdminImpl.h"
#include "App.h"


AdminImpl::AdminImpl(App &app_)
  : app(app_)
{
}

AdminImpl::~AdminImpl()
{
}

CORBA::Boolean 
AdminImpl::ping()
{
  return true;
}

void 
AdminImpl::statusmessage(CORBA::Short details, 
			 CORBA::String_out message)
{
  message=CORBA::string_dup("--- Not implemented!! ---");
}
 
CORBA::Boolean 
AdminImpl::shutdown()
{
  app.doShutdown();
  return true;
}

