/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvQtApp.h,v 1.12.2.4 2007/09/27 09:02:47 paule Exp $                                                       

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
#ifndef __kvQtApp_h__
#define __kvQtApp_h__

#include <list>
#include <qapplication.h>
#include <kvskel/datasource.hh>
#include <kvalobs/kvData.h>
#include <kvalobs/kvModelData.h>
#include <kvalobs/kvReferenceStation.h>
#include <kvalobs/kvParam.h>
#include <kvalobs/kvStation.h>
#include <kvalobs/kvTypes.h>
#include <kvalobs/kvStationParam.h>
#include <kvalobs/kvObsPgm.h>
#include <kvalobs/kvOperator.h>
#include "WhichDataHelper.h"
#include "kvservicetypes.h"
#include "kvDataSubscribeInfoHelper.h"

namespace boost{
  class thread;
};

namespace kvservice{
    namespace priv{
      class KvQtCorbaThread;
      //	class KvNewDataThread;
      //class KvWhatListQue;
    }
    
    /**
     * \addtogroup kvqt
     * @{
     */
    
  /**
   * \brief A application singleton class for kvservice that use
   *        the Qt library from http://www.trolltech.com/.
   *
   * There is nothing that prevent you from creating more instanceses, 
   * but the behavior is undefined. Don't do it.
   */
    
    
    class KvQtApp : public QApplication
    {
	Q_OBJECT;
	//priv::KvWhatListQue   *whatListQue;
	//	priv::KvNewDataThread *newDataThread_;
	//boost::thread         *newDataThread;
	priv::KvQtCorbaThread *corbaThread_;
	bool corbaIsShutdown;

  protected:
	bool guiapp;
	virtual void customEvent(QCustomEvent *event);
	void quit();
      
  public:
      
      static KvQtApp *kvQApp;
      
      /**
       * \brief Constructor for KvQtApp. There must be one and only one 
       * instance ofthis class in the application, singleton. 
       *
       * There is nothing that prevent you from creating more instanceses 
       * but the behavior is undefined. Don't do it.
       *
       * If this is an GUI application set guiapp to true. If it is 
       * a console application set it to false. This will prevent Qt
       * from any attempt to connect to the X server. Unfortunately, you
       * still have to link with X11.
       *
       * \param argn from main.
       * \param argv from main.
       * \param guiapp, true an event loop for X11 is initiated.
       *                false there is no attempt to connect to an X11 server.
       * \param options can be used to set additionals CORBA option. This
       *                is rarely used. Read the documentation for omniORB
       *                if you intend to use it.
       */
      
      KvQtApp(int &argn, char **argv, 
	      bool guiapp,
	      const char *options[0][2]=0);
      
      virtual ~KvQtApp();
      
	  /**
	   * Get the used path in the CORBA nameserver.
	   */
      std::string corbaName() const;
      
      /**
       * \brief Subscribe on data notify from a kvalobs server.
       *
       * \param info A helper class to set up the stations you want to receive
       *             information from.
       * \param receiver The observer object to receive the information on.
       * \param The member function to receive the information on. This is a 
       *        \em slot in \em receiver.
       * \return subscriberid on success and a empty string on failure.
       */  
      std::string subscribeDataNotify(const KvDataSubscribeInfoHelper &info,
			              const QObject *receiver, 
			              const char *mebmer);

      /**
       * \brief Subscribe on data from a kvalobs server.
       *
       * \param info A helper class to set up the stations you want to receive
       *             data from.
       * \param receiver The observer object to receive the data on.
       * \param The member function to receive the information on. This is a 
       *        \em slot in \em receiver. 
       * \return subscriberid on success and a empty string on failure.
       */  
      std::string subscribeData(const KvDataSubscribeInfoHelper &info,
				const QObject *receiver, 
				const char *mebmer);

      /**
       * \brief Subscribe on up/down hint from a kvalobs server.
       *
       * Every time a kvalobs server ist started an ready it will sends 
       * a up hint to hint-subscribers. Every time the kvalobs server is 
       * stopped a down hint is sendt to the hint-subscriber. There is down 
       * hint may not be sendt in the case of a server crash.
       *
       * \param info A helper class to set up the stations you want to receive
       *             data from.
       * \param receiver The observer object to receive the hints on.
       * \param The member function to receive the hints on. This is a 
       *        \em slot in \em receiver. 
       * \return subscriberid on success and a empty string on failure.
       */  
      std::string subscribeKvHint(const QObject *receiver, 
			          const char *mebmer);
      
      /**
       * \brief Unsubscribe from a kvalobs service.
       *
       * \param subscriberid a id previous received from a call to 
       *        subscribeData, subscribeDataNotify or subscribeKvHint.
       */
      void unsubscribe(const std::string &subscriberid);
    
      /**
       * \brief get data from kvalobs. 
       *
       * Use WhichDataHelper to speciefy which stations you want the data from.
       * If you only specify the stations id 0 you will get data from all 
       * stations that is registred in kvalobs.
       *
       * \param dataList[out] The requested data on success.
       * \param wd use wd to specify which stations you want data from.
       * \return true on success false otherwise.
       */
      bool getKvData(KvObsDataList &dataList,
		     const WhichDataHelper &wd);      

