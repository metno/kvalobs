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

#include <stdlib.h>
#include <iostream>
#include <string>
#include "kvalobs/kvPath.h"
#include "miconfparser/miconfparser.h"
#include "miutil/aexecclient.h"

using namespace std;

int
main( int argn, char **argv )
{
    string confile;
    miutil::conf::ConfSection *conf;
    string logdir=kvalobs::kvPath( kvalobs::logdir );
    string cmd;

    confile =  kvalobs::kvPath(kvalobs::sysconfdir) + "/aexecd.conf";
    try {
        conf = miutil::conf::ConfParser::parse( confile );
    }
    catch( const std::exception &ex ) {
        conf = 0;
    }

    if( ! conf ) {
        cerr << "Could not read the configuration file '" << confile << "'.\n\n";
        exit(1);
    }

    miutil::conf::ValElementList val = conf->getValue("port");

    if( val.empty() ) {
        cerr << "aexecd no port defined in the configuration file '" << confile << "'.\n\n";
        exit(1);
    }

    int port = val[0].valAsInt( -1 );

    if( port == -1 ) {
        cerr << "aexecd no port defined in the configuration file '" << confile << "'.\n\n";
        exit(1);
    }

    miutil::AExecClient aclt("localhost", port );
    string logfile=logdir+"/env.log";
    unlink( logfile.c_str() );

    cmd ="/home/borgem/projects/build/kvalobs_1/mytest.sh";

    if( argn > 1 ) {
        cmd += " " + string( argv[1] );
    } else {
        cmd += " 2";
    }

    pid_t pid = aclt.exec( cmd, logfile );

    if( pid == 0 ) {
        cerr << "aexecd, could not start command.\n\n";
        exit( 1 );
    }

    int ret = aclt.wait();

    if( ret < 0 ) {
        cerr << "aexecd failed: " << aclt.getErrMsg() << (aclt.timeout()?" (timeout)":"") <<".\n";
    } else {
        cerr << "aexecd: cmd exitstatus: " << ret << ".\n";
    }

    system( string( "cat " + logfile).c_str() );
}



