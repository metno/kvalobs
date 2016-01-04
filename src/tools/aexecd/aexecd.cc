/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: LogManager.cc,v 1.6.6.3 2007/09/27 09:02:32 paule Exp $

 Copyright (C) 2007 met.no

 Contact information:
 Norwegian Meteorological Institute
 Box 43 Blindern
 0313 OSLO
 NORWAY
 email: kvalobs-dev@met.no

 This file is part of KVALOBS

 KVALOBS is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License as
 published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 KVALOBS is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 General Public License for more details.

 You should have received a copy of the GNU General Public License along
 with KVALOBS; if not, write to the Free Software Foundation Inc.,
 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <iostream>
#include <signal.h>
#include <string.h>
#include <string>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include "fileutil/pidfileutil.h"
#include "milog/milog.h"
#include "InitLogger.h"
#include "miutil/simplesocket.h"
#include "miutil/signalhelpers.h"
#include "kvalobs/kvPath.h"
#include "miutil/splitstr.h"
#include "ExecHelper.h"
#include "aexecd.h"

using namespace std;

static volatile sig_atomic_t sigChild = 0;
static volatile sig_atomic_t sigTerm = 0;

int main(int argn, char *argv[]) {
  using namespace std;

  std::string pidFile;
  bool pidFileError;
  struct sigaction act, oldact;
  pid_t pid, deadpid;
  miutil::SimpleSocket *pSocket;
  int port;
  int exitStatus;
  int retStatus;
  std::string logfile("");
  TPidMap childMap;
  std::string buf;
  ITPidMap itPid;
  std::string childLog("");
  std::string command("");
  bool interupted;
  char sPid[256];
  std::string confile;
  int timeout;
  miutil::conf::ConfSection *conf;

  confile = kvalobs::kvPath(kvalobs::sysconfdir) + "/aexecd.conf";
  try {
    conf = miutil::conf::ConfParser::parse(confile);
  } catch (const std::exception &ex) {
    conf = 0;
  }
  if (!conf) {
    cerr << "Could not read the configuration file '" << confile << "'.\n\n";
    exit(1);
  }

  InitLogger(argn, argv, "aexecd", conf);

  pidFile = dnmi::file::createPidFileName(kvalobs::kvPath(kvalobs::rundir),
                                          "aexecd");

  if (dnmi::file::isRunningPidFile(pidFile, pidFileError)) {
    if (pidFileError) {
      LOGFATAL(
          "An error occured while reading the pidfile:" << endl << pidFile << " remove the file if it exist and" << endl << "aexecd is not running. If it is running and" << endl << "there is problems. Kill aexecd and" << endl << "restart it." << endl << endl);
      return 1;
    } else {
      LOGFATAL(
          "Is aexecd allready running?" << endl << "If not remove the pidfile: " << pidFile << endl << endl);
      return 1;
    }
  }

  //The pidfile is automatically removed when `thePidfile` goes
  //out of scope, ie when the main function end.
  dnmi::file::PidFileHelper thePidfile(pidFile);

  miutil::SimpleSocketServer server;

  if (argn == 2) {
    port = atoi(argv[1]);
  } else {
    miutil::conf::ValElementList val = conf->getValue("port");
    if (val.empty())
      port = 0;
    else
      port = val[0].valAsInt(0);
  }

  if (port == 0)
    Usage(1);

  LOGINFO(
      "aexecd started. Listening on port: " << port << ". Pidfile '" << pidFile << "'.");

  try {
    miutil::setSignalHandlerThrow( SIGCHLD, sig_child, SA_NOCLDSTOP);
    miutil::setSignalHandlerThrow( SIGTERM, sig_term);
    miutil::setSignalHandlerThrow( SIGINT, sig_term);
  } catch (std::runtime_error &ex) {
    LOGFATAL("ERROR: Can't install signal handlers. (" << ex.what() << ").");
    return 2;
  }

  if (!server.bind(port)) {
    LOGFATAL("Server error, can't bind port (" << port<< ")!");
    return 3;
  }

  if (!server.listen(10)) {
    LOGFATAL("Server error, can't listen to port (" << port << "!");
    return 3;
  }
  LOGINFO("Ready!");

  while (sigTerm == 0) {

    if (sigChild > 0) {
      resetSigChildFlag();

      while ((deadpid = waitpid(-1, &exitStatus, WNOHANG)) > 0) {
        if (WIFEXITED(exitStatus))
          retStatus = WEXITSTATUS(exitStatus);
        else
          retStatus = -1;

        itPid = childMap.find(deadpid);

        if (itPid != childMap.end()) {
          if (retStatus == 0) {
            LOGINFO(itPid->second << ", complited ok.");
          } else {
            LOGERROR(
                itPid->second << ", terminated with exitstatus " << retStatus << ".");
          }
          childMap.erase(itPid);
        } else {
          LOGERROR(
              "ERROR, the child with pid " << deadpid << " is not registred in the childMap.");
        }
      }
    }

    pSocket = server.accept(interupted);

    if (!pSocket) {
      if (!interupted) {
        LOGERROR("Server accepterror, can't accept incomming connection!");
        if (server.errorType() == miutil::socket_status_codes::NO_MEM) {
          LOGFATAL(
              "Server, out of memmory. Exiting and in the hope that someone/something restart me.");
          exit(4);
        }
      }

      //std::cout << "Interupted ..... \n";
      continue;
    }

    LOGDEBUG("New command arrived.");

    int nBytes = pSocket->readln(buf, 10);

    if (nBytes <= 0) {
      if (nBytes == 0) {
        LOGDEBUG("Oppps: nothing arrived!");
      } else {
        LOGERROR("Server: error reading incomming data from the socket!");
      }

      delete pSocket;
      continue;
    }

    if (!GetCommandAndLogfile(buf, command, childLog, timeout)) {
      LOGERROR("Invalidformat: " << buf << ".");

      buf = "ERROR: INVALIDFORMAT '" + buf + "'\n";
      pSocket->writeln(buf);

      delete pSocket;
      continue;
    }

    LOGINFO(
        "Command: " << command << ".\nLogfile: " << childLog << ".\ntimeout: " << timeout << ".");

    pid = fork();

    if (pid < 0) {
      LOGERROR("Command: " << command << ".\n Server: fork error");
    } else if (pid == 0) { /* child process */
      int fdin;
      int fdout;
      int fderr;
      int fd;

      thePidfile.forget();  //Forget the pidfile to the parent.
      server.close(); /* close original socket */
      close(0); /* close stdin  */
      close(1); /* close stdout */
      close(2); /* close stderr */

      fdin = open("/dev/zero", O_RDONLY);
      if (fdin < 0)
        exit(126);

      if (!childLog.empty())
        fd = open(childLog.c_str(), O_WRONLY | O_CREAT | O_TRUNC,
                  S_IRUSR | S_IWUSR | S_IRGRP);
      else
        fd = open("/dev/null", O_WRONLY);

      if (fd < 0)
        exit(125);

      fderr = dup(fd);
      //fdout=dup2(fd, 1);
      //fderr=dup2(fd, 2);

      if (fderr < 0)
        exit(124);

      exechelper::exec(pSocket, command, timeout);
      //DoExec(pSocket, command);

      //Kommer vi hit har ikke DoExec kunnet utført execv.
      //Vi avslutter og returnerer 127 som exitstatus.
      exit(127);
    } else {  //Parent
      std::string cmdMsg;

      sprintf(sPid, "%ld", (long) pid);

      cmdMsg = "[";
      cmdMsg += sPid;
      cmdMsg += "] ";
      cmdMsg += command;
      childMap[pid] = cmdMsg;
      delete pSocket;
    }
  }

  LOGINFO("Terminating.");

  remove(pidFile.c_str());
}

