/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id$                                                       

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
#include "ProcessImpl.h"
#include "BasicStatistics.h"
#include "Qc2App.h"
#include "Qc2Connection.h"
#include "Qc2D.h"
#include "ReadProgramOptions.h"
#include <sstream>
#include <milog/milog.h>
#include <kvalobs/kvDbGate.h>
#include <puTools/miTime>
#include <memory>
#include <stdexcept>

#include "ProcessControl.h"
#include "CheckedDataCommandBase.h"
#include "CheckedDataHelper.h"

#include "scone.h"
#include "tround.h"

using namespace kvalobs;
using namespace std;
using namespace miutil;

int 
ProcessImpl::
FlagTester( ReadProgramOptions params )
{
  LOGINFO("FlagTester");
  int pid=params.pid;
  miutil::miTime stime=params.UT0;
  miutil::miTime etime=params.UT1;
  std::string CIF=params.ControlInfoString;

  std::list<kvalobs::kvData> Qc2Data;
  std::list<kvalobs::kvData> ReturnData;
  bool result;

  ProcessControl CheckFlags;
  kvalobs::kvControlInfo fixflags;
  kvData d;

  kvUseInfo ui = d.useinfo();
  std::cout << d.controlinfo() << std::endl;
  std::cout << ui << std::endl;
  ui.setUseFlags( d.controlinfo() );
  std::cout << ui << std::endl;
  std::cout << "---------------" << std::endl;

return 0;
}