      /**
       * \brief return the param list from kvalobs.
       *
       * \param[out] paramList The paramlist kvalobs use on return.
       * \return true ons success and false otherwise.
       */
      bool getKvParams(std::list<kvalobs::kvParam> &paramList);

      /**
       * \brief return the stationlist in kvalobs.
       *
       * \param stationList[out] The station list in kvalobs on return.
       * \return true ons success and false otherwise.
       */
      bool getKvStations(std::list<kvalobs::kvStation> &stationList);
      
      /**
       * \brief return model data.
       *
       * \param[out] dataList The model data on return.
       * \param wd For which stations do we want the model data for.
       * \return true ons success and false otherwise.
       */
      bool getKvModelData(std::list<kvalobs::kvModelData> &dataList,
			  const WhichDataHelper &wd);


      /**
       * \brief return the referance stastions for a given stationid.
       *
       * \param stationid The stationid for the station we want the referance 
       *        station too.
       * \param paramid The paramid we are interested in.
       * \param[out] refList A list of referance statioins to stationid.
       * \return true ons success and false otherwise.
       */
      bool getKvReferenceStations(int stationid, 
				  int paramid, 
				  std::list<kvalobs::kvReferenceStation> &refList); 

      /**
       * \brief return the tyeps that is used in kvalobs.
       *
       * \param[out] typeList The typeList on return.
       * \return true ons success and false otherwise.
       */
      bool getKvTypes(std::list<kvalobs::kvTypes> &typeList);

      /**
       * \brief return obs_pgm.
       *
       * \param[out] obsPgm The obsPgm on return.
       * \param[oot] stationList List of station numbers
       * \return true ons success and false otherwise.
       */
      bool getKvObsPgm(std::list<kvalobs::kvObsPgm> &obsPgm,
		       const std::list<long> &stationList,
		       bool aUnion);

      /**
       * \brief Get station parameters from the kvalobs database.
       *
       * \param stParam The list to be populated with station parameters.
       * \param stationid the stationid for which parameters is to be fetched.
       * \param paramid What parameterid we want. If paramid < 0: get parameters for all paramid.
       * \param day what day of the year we want parameters for. day < 0 indicates all days of the year.
       *
       * \return true on success, otherwqise false.
       */
      bool getKvStationParam( std::list<kvalobs::kvStationParam> &stParam,
			      int stationid, int paramid = -1, int day = -1 );


      /**
       * \brief return the operators that has access to manipulate
       *        data in kvalobs through the HQC application.
       *
       * \param[out] operatorList The operators on return.
       * \return true ons success and false otherwise.
       */
      bool getKvOperator(std::list<kvalobs::kvOperator> &operatorList);

          /**
     * \brief sendDataToKv, sends data to kvalobs, kvDataInputd. 
     * 
     * ObsType is used to tell kvalobs what format the data is coded in. It 
     * is used by kvDataInputd to select the proper decoder to be used
     * to decode the data.  <p\>
     * 
     * The method return a Result. The Result has two fields.
     * <pre>
     * - EResult res
     * - string  message
     *
     *  The value of res:
     *     NODECODER,   there is no decoder for the obsType.
     *                  The observation is not saved to the database. Don't
     *                  mind to retry to send the observation until a
     *                  decoder is written and installed.
     *     DECODEERROR, cant decode the message. The
     *                  message is saved to rejectdecode.
     *     NOTSAVED,    the message is not SAVED to the database,
     *                  if possible try to resend it later, after 
     *                  a delay.
     *     ERROR,       A general error. Look at the 'message'. The
     *                  observation is not saved to the database.
     *     OK           The message is decoded and saved to the 
     *                  database.
     * </pre>
     *  If the value of res is NOT OK a error message is written in message.
     *
     * @param data the data coded in the format given with obsType.
     * @param obsType the format of the data.
     * @return A reference to a Result if we successfully connected to 
     *         kvinput. nil if we failed to connect with kvinput. kvinput 
     *         may be down or the CORBA nameserver may be down.
     */

    const CKvalObs::CDataSource::Result_var
      sendDataToKv(const char *data, const char *obsType);



  public slots:
    /**
     * A slot that must be called when a application want to quit.
     */
    void onQuit();
      
  signals:
  
  /**
   * \brief Connect this signal to a slot you want to receive data
   * notification on.
   * 
   * \param what A list of station that has new data.
   */
      void kvDataNotify(kvservice::KvWhatListPtr what);

      /**
       * \brief Connect this signal to a slot you want to receive data on.
       *
       * \param data The data.
       */ 
      void kvData(kvservice::KvObsDataListPtr data);

      
      /**
       * \brief Connect this signal to a slot you want to receive up/down
       * hint on.
       *
       * \param commingUp CommingUp is true if the kvalobs server that you are
       *        subscribed to is ready. It is false if the kvalobs server is 
       *        going down.
       */
      void kvHint(bool commingUp);
  };

    /** @} */
}

#endif
