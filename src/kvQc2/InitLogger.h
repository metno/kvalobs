#ifndef __initlogger_h__
#define __initlogger_h__
///Generic kvalobs library code for the initialisation of the logging system.

void
InitLogger(int argn, char **argv, const std::string &logname,
	   std::string &htmlpath);

#endif
