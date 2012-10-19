/*
  Kvalobs - Free Quality Control Software for Meteorological Observations

  $Id: kvsynopd.cc,v 1.12.2.11 2007/09/27 09:02:23 paule Exp $

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
#include <unistd.h>
#include <sstream>
#include <fstream>
#include <list>
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <milog/milog.h>
#include <fileutil/pidfileutil.h>
#include <fileutil/dir.h>
#include <miconfparser/miconfparser.h>
#include <kvalobs/kvapp.h>
#include <kvalobs/kvPath.h>
#include <kvcpp/corba/CorbaKvApp.h>
#include <kvcpp/KvGetDataReceiver.h>

using namespace std;
using namespace miutil;


miutil::conf::ConfSection*
loadConfFile( const std::string &file="");


/**
   * KvGetDataReceiver is an interface used to retrive data
   * from kvalobs. It is used by KvApp::getKvData.
   */

class GetDataReceiver :
	public kvservice::KvGetDataReceiver
{

  public:

	miutil::miTime prevTime;
	int nObstimes;
	int nParams;

	GetDataReceiver()
		: nObstimes(0), nParams(0)
		  {
		  }

    virtual bool next(kvservice::KvObsDataList &datalist)
    {
    	using namespace kvservice;

    	for( KvObsDataList::iterator it = datalist.begin(); it != datalist.end(); ++it ) {
    		KvObsData::kvDataList &data=it->dataList();
    		KvObsData::kvTextDataList textData=it->textDataList();

    		for( KvObsData::kvDataList::iterator dit=data.begin(); dit != data.end(); ++dit ) {
    			if( prevTime.undef() ) {
    				prevTime = dit->obstime();
    				nObstimes++;
    			} else if( prevTime != dit->obstime() ) {
    				prevTime = dit->obstime();
    				nObstimes++;
    			}

    			nParams++;
    			cerr << dit->stationID() << "," << dit->obstime() << "," << dit->typeID() << ","
    				 << dit->paramID() << "," << dit->original() << "," << dit->corrected() << ": Data" << endl;
    		}

    		for( KvObsData::kvTextDataList::iterator dit=textData.begin(); dit != textData.end(); ++dit ) {
    			cerr << dit->stationID() << "," << dit->obstime() << "," << dit->typeID() << ","
    				 << dit->paramID() << "," << dit->original()  << ": TextData" <<endl;
    		}

    		cerr << " -----------------------------------------------------------------------  " << endl;
    	}
    }
};



class GetThread
{
	GetThread();
	kvservice::corba::CorbaKvApp *app;

public:
	struct Results{
		int nRuns;
		int nFailed;
		int nUnexpected;
		int tid;

		Results( int tid_ )
			: nRuns( 0 ), nFailed(0), nUnexpected( 0 ), tid( tid_ ) {}

		Results( const Results &r )
			: nRuns( r.nRuns ), nFailed( r.nFailed ), nUnexpected( r.nUnexpected ), tid( r.tid ){}
	};

	boost::shared_ptr<Results> results;
	GetThread( kvservice::corba::CorbaKvApp *app_, int tid_ )
		: app( app_ ), results( new Results( tid_) ){}

	GetThread( const GetThread &gt )
		: app( gt.app ), results( gt.results )
	{
	}

	void operator()() {
		if( ! app )
			return;

		bool first=true;
		int nObstimes;
		int nParams;
		kvservice::WhichDataHelper whichData;
        //whichData.addStation( 18700, miutil::miTime("2006-02-06 06:00:00"), miutil::miTime("2006-02-06 06:00:00"));
		whichData.addStation( 0, miutil::miTime("2010-02-22 06:00:00"), miutil::miTime("2010-02-22 06:00:00"));
		//whichData.addStation( 18700, miutil::miTime("2006-02-06 06:00:00"), miutil::miTime("2006-02-10 06:00:00"));
		//whichData.addStation( 18700, miutil::miTime("2010-02-11 10:00:00"), miutil::miTime("2010-02-12 09:00:00"));

		for( int N=1; N>0; N-- ) {
			GetDataReceiver dataReceiver;
			results->nRuns++;

			if( !app->getKvData( dataReceiver, whichData ) ) {
				cerr <<  results->tid << " (" << N <<") FAILED!!!!!!" << endl;
				results->nFailed++;
			} else {
				if( first ) {
					first = false;
					nObstimes = dataReceiver.nObstimes;
					nParams = dataReceiver.nParams;
				} else if( nObstimes!=dataReceiver.nObstimes || nParams != dataReceiver.nParams ) {
					results->nUnexpected++;
				}

				cerr << results->tid << " (" << N <<")" << " nObstimes: " << dataReceiver.nObstimes << " nParams: " << dataReceiver.nParams << endl;
			}
		}
	}

};



int
main(int argn, char **argv)
{
  bool error;
  std::string pidfile;
  std::string confFile;

  confFile = "stressTestKvGetData.conf";


  miutil::conf::ConfSection *conf=loadConfFile();

  if(  !conf ) {
	  ostringstream ost;

	  ost << "No working configuration file found!" << endl;
	  ost << "  searched: ./stressTestKvGetData.conf" << endl;
	  ost << "  searched: $HOME/.stressTestKvGetData.conf" << endl;
	  ost << "  searched: $HOME/etc/stressTestKvGetData.conf" << endl;

	  LOGFATAL( ost.str() );

	  return 1;
  }

  kvservice::corba::CorbaKvApp app( argn, argv, conf );
  boost::thread_group getThreadsGroup;
  GetThread *th;
  std::list<GetThread*> threadList;
  int nThreads=1;

  for( int i=0; i<nThreads; ++i ) {
	  th = new GetThread( &app, i );
	  threadList.push_back( th );
	  getThreadsGroup.add_thread( new boost::thread( *th ) );
  }

  getThreadsGroup.join_all();

  for( std::list<GetThread*>::iterator it = threadList.begin(); it != threadList.end(); ++it )
  {
	  cerr << (*it)->results->tid << ": runs: " << (*it)->results->nRuns
		   << " Failed: " << (*it)->results->nFailed
		   << " Unexpected: " << (*it)->results->nUnexpected << endl;
  }

}

miutil::conf::ConfSection *
loadConfFile( const std::string &file)
{
	string conf;

	if( ! file.empty() )
		conf = file;

	if( ! conf.empty() ) {
		cerr << "Checking: " << conf << endl;

		if( ! dnmi::file::canRead( "stressTestKvGetData.conf") )
			return 0;
	}

	if( conf.empty() ) {
		conf = "stressTestKvGetData.conf";
		cerr << "Checking: " << conf << endl;

		if( ! dnmi::file::canRead( conf ) )
			conf.erase();
	}

	if( conf.empty() ) {
		char *homep = getenv( "HOME" );

		if( !homep )
			return 0;

		string home( homep );

		conf = home+"/.stressTestKvGetData.conf";

		cerr << "Checking: " << conf << endl;

		if( ! dnmi::file::canRead( conf ) )
			conf.erase();

		if( conf.empty() ) {
			conf = home+"/etc/stressTestKvGetData.conf";

			cerr << "Checking: " << conf << endl;
			if( ! dnmi::file::canRead( conf ) )
				conf.erase();
		}
	}

	if( conf.empty() )
		return 0;

	LOGINFO("Using configuration file: " << conf );

	try {
		return miutil::conf::ConfParser::parse( conf );
	}
	catch( const logic_error &ex ){
		LOGFATAL( ex.what() );
		return 0;
	}

	return 0;
}


