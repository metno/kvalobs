/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: KvAppSimple.h,v 1.4.2.2 2007/09/27 09:02:44 paule Exp $                                                       

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
#ifndef __KvAppSimple_h__
#define __KvAppSimple_h__

#include <list>
#include "KvApp.h"
#include "kveventinterface.h"


namespace kvservice{

  /**
   * \addtogroup kvcpp
   * @{
   */
  class Timer;

  /**
   * \brief A simplified version if KvApp.
   */
  class KvAppSimple : public KvEventInterface
  {
    KvAppSimple(const KvAppSimple &);
    KvAppSimple& operator=(const KvAppSimple &);
    KvAppSimple();

    friend class Timer;

    class TimerWrap{
      Timer          *timer_;
      miutil::miTime expire_;
      int            delay_;
      long           timerid_;

      friend class KvAppSimple;
      friend class Timer;
    public:
      TimerWrap(Timer *t, const miutil::miTime &e, int d, long id);
      ~TimerWrap();

      bool periodic()const{ return delay_>=0;}
      void add(Timer *timer, const miutil::miTime &time);
      int delay()const{ return delay_;}
      miutil::miTime expire()const{ return expire_;}
      void           expire(const miutil::miTime &time);
      long timerid()const{ return timerid_;}
      Timer* timer(){ return timer_;}
      void   timer(Timer *t){ timer_=t;}
    };

    KvApp   app;
    std::list<TimerWrap*> timerList;
    long    nextTimerid;
    dnmi::thread::CommandQue que;
    bool   shutdown_;
    
    void insertTimerWrap(TimerWrap *wrap);

  protected:
    /**
     * \brief Subscribe to DataNotify events. 
     * 
     * The events is posted to the method
     * onKvDataNotify.
     *
     * \param info Which stations are we intrested in.
     * \return subscriberid on success and a empty string on failure.
     */  
    std::string subscribeDataNotify(const KvDataSubscribeInfoHelper &info);
    
    /**
     * \brief Subscribe to Data events. 
     *
     * The events is posted to the method
     * onKvData 
     *
     * \param info Which stations are we intrested in.
     * \return subscriberid on success and a empty string on failure.
     */  
    std::string subscribeData(const KvDataSubscribeInfoHelper &info);
    
    /**
     * \brief Subscribe to Hint events. 
     * The events is posted to the method onKvHint.
     *
     * \return subscriberid on success and a empty string on failure.
     */  
    std::string subscribeKvHint();
    
    /**
     * \brief Tell the kvManager that we are no longer interessted in 
     * notifications.
     */
    void unsubscribe(const std::string &subscriberid);


    
  public:
    KvAppSimple(int argn, char **argv,  miutil::conf::ConfSection *conf);
    ~KvAppSimple();

    static KvAppSimple *kvAppSimple;

    void   doShutdown();
    bool   shutdown();

    static miutil::conf::ConfSection* readConf(const std::string &fname);

    /**
     * \brief Return the poa (CORBA). 
     * 
     * This is a nil reference if the aplication is in shutdown.
     */
    PortableServer::POA_ptr        getPoa()const;
    
    /**
     * Return a reference to the Orb (CORBA).
     *
     * This is a nil reference if the aplication is in shutdown.
     */
    CORBA::ORB_ptr                 getOrb()const;
    
    /**
     * \brief Return a reference to the PoaManager (CORBA).
     *
     * This is a nil reference if the aplication is in shutdown.
     */
    PortableServer::POAManager_ptr getPoaMgr()const;
    
    /**
     * \brief Put the CORBA reference, objref, into the CORBA nameserver. 
     *
     * The reference will be known under tha name,
     * name.
     *
     * The name is in the form /path/name. 
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
     * \brief Look up the name in the CORBA nameserver 
     *
     * and return a reference to the CORBA object if it
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
     * \brief Return a stringified reference for the Object, ptr.
     *
     * \param ptr, the Object we want a stringified reference to.
     * \return a stringified refererence to ptr on success, and a 
     *         empty string on failure.
     */
    std::string corbaRef(CORBA::Object_ptr ptr);
    
    /**
     * \brief Returns a CORBA reference for the stringified
     * reference given with ref.
     *
     * \param a stringified CORBA reference.
     * \return A CORBA Object on success, and a nil reference on failure.
     */
    CORBA::Object_ptr corbaRef(const std::string &ref);
    
    
    /**
     * corbanameserver, return the nameserver that is used to lookup
     * CORBA object. The nameserver is given in the configuration file.
     *
     * \return a string that represendt the nameserver. The string has
     *        the format speciefied in the specification for corbalocation.
     *        An empty strin is returnd if no nameserver is given.
     */
    std::string  corbanameserver()const; 
    
