/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: KvApp.h,v 1.3.2.4 2007/09/27 09:02:45 paule Exp $                                                       

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
#ifndef __kvservice__KvApp_h__
#define __kvservice__KvApp_h__

#include <list>
#include <boost/utility.hpp>
#include <kvskel/datasource.hh>
#include <kvalobs/kvData.h>
#include <kvalobs/kvModelData.h>
#include <kvalobs/kvReferenceStation.h>
#include <kvalobs/kvParam.h>
#include <kvalobs/kvStation.h>
#include <kvalobs/kvStationParam.h>
#include <kvalobs/kvStationMetadata.h>
#include <kvalobs/kvTypes.h>
#include <kvalobs/kvObsPgm.h>
#include <kvalobs/kvOperator.h>
#include <dnmithread/CommandQue.h>
#include "WhichDataHelper.h"
#include "KvGetDataReceiver.h"
#include "RejectdecodeIterator.h"
#include "kvDataSubscribeInfoHelper.h"


namespace kvservice
{  
  /**
   * \addtogroup kvcpp
   * @{
   */

  namespace details
  {
    /**
     * \brief Various functions to get data from kvalobs.
     */
    class KvalobsGet
    {
    public:
      virtual ~KvalobsGet() {}

      /**
       * \brief Get observation data from kvalobs
       *
       * \param dataReceiver The recepient object.
       * \param wd A specification of what stations we want.
       * \return True on success, otherwise false.
       */
      virtual bool getKvData( KvGetDataReceiver &dataReceiver, const WhichDataHelper &wd ) =0;
      
      /**
       * \brief Get messages that kvalobs has rejected. \brief
       *
       * \param decodeInfo Informtion on the type of rejects we want.
       * \param it Handler object to receive info.
       * \return True  on success, otherwise false.
       */
      virtual bool getKvRejectDecode( const CKvalObs::CService::RejectDecodeInfo &decodeInfo, kvservice::RejectDecodeIterator &it ) =0;
      

      /**
       * \brief return the param list from kvalobs.
       *
       * \param[out] paramList The paramlist kvalobs use on return.
       * \return true ons success and false otherwise.
       */
      virtual bool getKvParams( std::list<kvalobs::kvParam> &paramList ) =0;

      /**
       * \brief return the stationlist in kvalobs.
       *
       * \param stationList[out] The station list in kvalobs on return.
       * \return true ons success and false otherwise.
       */
      virtual bool getKvStations( std::list<kvalobs::kvStation> &stationList ) =0;

      /**
       * \brief return model data.
       *
       * \param[out] dataList The model data on return.
       * \param wd For which stations do we want the model data for.
       * \return true ons success and false otherwise.
       * \see getKvData(KvGetDataReceiver &dataReceiver,const WhichDataHelper &wd)
       *
       */
      virtual bool getKvModelData( std::list<kvalobs::kvModelData> &dataList, const WhichDataHelper &wd ) =0;

      /**
       * \brief return the referance stastions for a given stationid.
       *
       * \param stationid The stationid for the station we want the referance station too.
       * \param paramid The paramid we are interested in.
       * \param[out] refList A list of referance statioins to stationid.
       * \return true ons success and false otherwise.
       */
      virtual bool getKvReferenceStations( int stationid, int paramid, std::list<kvalobs::kvReferenceStation> &refList ) =0;

      /**
       * \brief return the tyeps that is used in kvalobs.
       *
       * \param[out] typeList The typeList on return.
       * \return true ons success and false otherwise.
       */    
      virtual bool getKvTypes( std::list<kvalobs::kvTypes> &typeList ) =0;

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
      virtual bool getKvOperator( std::list<kvalobs::kvOperator> &operatorList ) =0;

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
      virtual bool getKvStationParam( std::list<kvalobs::kvStationParam> &stParam, int stationid, int paramid = -1, int day = -1 ) =0;

     /**
      * \brief Get metadata for a specific station.
      *
      * \param stMeta The list to be populated with station metadata.
      * \param stationid the stationid for which parameters is to be fetched.
      *    If stationid == -1, request metadata for all stations defined in the
      *    station table.
      * \param obstime The obsetime we want metadata to. If obstime is undef return all metadata regardless of obstime.
      * \param metadataName The name of the metadata we want. An empty string
      *    request all metadata for the stationd.
      *
      * \return on success and false othewise.
      */
      virtual bool getKvStationMetaData( std::list<kvalobs::kvStationMetadata> &stMeta, int stationid, const miutil::miTime &obstime, const std::string & metadataName = "") =0;

