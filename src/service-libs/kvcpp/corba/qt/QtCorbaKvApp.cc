/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: QtCorbaKvApp.cc,v 1.2.2.3 2007/09/27 09:02:46 paule Exp $                                                       

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
#include "../../../include/corba/qt/QtCorbaKvApp.h"
#include "../../../include/kvevents.h"

using namespace std;
using namespace kvservice;
using namespace miutil;
using namespace dnmi::thread;

namespace kvservice
{
  namespace corba
  {
    namespace qt
    {
      namespace{
	class GotDataEvent 
	  : public QEvent
	{
	  CommandBase * cmdBase;
	public:
	  static const QEvent::Type eventVal = (QEvent::Type) 1042;
	  GotDataEvent( CommandBase * base ) 
	    : QEvent( eventVal ), cmdBase( base ) { }
	  virtual ~GotDataEvent() {
	    delete cmdBase;
	  }
	  CommandBase * getCmd() {
	    return cmdBase;
	  }
	};
      }


      QtCorbaKvApp * QtCorbaKvApp::kvQApp = 0;

      QtCorbaKvApp::QtCorbaKvApp( int &argc, char **argv, bool guiApp, 
				  conf::ConfSection *conf,
				  const char *options[][2] )
	: QApplication( argc, argv, guiApp )
	, CorbaKvApp( argc, argv, conf, options )
      {
	kvQApp = this;
      }

      QtCorbaKvApp::~QtCorbaKvApp()
      {
	kvQApp = 0;
      }

      QtCorbaKvApp::SubscriberID 
      QtCorbaKvApp::subscribeDataNotify(const KvDataSubscribeInfoHelper &info,
					const QObject *receiver, const char *member)
      {
	SubscriberID ret = subscribeDataNotify( info, signalQueue );
	if ( (not ret.empty()) and receiver and member ) {
	  if ( ! connect( this, SIGNAL( kvDataNotify(kvservice::KvWhatListPtr) ),
			  receiver, member ) ) {
	    unsubscribe( ret );
	    return "";
	  }
	}
	return ret;
      }

      QtCorbaKvApp::SubscriberID 
      QtCorbaKvApp::subscribeData(const KvDataSubscribeInfoHelper &info,
				  const QObject *receiver, const char *member)
      {
	SubscriberID ret = subscribeData( info, signalQueue );
	if ( (not ret.empty()) and receiver and member ) {
	  if ( ! connect( this, SIGNAL( kvData(kvservice::KvObsDataListPtr) ),
			  receiver, member ) ) {
	    unsubscribe( ret );
	    return "";
	  }
	}
	return ret;
      }

      QtCorbaKvApp::SubscriberID 
      QtCorbaKvApp::subscribeKvHint(const QObject *receiver, const char *member)
      {
	SubscriberID ret = subscribeKvHint( signalQueue );
	if ( (not ret.empty()) and receiver and member ) {
	  if ( ! connect( this, SIGNAL( kvHint(bool) ), receiver, member ) ) {
	    unsubscribe( ret );
	    return "";
	  }
	}
	return ret;
      }

      bool QtCorbaKvApp::event( QEvent *e )
      {
	if ( e->type() == GotDataEvent::eventVal ) {
	  GotDataEvent * gde = dynamic_cast<GotDataEvent*>( e );
	  if ( gde ) {
	    CommandBase *base = gde->getCmd();
	    DataEvent *dataEvent = dynamic_cast<DataEvent*>( base );
	    if ( dataEvent ) {
	      kvservice::KvObsDataListPtr data = dataEvent->data();
	      emit kvData( data );
	      return true;
	    }
	    DataNotifyEvent *dataNotifyEvent = dynamic_cast<DataNotifyEvent*>( base );
	    if ( dataNotifyEvent ) {
	      KvWhatListPtr what = dataNotifyEvent->what();
	      emit kvDataNotify( what );
	      return true;
	    }
	    HintEvent *hintEvent = dynamic_cast<HintEvent*>( base );
	    if ( hintEvent ) {
	      emit kvHint( hintEvent->upEvent() );
	      return true;
	    }
	  }
	}
	if ( e->type() == (QEvent::Type) 1043 )
	  qApp->quit();
	return false;
      }

      struct QtCorbaKvApp::mainthread 
      {
	void operator()()
	{
	  QtCorbaKvApp * app = QtCorbaKvApp::kvQApp;
	  while ( ! qApp )
	    sleep( 1 );
	  CommandBase *com = 0;
	  while ( not app->shutdown() ) {
	    com = app->signalQueue.get( 1 );
	    if ( com )
	      qApp->postEvent( QtCorbaKvApp::kvQApp, new GotDataEvent( com ) );
	  }
	  qApp->postEvent( qApp, new QEvent( (QEvent::Type) 1043 ) );
	}
      };

      void QtCorbaKvApp::run()
      {
	mainthread mt;
	boost::thread t( mt );
	qApp->exec();
	doShutdown();
	t.join();
      }
    }
  }
}
