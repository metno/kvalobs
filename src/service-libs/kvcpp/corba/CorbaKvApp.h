/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: CorbaKvApp.h,v 1.3.2.3 2007/09/27 09:02:45 paule Exp $                                                       

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
#ifndef __kvservice__corba__CorbaKvApp_h__
#define __kvservice__corba__CorbaKvApp_h__

#include "../KvApp.h"
#include "kvCorbaThread.h"
#include "kvDataNotifySubscriberImpl.h"
#include "kvDataSubscriberImpl.h"
#include "kvHintSubscriberImpl.h"
#include <miconfparser/confsection.h>
#include <corbahelper/corbaApp.h>
#include <kvskel/kvService.hh>
#include <stdexcept>
#include <string>

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

    /**
     * \brief Could not find nameserver or kvalobs
     */
    class LookUpException
      : public std::runtime_error
    {
    public:
      LookUpException( const char * what )
        : runtime_error( what )
      {
      }
    };
    //typedef std::runtime_error LookUpException;

    /**
     * \brief The CORBA implementation of KvApp
     */
    class CorbaKvApp
      : public KvApp
      , protected CorbaHelper::CorbaApp
    {
    protected:
      ::kvservice::priv::KvCorbaThread *corbaThread;

      CKvalObs::CService::kvService_var  refService;
      CKvalObs::CDataSource::Data_ptr dataInput;

      std::string nameserverpath;
      std::string nameserver;

      bool shutdown_;

      typedef omni_mutex Mutex;
      typedef omni_mutex_lock Lock;
      Mutex mutex;
      

      typedef std::map<std::string, kvservice::priv::DataNotifySubscriber*> DataNotifyList;
      typedef DataNotifyList::iterator IDataNotifyList;
      typedef DataNotifyList::const_iterator CIDataNotifyList; 
      
      typedef std::map<std::string, kvservice::priv::DataSubscriber*> DataList;
      typedef DataList::iterator IDataList;
      typedef DataList::const_iterator CIDataList;
      
      typedef std::map<std::string, kvservice::priv::HintSubscriber*>  HintList;
      typedef HintList::iterator        IHintList;
      typedef HintList::const_iterator CIHintList;
      
      DataNotifyList dataNotifySubs; 
      DataList       dataSubs; 
      HintList       hintSubs;

      bool connectToKvInput( bool reConnect );

      bool addDataNotifySubscriber(kvservice::priv::DataNotifySubscriber *ptr, 
				   const std::string &subid);
      
      bool addDataSubscriber(kvservice::priv::DataSubscriber *ptr, 
			     const std::string &subid);

      bool addHintSubscriber(kvservice::priv::HintSubscriber *ptr,
			       const std::string &subid);

    private:
      template<class SubList> bool unsubscribe( const std::string &id, SubList &subList );
      template<class SubList> void unsubscribeAll( SubList &subList );
      void unsubscribe_(const std::string &id);

    public:

      /**
       * \brief This is the constructor that allways must be used.
       *
       * \param argc from main.
       * \param argv from main.
       * \parm conf, a pointer to the configuration information.
       * \param options can be used to set additionals CORBA option. This
       *                is rarely used. Read the documentation for omniORB
       *                if you intend to use it.
       */
      CorbaKvApp( int &argc, char **argv,
		  miutil::conf::ConfSection *conf,
		  const char *options[][2] = NULL );

      virtual ~CorbaKvApp();

      /**
       * \brief read a configuration file.
       *
       * \param The configuration file to read.
       * \return The configuration.
       */
      static miutil::conf::ConfSection* readConf(const std::string &fname);

      std::string kvpathInCorbaNameserver() const;

      virtual bool getKvData( KvGetDataReceiver &dataReceiver, const WhichDataHelper &wd );
      virtual bool getKvRejectDecode( const CKvalObs::CService::RejectDecodeInfo &decodeInfo, kvservice::RejectDecodeIterator &it );
      virtual bool getKvParams( std::list<kvalobs::kvParam> &paramList );
      virtual bool getKvStations( std::list<kvalobs::kvStation> &stationList );
      virtual bool getKvModelData( std::list<kvalobs::kvModelData> &dataList, const WhichDataHelper &wd );
      virtual bool getKvReferenceStations( int stationid, int paramid, std::list<kvalobs::kvReferenceStation> &refList );
      virtual bool getKvTypes( std::list<kvalobs::kvTypes> &typeList );
      virtual bool getKvOperator( std::list<kvalobs::kvOperator> &operatorList );
      virtual bool getKvStationParam( std::list<kvalobs::kvStationParam> &stParam, int stationid, int paramid = -1, int day = -1 );
      virtual bool getKvStationMetaData( std::list<kvalobs::kvStationMetadata> &stMeta,
    		                             int stationid, const miutil::miTime &obstime,
    		                             const std::string & metadataName = "");
      virtual bool getKvObsPgm( std::list<kvalobs::kvObsPgm> &obsPgm, const std::list<long> &stationList, bool aUnion );
      virtual bool getKvData( KvObsDataList &dataList, const WhichDataHelper &wd );
      virtual bool getKvWorkstatistik(CKvalObs::CService::WorkstatistikTimeType timeType,
                                      const miutil::miTime &from, const miutil::miTime &to,
                                      KvWorkstatistikReceiver &receiver
                                      );


      virtual const CKvalObs::CDataSource::Result_var sendDataToKv( const char *data, const char *obsType );

      virtual SubscriberID subscribeDataNotify( const KvDataSubscribeInfoHelper &info, dnmi::thread::CommandQue &que );
      virtual SubscriberID subscribeData( const KvDataSubscribeInfoHelper &info, dnmi::thread::CommandQue &que );
      virtual SubscriberID subscribeKvHint( dnmi::thread::CommandQue &que );  
      virtual void unsubscribe( const SubscriberID &subscriberid );
      virtual void unsubscribeAll();

      CKvalObs::CService::kvService_ptr lookUpManager( bool forceNS, bool & usedNS );

      virtual bool shutdown() const;
      virtual void doShutdown();
      virtual void run();
    };
    /** @} */
  }
  /** @} */
}

#endif // __kvservice__corba__CorbaKvApp_h__