       /**
       * \brief return the observation programs.
       *
       * \param[out] obsPgm The observations programs.
       * \param stationList Return the observations program for this stations.
       *                    A empty list returns the observations program for 
       *                    all stations.
       * \param aUnion Return a only one kvObsPgm for every station. This 
       *               kvObsPgm is a union of of all paramid in a obsservation
       *               prorogram for a station. ie. we expect some sort of 
       *               data for the timespec in the kvObsPgm. 
       * \return true ons success and false otherwise.
       */
      virtual bool getKvObsPgm( std::list<kvalobs::kvObsPgm> &obsPgm, const std::list<long> &stationList, bool aUnion ) =0;

      /**
       * \deprecated
       */
      virtual bool getKvData( KvObsDataList &dataList, const WhichDataHelper &wd ) =0;
    };


    /**
     * \brief Functions for sending data to kvalobs.
     */
    class KvalobsSend
    { 
    public:
      virtual ~KvalobsSend() {}

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
      virtual const CKvalObs::CDataSource::Result_var sendDataToKv( const char *data, const char *obsType ) =0;
    };

    /**
     * \brief Functions for subscribing to data from kvalobs
     */
    class KvalobsSubscribe
    {
    public:
      typedef std::string SubscriberID;

      virtual ~KvalobsSubscribe() {}

      /**
       * \brief Subscribe to DataNotify events. 
       *
       * The events is posted on the que as DataNotifyEvent. The 
       * DataNotifyEvent is declared in the file \em kvevents.h.
       *
       * \param que The que to receive DataNotifyEvent.
       * \return subscriberid on success and a empty string on failure.
       */  
      virtual SubscriberID subscribeDataNotify( const KvDataSubscribeInfoHelper &info, dnmi::thread::CommandQue &que ) =0;

      /**
       * \brief Subscribe to Data events. 
       *
       * The events is posted on the que as DataEvent. The DataEvent 
       * is declared in the file \em kvevents.h.
       *
       * \param que The que to receive DataEvent.
       * \return subscriberid on success and a empty string on failure.
       */  
      virtual SubscriberID subscribeData( const KvDataSubscribeInfoHelper &info, dnmi::thread::CommandQue &que ) =0;

      /**
       * \brief Subscribe to Hint events. 
       *
       * The events is posted on the que as HintEvent. The HintEvent is 
       * declared in the file \em kvevents.h.
       *
       * \param que The que to receive HintEvent.
       * \return subscriberid on success and a empty string on failure.
       */  
      virtual SubscriberID subscribeKvHint( dnmi::thread::CommandQue &que ) =0;  

      /**
       * \brief Unsubscribe from a service.
       *
       * Tell the kvService that we are no longer interessted in 
       * notifications.
       */
      virtual void unsubscribe( const SubscriberID &subscriberid ) =0;

      /**
       * Unsubscribe from all services.
       */
      virtual void unsubscribeAll() =0;
    };

    class KvAppControl
    {
    public:

      /**
       * \brief the shutdown status.
       *
       * \return true if we are in a shutdown.
       */
      virtual bool shutdown() const =0;

      /**
       * \brief request shutdown, ie we want to terminate.
       */
      virtual void doShutdown() =0;

      /**
       * \brief start the eventloop.
       * 
       * Must be called for every aplication that calls one of the subscriber
       * functions.
       *
       * It doesn't return before the application is ready to terminate,
       * ie doShutdown is called or SIGQUIT/SIGINT is received.
       * 
       * The event loop is in principle run as follow 
       * 
       * run() {
       * 	
       *    ......   
       * 
       *    while( not in shutdown ) {
       * 		work();
       * 		
       *       ....
       *    }
       *   
       *     .....
       * }
       * 
       */
      virtual void run() =0;
       
      

      /**
       * \brief work is called on every turn of the event loop.
       */
      virtual void work() {};

    protected:
      virtual ~KvAppControl() {}
    };
  }
    
  /**
   * \brief A application singleton class for kvservice applications.
   *
   * Note that all methods in this class are abstract.
   *
   * There is nothing that prevent you from creating more instances
   * but the behavior is undefined. Don't do it.
   */
  class KvApp
    : private boost::noncopyable
    , public details::KvalobsGet
    , public details::KvalobsSend
    , public details::KvalobsSubscribe
    , public details::KvAppControl
  {
  public:

    /**
     * \brief A pointer to the KvApp singleton, if one has been instatiated.
     */
    static KvApp *kvApp;
              
    KvApp();

  protected:
    virtual ~KvApp();
  };

  /** @} */
}

#endif //__kvservice__KvApp_h__
