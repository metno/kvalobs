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
#include <string.h>
#include <errno.h>
#include <sstream>

#include "signalhelpers.h"

namespace miutil {

bool setSignalHandler(int signum, void (*sigfunc)(int signum), int sa_flags,
                      struct sigaction *oldact) {
  struct sigaction myact;
  struct sigaction newact;

  if (!oldact)
    oldact = &myact;

  newact.sa_handler = sigfunc;
  sigemptyset(&newact.sa_mask);
  newact.sa_flags = sa_flags;

  if (sigaction(signum, &newact, oldact) < 0)
    return false;

  return true;
}

void setSignalHandlerThrow(int signum, void (*sigfunc)(int signum),
                           int sa_flags, struct sigaction *oldact) {
  if (!setSignalHandler(signum, sigfunc, sa_flags, oldact)) {
    std::ostringstream ost;
    char errbuf[256];
    ost << "setSignalHandler: signum: " << signum << " errno: " << errno << ".";
    if (strerror_r( errno, errbuf, 256) == 0)
      ost << " " << errbuf;
    throw std::runtime_error(ost.str());
  }
}

bool blocksignal(int signum) {
  sigset_t newmask;

  sigemptyset(&newmask);
  sigaddset(&newmask, signum);

  if (sigprocmask(SIG_BLOCK, &newmask, 0) < 0)
    return false;
  else
    return true;
}

void blocksignalThrow(int signum) {
  if (!blocksignal(signum)) {
    std::ostringstream ost;
    char errbuf[256];
    ost << "blocksignal: signum: " << signum << " errno: " << errno << ".";
    if (strerror_r( errno, errbuf, 256) == 0)
      ost << " " << errbuf;
    throw std::runtime_error(ost.str());
  }
}

bool unblocksignal(int signum) {
  sigset_t newmask;

  sigemptyset(&newmask);
  sigaddset(&newmask, signum);

  if (sigprocmask(SIG_UNBLOCK, &newmask, 0) < 0)
    return false;
  else
    return true;
}

void unblocksignalThrow(int signum) {
  if (!unblocksignal(signum)) {
    std::ostringstream ost;
    char errbuf[256];
    ost << "unblocksignal: signum: " << signum << " errno: " << errno << ".";
    if (strerror_r( errno, errbuf, 256) == 0)
      ost << " " << errbuf;
    throw std::runtime_error(ost.str());
  }
}

}

