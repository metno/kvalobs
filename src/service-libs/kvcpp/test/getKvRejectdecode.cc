/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: getKvRejectdecode.cc,v 1.1.2.2 2007/09/27 09:02:45 paule Exp $                                                       

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
#include <iostream>
#include <KvApp.h>
#include <kvRejectdecode.h>

using namespace std;
using namespace kvalobs;
using namespace kvservice;
using namespace CKvalObs::CService;

int main( int argc, char **argv ) 
{
  KvApp app( argc, argv, KvApp::readConf("test.conf") );

  RejectDecodeInfo rdi;

  if ( argc > 1 ) {
    rdi.decodeList.length( argc -1 );
    for ( int i = 1; i < argc; i++ ) {
      cerr << argv[i] << endl;
      rdi.decodeList[i -1] = argv[i];
    }
  }

  rdi.fromTime      = "2004-12-01 06:00:00";
  rdi.toTime        = "2005-04-01 06:00:00";

  kvservice::RejectDecodeIterator it;

  {
    app.getKvRejectDecode( rdi, it );
    kvRejectdecode rDec;  
    it.next( rDec );
  }

  cerr << "getIterator success: " 
       << app.getKvRejectDecode( rdi, it ) << endl;

  kvRejectdecode rDec;  
  while ( it.next( rDec ) )
    cout << rDec << endl;

  return 0;
}
