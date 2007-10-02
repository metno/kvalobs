/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kv2kvTest.cc,v 1.1.2.1 2007/09/27 09:02:45 paule Exp $                                                       

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
#include <KvAppSimple.h>
#include <decodeutility/kvDataFormatter.h>
#include <kvalobs/kvData.h>
#include <miutil/miString>
#include <WhichDataHelper.h>
#include <cassert>
#include <iostream>

using namespace std;
using namespace kvservice;
using namespace kvalobs;
using namespace decodeutility::kvdataformatter;

class ReEmitter : public KvAppSimple {

  static const float correction = 3.14;

public:

  ReEmitter(int argn, char **argv, miutil::conf::ConfSection *conf)
    :KvAppSimple(argn, argv, conf) {
  }
  
  void onKvHintEvent(bool up){
    if(up){
      cerr << "KvUpEvent received!" << endl;
    }else{
      cerr << "KvDownEvent received!" << endl;
    }
  }

  void onKvDataEvent(KvObsDataListPtr data) {

    assert(!data->empty());

    for (IKvObsDataList it = data->begin(); it != data->end(); it++) {
      assert(!it->empty());
      for (IKvDataList it2 = it->begin(); it2 != it->end(); it2++)
	it2->corrected(correction);
      miutil::miString s = createString( *it );

      //cerr << s << endl;
      cerr << "DATA!" << endl;
    }
  }
};


int main(int argc, char **argv) {

  //WhichDataHelper wd = 0;

  ReEmitter e(argc, argv, KvAppSimple::readConf("test.conf"));
  //e.getKvData(, wd);
  e.run();
}