void sig_child(int) {
  sigChild = 1;
}

void sig_term(int) {
  sigTerm = 1;
}

void resetSigChildFlag() {
  try {
    miutil::blocksignalThrow( SIGCHLD);
    sigChild = 0;
    miutil::unblocksignalThrow( SIGCHLD);
  } catch (const std::runtime_error &e) {
    LOGERROR("Failed to reset the sigChild flag.");
    sigChild = 0;  //This is NOT safe to do. It have race.
  }
}

void Usage(int exitcode) {
  printf("Usage: \n\n  aexecd [portnumber]\n\n");
  printf("\tThe port number the aexecd i listening on. The port must\n"
         "\teither be given on the commandline or in the configuration\n"
         "\tfile, aexecd.conf, the variable name is port=portnummer\n\n");

  exit(exitcode);
}

/**
 * GetCommandAndLogFile splitter en streng i formatet
 * "[LOG:logfile;TIMEOUT:timeout]command\n" i logfile og en command.
 *
 * \param buf Et buffer med en streng som er foramatert som beskrevet over.
 * \param command Kommando elementet i buf.
 * \param logfile logfile elementet i bufferet over.
 *
 * \return true dersom buf er p� formen over og vi har plukket ut
 *         kommando og logfile elementene. logfile elementet kan v�re tom.
 */
bool GetCommandAndLogfile(const std::string &buf, std::string &command,
                          std::string &logfile, int &timeout) {
  std::string::const_reverse_iterator rit;
  unsigned int p;
  unsigned int p1;
  std::string tmp;
  vector<string> keyval;
  vector<string> options;

  timeout = 0;
  command = "";
  logfile = "";
  rit = buf.rbegin();

  if (rit == buf.rend())
    return false;

  if (*rit != '\n')
    return false;

  p = buf.find("[");

  if (p == std::string::npos)
    return false;

  p1 = buf.find("]", p);

  if (p1 == std::string::npos)
    return false;

  command = buf.substr(p1 + 1);
  tmp = buf.substr(p + 1, p1 - p - 1);

  if (tmp.empty())
    return false;

  boost::split(options, tmp, boost::is_any_of(";"));

  for (vector<string>::iterator it = options.begin(); it != options.end();
      ++it) {
    LOGINFO("option: '" << *it << "'.");
    boost::split(keyval, *it, boost::is_any_of(":"));

    if (keyval.size() != 2)
      return false;
    if (keyval[0] == "LOG") {
      logfile = keyval[1];
      boost::algorithm::trim(logfile);
    } else if (keyval[0] == "TIMEOUT") {
      string t = keyval[1];
      boost::algorithm::trim(t);
      try {
        timeout = boost::lexical_cast<int>(t);
      } catch (...) {
      }
    }
  }

  boost::algorithm::trim(command);  //remove the newline character (\n).
  return true;
}
