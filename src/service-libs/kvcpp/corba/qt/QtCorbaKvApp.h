/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: QtCorbaKvApp.h,v 1.3.2.2 2007/09/27 09:02:45 paule Exp $                                                       

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
#ifndef __kvservice__corba__qt__QtCorbaKvApp_h__
#define __kvservice__corba__qt__QtCorbaKvApp_h__

#include <qapplication.h>
#include "../CorbaKvApp.h"

namespace kvservice
{
  /**
   * \addtogroup kvcpp
   * @{
   */
  namespace corba
  {
    /**
     * \addtogroup corba
     * @{
     */
    namespace qt
    {
      /**
       * \addtogroup qt
       * @{
       */

      /**
       * \brief The Qt version of CorbaKvApp, supporting a few signals.
       *
       * This version of CorbaKvApp can be used if you use Qt as a base for
       * your application. Note that this class also extends QApplication, so
       * you should not create any other instances of that class.
       */
      class QtCorbaKvApp
	: public QApplication
	, public CorbaKvApp
      {
	Q_OBJECT;
      public:

        /**
         * An alternate pointer to the KvApp::kvApp, for compatibility with the
         * old kvservice/qt/kvQtApp.h
         */
	static QtCorbaKvApp * kvQApp;

	QtCorbaKvApp( int &argc, char **argv, bool guiApp, 
		      miutil::conf::ConfSection *conf,
		      const char *options[][2] = NULL );

	virtual ~QtCorbaKvApp();

	virtual void run();

        /**
         * \brief Subscribe to DataNotify events, getting the data throgh the <code> kvDataNotify </code> signal.
         *
         * If receiver and member is not 0, the signal will be connected to
         * slot (or signal) member of object receiver.
         */
        virtual std::string
	subscribeDataNotify(const KvDataSubscribeInfoHelper &info,
			    const QObject *receiver = 0, const char *member  = 0);

        /**
         * \brief Subscribe to Data events, getting the data throgh the <code> kvData </code> signal.
         *
         * If receiver and member is not 0, the signal will be connected to
         * slot (or signal) member of object receiver.
         */
        virtual std::string
	subscribeData(const KvDataSubscribeInfoHelper &info,
		      const QObject *receiver = 0, const char *member  = 0);
	
        /**
         * \brief Subscribe to Hint events, getting the data throgh the <code> kvHint </code> signal.
         *
         * If receiver and member is not 0, the signal will be connected to
         * slot (or signal) member of object receiver.
         */
        virtual std::string
	subscribeKvHint(const QObject *receiver = 0, const char *member = 0);

	virtual bool event( QEvent * e );

	using CorbaKvApp::subscribeDataNotify;
	using CorbaKvApp::subscribeData;
	using CorbaKvApp::subscribeKvHint;


      signals:
	void kvDataNotify(kvservice::KvWhatListPtr what);
	void kvData(kvservice::KvObsDataListPtr data);
	void kvHint(bool comingUp);
      protected:
        /**
         * \brief The queue used for getting events from kvalobs that should be sent as signals.
         */
        dnmi::thread::CommandQue signalQueue;

      private:
        struct mainthread;
	friend struct mainthread;
      };
    }
  }
}

#endif // __kvservice__corba__qt__QtCorbaKvApp_h__
