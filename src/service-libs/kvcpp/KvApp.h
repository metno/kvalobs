/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: KvApp.h,v 1.8.2.3 2007/09/27 09:02:44 paule Exp $                                                       

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
#ifndef __kvservice_KvApp_h__
#define __kvservice_KvApp_h__

#include <list>
#include <miconfparser/miconfparser.h>
#include <dnmithread/CommandQue.h>
#include "kvservicetypes.h"
#include "kvDataSubscribeInfoHelper.h"
#include <kvalobs/kvData.h>
#include <kvalobs/kvModelData.h>
#include <kvalobs/kvReferenceStation.h>
#include <kvalobs/kvParam.h>
#include <kvalobs/kvStation.h>
#include <kvalobs/kvStationParam.h>
#include <kvalobs/kvTypes.h>
#include <kvalobs/kvObsPgm.h>
#include <kvalobs/kvOperator.h>
#include "WhichDataHelper.h"
#include "kvevents.h"
#include "KvGetDataReceiver.h"

#include <kvskel/datasource.hh>
#include "RejectdecodeIterator.h"

namespace boost{
  class thread;
};

namespace kvservice{
  namespace priv{
    class KvCorbaThread;
  }
    
  /**
   * \addtogroup kvcpp
   * @{
   */
    
  /**
   * \brief A application singleton class for kvservice aplications.
   *
   * There is nothing that prevent you from creating more instanceses 
   * but the behavior is undefined. Don't do it.
   */
  class KvApp
  {
    priv::KvCorbaThread *corbaThread_;
    
    bool shutdown_;

  public:
      
      static KvApp *kvApp;
      
      /**
       * \brief This is the constructor that allways must be used.
       *
       * \param argn from main.
       * \param argv from main.
       * \parm conf, a pointer to the configuration information.
       * \param options can be used to set additionals CORBA option. This
       *                is rarely used. Read the documentation for omniORB
       *                if you intend to use it.
       */
      
      KvApp(int &argn, char **argv, 
	    miutil::conf::ConfSection *conf,
	    const char *options[][2]=0);
      
      virtual ~KvApp();
      
      /**
       * \brief read a configuration file.
       *
       * \param The configuration file to read.
       * \return The configuration.
       */
      static miutil::conf::ConfSection* readConf(const std::string &fname);

      /**
       * \brief the shutdown status.
       *
       * \return true if we are in a shutdown.
       */
      bool shutdown()const;
      
      /**
       * \brief request shutdown, ie we want to terminate.
       */
      void doShutdown();

      /**
       * \brief start the eventloop.
       * 
       * Must be called for every aplication that calls one of the subscriber
       * functions.
       *
       * It does'nt return before the application is ready to terminate,
       * ie doShutdown is called or SIGQUIT/SIGINT is received. 
       * 
       */
      void run();
      
      
      /**
       * \brief return the poa (CORBA). 
       * 
       * This is a nil reference if the aplication is in shutdown.
       */
      PortableServer::POA_ptr        getPoa()const;

      /**
       * \brief return a reference to the Orb (CORBA).
       *
       * This is a nil reference if the aplication is in shutdown.
       */
      CORBA::ORB_ptr                 getOrb()const;

      /**
       * \brief return a reference to the PoaManager (CORBA).
       *
       * This is a nil reference if the aplication is in shutdown.
       */
      PortableServer::POAManager_ptr getPoaMgr()const;

      /**
       * \brief put the CORBA reference, objref, into
       * the CORBA nameserver. 
       *
       * The reference will be known under tha name, \em name.
       *
       * The name is on the form \em /path/name. 
       *
       * The nameserve is given in the configuration file.
       * 
       * \param objref, a CORBA reference.
       * \param name,   the name objref shall be known under.
       *
       * \return true on success, false otherwise.
       */
      bool   putObjInNS(CORBA::Object_ptr objref, 
			const std::string &name);
  
      /**
       * getObjFromNS, will lookup the name in the CORBA 
       * nameserver and return a reference to the CORBA object if it
       * is registred in the nameserver.
       *
       * The name is in the form /path/name. 
       *
       * The nameserve is given in the configuration file.
       * 
       * \param name,   the name to lookup in the nameservice.
       *
       * \return A Object reference if it finds the name and a
       *         nil reference otherwise.
       */
      CORBA::Object_ptr getObjFromNS(const std::string &name);
      
      /**
       * \brief return a stringified reference for the Object, \em ptr.
       *
       * \param ptr, the Object we want a stringified reference to.
       * \return a stringified refererence to ptr on success, and a 
       *         empty string on failure.
       */
      std::string corbaRef(CORBA::Object_ptr ptr);

