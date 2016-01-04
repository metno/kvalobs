#include "signalNewFileApp.h"

int main(int argn, char **argv) {
  SignalNewFileApp app(argn, argv);

  app.createFile("station", "data", "dummydecode");

}