    /**
     * \brief Returns the path in CORBA nameserver
     * we shall lookup kvalobs in. 
     *
     * The path is given in the configuration file.
     *
     * \return the path of 'kvalobs' we shal use for name lookup of
     *         kvalobs objects. An empty string on failure.
     */
    std::string  kvpathInCorbaNameserver()const;

    /**
     * \brief Get data from kvalobs. 
     *
     * dataReceiver is called for every dataset from kvalobs. dataReceiver 
     * is called in the context of the CORBA thread. 
     * Use WhichDataHelper to speciefy which stations you want the data from.
     * If you only specify the stations id 0 you will get data from all stations
     * that is registred in kvalobs.
     *
     * KvGetDataReceiver is an abstract class.
     *
     * \param dataReceiver a instance of KvGetDataReiceiver.
     * \param wd use wd to specify which stations you want data from.
     * \return true on success false otherwise.
     */
    bool getKvData(KvGetDataReceiver &dataReceiver,
		   const WhichDataHelper &wd);      


    bool getKvParams(std::list<kvalobs::kvParam> &paramList);
    bool getKvStations(std::list<kvalobs::kvStation> &stationList);
    bool getKvModelData(std::list<kvalobs::kvModelData> &dataList,
			const WhichDataHelper &wd);
    bool getKvReferenceStations(int stationid, 
				int paramid, 
				std::list<kvalobs::kvReferenceStation> &refList); 
    bool getKvTypes(std::list<kvalobs::kvTypes> &typeList);
    bool getKvOperator(std::list<kvalobs::kvOperator> &operatorList);
    
    bool getKvObsPgm(std::list<kvalobs::kvObsPgm> &obsPgm,
		     const std::list<long> &stationList,
		     bool aUnion);
    


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
    


    /**
     * \brief onKvHintEvent, overide this to process KvHintEvent.
     */
    virtual void onKvHintEvent(bool up);

    /**
     * \brief onKvDataNotifyEvent, overide this to process KvDataNotifyEvent.
     */
    virtual void onKvDataNotifyEvent(KvWhatListPtr what);
     
    /**
     * \brief onKvDataEvent, overide this to process KvDataEvent.
     */
    virtual void onKvDataEvent(KvObsDataListPtr data);

    /**
     * \brief onStartup, is a hook to insert code that shall be run before
     * the event loop. 
     *
     * If the function return false the event loop is not 
     * entred and run returns.
     */
    virtual bool onStartup();
    virtual void onShutdown();

    /**
     * \brief addTimer, add a timer that shall be called every periodInSec.
     * 
     * The Timer exec function is called every periodInSec seconds. If 
     * exec return false the timer function is removed.
     *
     * KvAppSimple is taking over the ownership of the pointer to the timer 
     * object and will delete the object when it is not needed anymore. 
     * 
     * \param timer a pointer to a instance of Timer.
     * \param periodInSec is delay to wait for the next call.
     * \return timerid The timerid can be used to remove the timer
     *                 if it is not needed anymore.
     */
    long addTimer(Timer *timer, int periodInSec);
    
    /**
     * \brief addTimeJob, adds a job to be run at time. 
     * 
     * The Timer.exec is called at the specified time.
     *
     * KvAppSimple is taking over the ownership of the pointer to the timer 
     * object and will delete the object when it is not needed anymore. 
     * 
     * \param timer a pointer to a instance of Timer.
     * \param periodInSec is delay to wait for the next call.
     * \return timerid The timerid can be used to remove the timer
     *                 if it is not needed anymore.
     */
    long addTimeJob(Timer *timer, const miutil::miTime &time);
    void removeTimer(long timerid);
    

    /**
     * \brief run starts the event loop. The functions will not end until
     * the doShutdown is called or the signals SIGTERM or SIGQUIT is received.
     *
     * The first function that is called before the event loop is started is
     * onStartup and the last function that is called before the the function
     * return is onShutdown. So the run function is in prinsip implemeted as.
     *
     * \code 
           run() 
           {
                onStartup();
     
                while(!inShutdown){
                    - do event processing 
                }
     
                onShutdown();
            }
     \endcode
     */

    void run();

  };

  /** @} */
}



#endif