      /**
       * \brief returns a CORBA reference for the stringified
       * reference given with ref.
       *
       * \param a stringified CORBA reference.
       * \return A CORBA Object on success, and a nil reference on failure.
       */
      CORBA::Object_ptr corbaRef(const std::string &ref);


      /**
       * \brief return the nameserver that is used to lookup
       * CORBA object. 
       *
       * The nameserver is given in the configuration file.
       *
       * \return a string that represendt the nameserver. The string has
       *        the format speciefied in the specification for corbalocation.
       *        An empty strin is returnd if no nameserver is given.
       */
      std::string  corbanameserver()const; 

      /**
       * \brief returns the path in CORBA nameserver to be used to lookup 
       * kvalobs. 
       *
       * The path is given in the configuration file.
       *
       * \return the path of 'kvalobs' we shal use for name lookup of
       *         kvalobs objects. An empty string on failure.
       */
      std::string  kvpathInCorbaNameserver()const;
	
      

      /**
       * \brief Subscribe to DataNotify events. 
       *
       * The events is posted on the que as DataNotifyEvent. The 
       * DataNotifyEvent is declared in the file \em kvevents.h.
       *
       * \param que The que to receive DataNotifyEvent.
       * \return subscriberid on success and a empty string on failure.
       */  
      std::string subscribeDataNotify(const KvDataSubscribeInfoHelper &info,
			              dnmi::thread::CommandQue &que);

      /**
       * \brief Subscribe to Data events. 
       *
       * The events is posted on the que as DataEvent. The DataEvent 
       * is declared in the file \em kvevents.h.
       *
       * \param que The que to receive DataEvent.
       * \return subscriberid on success and a empty string on failure.
       */  
      std::string subscribeData(const KvDataSubscribeInfoHelper &info,
			              dnmi::thread::CommandQue &que);

      /**
       * \brief Subscribe to Hint events. 
       *
       * The events is posted on the que as HintEvent. The HintEvent is 
       * declared in the file \em kvevents.h.
       *
       * \param que The que to receive HintEvent.
       * \return subscriberid on success and a empty string on failure.
       */  
      std::string subscribeKvHint(dnmi::thread::CommandQue &que);
      
      /**
       * \brief Unsubscribe from a service.
       *
       * Tell the kvService that we are no longer interessted in 
       * notifications.
       */
      void unsubscribe(const std::string &subscriberid);


      /**
       * \brief get data from kvalobs. 
       *
       * \em dataReceiver is called for every dataset from kvalobs. 
       * \em dataReceiver is called in the context of the CORBA thread.
       *
       * Use WhichDataHelper to speciefy which stations you want the data from.
       * If you only specify the stations id 0 you will get data from all 
       * stations that is registred in kvalobs.
       *
       * KvGetDataReceiver is an abstract class.
       *
       * \param dataReceiver a instance of KvGetDataReiceiver.
       * \param wd use wd to specify which stations you want data from.
       * \return true on success false otherwise.
       */
      bool getKvData(KvGetDataReceiver &dataReceiver,
		     const WhichDataHelper &wd);      


      /**
       * \deprecated
       */
      bool getKvData(KvObsDataList &dataList,
		     const WhichDataHelper &wd);
      
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

    bool getKvRejectDecode( const CKvalObs::CService::RejectDecodeInfo &decodeInfo, 
			    kvservice::RejectDecodeIterator &it );

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
       * \see getKvData(KvGetDataReceiver &dataReceiver,const WhichDataHelper &wd)
       *
       */
      bool getKvModelData(std::list<kvalobs::kvModelData> &dataList,
			  const WhichDataHelper &wd);

      /**
       * \brief return the referance stastions for a given stationid.
       *
       * \param stationid The stationid for the station we want the referance station too.
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
       * \brief return the operators that has access to manipulate
       *        data in kvalobs through the HQC application.
       *
       * \param[out] operatorList The operators on return.
       * \return true ons success and false otherwise.
       */
      bool getKvOperator(std::list<kvalobs::kvOperator> &operatorList);
      
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
      bool getKvObsPgm(std::list<kvalobs::kvObsPgm> &obsPgm,
		       const std::list<long> &stationList,
		       bool aUnion);

  protected:
      CKvalObs::CDataSource::Data_ptr dataInput;
      bool connectToKvInput(bool reConnect = false);
  };
  
  /** @} */
}

#endif
