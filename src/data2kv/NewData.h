#ifndef __NewData_h__
#define __NewData_h__

#include <omnithread.h>
#include <string>
#include "App.h"

class NewData : public omni_thread {
  NewData(NewData&);
  NewData& operator=(const NewData&);
  NewData();

  App &app;

 public:
  NewData(App &app);

  ~NewData();

  void *run_undetached(void*);
};

#endif
