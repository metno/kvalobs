/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: decoder.h,v 1.1.2.2 2007/09/27 09:02:27 paule Exp $                                                       

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
#ifndef __dnmi_decoder_Decoder_h__
#define __dnmi_decoder_Decoder_h__

#include <list>
#include <set>
#include <sstream>
#include <string>
#include <miconfparser/miconfparser.h>
#include <puTools/miTime.h>
#include <kvdb/kvdb.h>
#include <fileutil/dso.h>
#include <kvalobs/kvObsPgm.h>
#include <kvalobs/paramlist.h>
#include <kvalobs/kvData.h>
#include <kvalobs/kvStationInfo.h>
#include <kvalobs/kvRejectdecode.h>
#include <kvalobs/kvStation.h>
#include <kvalobs/kvTextData.h>
#include <kvalobs/kvTypes.h>
#include <milog/milog.h>
#include <decoderbase/GenCacheElem.h>
#include <decoderbase/metadata.h>



namespace kvalobs{



/**
   * \brief The namespace the decoder interface is in.
   */

  namespace decoder{
    /**
     * \defgroup kvdecoder Data decoders
     * 
     * The data decoders is implemented as plugins to the kvDatainputd.

     * @{
     */
    
    /**
     * \brief The base class for all data decoders in kvalobs.
     */


    class ConfParser;



    typedef enum { YES, NO, MAYBE } Active; 
    
    class ObsPgmParamInfo {
    public:
       typedef std::list<kvalobs::kvObsPgm>                   ObsPgmList;
       typedef std::list<kvalobs::kvObsPgm>::iterator        IObsPgmList;
       typedef std::list<kvalobs::kvObsPgm>::const_iterator CIObsPgmList;
            
       ObsPgmList     obsPgm;
       miutil::miTime obstime;
       
       
       ObsPgmParamInfo();
       ObsPgmParamInfo( const ObsPgmParamInfo &cs );
       
       ObsPgmParamInfo& operator=( const ObsPgmParamInfo & );
       
       bool isActive( int stationid, 
                      int typeid_,
                      int paramid,
                      int sensor,
                      int level,
                      const miutil::miTime &obstime,
                      Active &state ) const;
    };
    
    class DecoderBase{
      DecoderBase();
      DecoderBase(const DecoderBase &);
      DecoderBase& operator=(const DecoderBase &);

      int decoderId;
      kvalobs::kvStationInfoList stationInfoList;
      std::list<std::string>     createdLoggers;
      std::list<GenCachElem>          genCachElem;
      miutil::conf::ConfSection *theKvConf;
    
    protected:
      milog::FLogStream *openFLogStream(const std::string &filename);
      
      void addStationInfo(long stationid,
			  const miutil::miTime &obstime,
			  long typeid_,
			  const miutil::miTime &tbtime,
			  int priority);

      void updateStationInfo( const kvalobs::kvStationInfoList& stationInfo );


      /**
       * \brief  isGenerated looks up in the table 'generated_data' 
       * 
       * to see if there exist an entry for the stationid/typeid
       * combination. If there is an entry the function returns 
       * true and false otherwise.
       *
       * The generated_data table has an enty for each message 
       * given by stationid and typeid that is not 'original' data, 
       * but a message generated from data from the kvalobs. This my be
       * SYNOP, METAR etc.
       *
       * @param stationid, the stationid for the message.
       * @param typeid, the typeid for the message.
       * @return true if this is a message for generated data and
       *         false if this is original data.
       * @throws dnmi::db::SQLException if there was a database error.
       */
      bool isGenerated(long stationid, long typeid_);

      dnmi::db::Connection &con;
      ParamList            paramList;
      const std::list<kvalobs::kvTypes> &typeList;


      /**
       * mataconf contains metadata that is given from the 
       * data providers. This is extra information about the data
       * that is not avaible in kvalobs, but is needed to decode the data.
       */
      miutil::parsers::meta::Meta       metaconf;



      /**
       * \brief This is the type (format) of the observation message that 
       * is to be decoded. 
       * 
       * It is an input parameter to the constructor. 
       */
      std::string  obsType;
      
      /**
       * \brief This is the message that is to be decoded 
       *
       * and inserted into the tables \em data and \em text_data. 
       * It is an input parameter to the constructor. 
       */
      std::string     obs;

      std::string logdir_; //Set the path to use for the logdir. Mainly for running tests.

      /**
       * \brief findTypes, look up the typeid in the typelist 
       * 
       * and returns a pointer to a kvTypes. This pointer must NOT
       * be deleted.
       *
       * \param typeid_ the type to look up.
       * \return A pointer to a kvTypes on success and 0 otherwise.
       */
      const kvalobs::kvTypes* findType(int typeid_)const;

      /**
       * \brief getStationId, looks up in the database to find the kvalobs
       * stationId from ex. climano, synopno, etc 
       *
       * \param key must be a valid column name in the table kv_stasjon.
       * \param value the value of key that we want.
       * \return >=0  when successful, this is the kvalobsnumber.
       *          =-1 when the key=value is not found.
       *          <-1 when the key is invalid.
       */
      long  getStationId(const std::string &key,
			 const std::string &value);


      /**
       * \brief deleteKvDataFromDb, removes the kvData from the databes if it exists.
       *
       * \return true if there was now error, false otherwise.
       */
      bool  deleteKvDataFromDb(const kvalobs::kvData &sd);
      
      /**
       * \brief insert/update data into the table \em data.
       *
       * It calls addStationInfo, so the caller must NOT do this.
       * The field tbtime will be updated if it is not set.
       *
       * \param sd The data.
       * \param priority The priority of the \em obs. Defaul value is 5.
       * \return true if successful false otherwise.
       */
      bool putKvDataInDb(const kvalobs::kvData &sd, int priority=5);


      /**
       * \brief insert/update data into the table \em data.
       *
       * It calls addStationInfo, so the caller must NOT do this.
       * The field tbtime will be updated if it is not set.
       *
       * \param sd A list of data.
       * \param priority The priority of the \em obs. Defaul value is 5.
       * \return true if successful false otherwise.
       */
      bool putKvDataInDb(const std::list<kvalobs::kvData> &sd, int priority=5);


      /**
       * \brief Inserts data and textdata into the database.
       *
       * Call the  addDataToDb( const miutil::miTime &obstime, int stationid, int typeid_,
       *                 std::list<kvalobs::kvData> &sd,
       *                 std::list<kvalobs::kvTextData> &textData,
       *                 int priority, const std::string &logid,
       *                 bool onlyAddOrUpdateData )
       * with onlyAddOrUpdateData set to false. That it exist two variant
       * is for backward binary comaptibility.
       *
       * \param obstime The observation time.
       * \param stationid The stationid.
       * \param typeid The typeid.
       * \param sd data to insert.
       * \param textData to insert.
       * \param priority The priority of the \em obs. Default value is 5.
       * \param logid Send all log to this logger.
       * \return true if successful false otherwise.
       * \see bool addDataToDb( const miutil::miTime &obstime, int stationid, int typeid_,
       *                 std::list<kvalobs::kvData> &sd,
       *                 std::list<kvalobs::kvTextData> &textData,
       *                 int priority, const std::string &logid,
       *                 bool onlyAddOrUpdateData )
       */
      bool addDataToDb( const miutil::miTime &obstime, int stationid, int typeid_,
                        std::list<kvalobs::kvData> &sd,
                        std::list<kvalobs::kvTextData> &textData,
                        int priority, const std::string &logid);


      /**
       * \brief Inserts data and textdata into the database.
       *
       * It calls addStationInfo, so the caller must NOT do this.
       * The field tbtime will be updated if it is not set.
       *
       * Before data is inserted into the database we checks if the
       * data already exist and is equal to the data we are trying to insert.
       * If it does, the method just returns true without actually inserting
       * the data.
       *
       * How the data is inserted depends on the value of paramter onlyAddOrUpdateData.
       *
       * onlyAddOrUpdateDate is false:
       * If there exist data for the dataset but the data is not equal all
       * data is marked as deleted before the new dataset is inserted.
       *
       * onlyAddOrUpdateDate is true:
       * If data all ready exist for a parameter, but the original value differs
       * in new and old data the data is updated. Data that do not exist is inserted.
       *
       * \param obstime The observation time.
       * \param stationid The stationid.
       * \param typeid The typeid.
       * \param sd data to insert.
       * \param textData to insert.
       * \param priority The priority of the \em obs. Default value is 5.
       * \param logid Send all log to this logger.
       * \param onlyAddOrUpdateDate if true only update or insert data.
       * \return true if successful false otherwise.
       */
      bool addDataToDb( const miutil::miTime &obstime, int stationid, int typeid_,
                        std::list<kvalobs::kvData> &sd,
                        std::list<kvalobs::kvTextData> &textData,
                        int priority, const std::string &logid,
                        bool onlyAddOrUpdateData );

      /**
       * \brief insert the \em obs message into the table \em rejectdecode. 
       *
       */

      bool putRejectdecodeInDb(const kvalobs::kvRejectdecode &rd);
      
       
      /**
       *  \brief putkvStationInDb, if a SHIP is not defined in
       *
       *  the database, then we load it here (temporary) 
       */
      
      bool putkvStationInDb(const kvalobs::kvStation &st);

      /**
       * \brief Insert/update the data into the table \em text_data..
       *
       * It calls addStationInfo, so the caller must NOT do this.
       * The field tbtime will be updated if it is not set.
       *
       * \param sd The data.
       * \param priority The priority of the \em obs. Defaul value is 5.
       * \return true if successful false otherwise.
       */
      bool putkvTextDataInDb(const kvalobs::kvTextData &td,int priority=5);


      bool putkvTextDataInDb(const std::list<kvalobs::kvTextData> &td_, 
			     int priority);



    public:

      typedef enum{
	///The \em obs was decoded and inserted into the database. 
	  Ok,
	    ///The \em obs was decoded, but not inserted into the database due to a database error.. 
	    NotSaved, 
	    ///There was an error while deoding the \em obs. The obs is inserted into the table \em rejectdecode.
	    Rejected, 
	    ///There was an unrecoverable error in the decoder. Not releated
	    ///to the \em obs.
	    Error} DecodeResult;

      /**
       * \brief This is the only avalable constructor, all classes that 
       * inherit from this baseclass must call this constructor. 
       *
       * The observation message to be decoded is \em obs_ and the type
       * (format) of the message is \em obsType_. You can also give
       * extra information in form of a meta parameter.
       *
       * The metadata is xml encoded on the form.
       *
       * <pre>
       *
       *    <?xml version="1.0" ?>
       *    <meta>
       *       <params>
       *          <param name="paramname">
       *              <value>value</value>
       *              <value>value</value>
       *          </param>
       *          <param name="paramname2">
       *              <value>value</value>
       *          </param>
       *       </params>
       *    </meta>
       *
       * </pre>
       *
       * Note that it is posible to give an array of values with repeated
       * value tags in an param section.
       *
       * The decoder will save the decoded data in the tables \em data and
       * \em text_data in the kvalobs database. 
       *
       * If the message cant be decoded the hole message \em obs_ is inserted
       * into the table \em rejectdecode.
       *
       * \note Failing to call this constructor will result in a real mess, 
       * and proably a core dump when the driver is unloaded. You are warned!
       *
       * \param con_ A database connection to use.
       * \param params A list of parameters defined in table \em param.
       * \param typeList A list of types defined in table \em types.
       * \param obsType The type (format) of the observation \em obs_.
       * \param obs_  The observation message to be decoded.
       * \param decoderId_ The decoderid the decoder is identified with.
       * \param metadata xml coded metadata.
       */
      DecoderBase(dnmi::db::Connection   &con_,
		  const ParamList        &params,
		  const std::list<kvalobs::kvTypes> &typeList,
		  const std::string &obsType_,
		  const std::string &obs_,
		  int               decoderId_=-1 );

      virtual ~DecoderBase();
      
      /**
       * Set the configuration of the <kvalobs.conf> that is in effect.
       */
      void setKvConf( miutil::conf::ConfSection *theKvConf );

      /**
       * Compute the useinfo flag 7 (to late or to early observation).
       * It use data from the obspgm.
       *
       * \param typeid The typeid to the observation.
       * \param nowTime The wall clock time that we use as the base in the check.
       * \param obstime The time the observation was taken.
       * \param logid Which logger should log messages be written to.
       */
      char getUseinfo7Code( int typeId,
              	  	  	  	const boost::posix_time::ptime &nowTime,
              	  	  	  	const boost::posix_time::ptime &obstime,
              	  	  	  	const std::string &logid="" );

      void setMetaconf(const std::string &metaconf);

      
      /**
       * \brief Return the value of a key in obstype.
       * 
       * The obstype string that identifies a message can contain
       * key=value pairs.
       * 
       * The generell format on the obstype string is:
       * 
       *   messagetype/key0=val0/key1=val1/.../keyN=valN
       * 
       * Where the keys may be specific for the messagetype.
       * 
       * \param key The key we want the value to.
       * \return the value to the key if the key exist or an
       *         empty string otherwise.
       */
      std::string getObsTypeKey( const std::string &key ) const;
      
      
      /**
       * \brief Returns the meta_SaSd key from ObsType.
       * 
       * The values indicates if the station report SA/SD/EM. 
       * The returned string is two character wide. Where each
       * value represent a boolean, 0 is false and 1 true.
       * 
       * The first character indicate is the value for SA and 
       * the second caharacter is the value of SD.
       *  
       *  -SA=1 The station report snowdepth.
       *  -SD/EM = 1 The station report the state of the ground. 
       * 
       * The values comes typically from the E'sss codings. See 
       * the WMO specificatipn for SYNOP.
       * 
       * \return The value of the meta_SaSd key from the ObsType 
       *         if present otherwise an empty string is returned.
       */
      std::string getMetaSaSd()const;
      
      /**
       * \brief return the decoder id.
       * \return The decoder id.
       */
      int getDecoderId()const;
      
      /**
       * \brief return a database connection.
       *
       * The database connection is created by the caller before
       * the instatiaiton of the decoder.
       *
       * You must never creat a connection in the decoder.
       *
       * \return A database connection.
       */
      dnmi::db::Connection *getConnection(){ return &con;}
      
      /**
       * \brief return a referance to the station list. 
       * Ths is a list of the stations that is defined in the table
       * \em stations in the kvalobs database, 
       *
       * \return The station list. 
       */
      kvalobs::kvStationInfoList &getStationList(){ return stationInfoList;}
      
      /**
       * \brief is the \em paramname a textparam.
       * The parameters that is defined as text params is inserted into
       * the table \em text_data in the kvalobs database.
       *
       * \param paramname The parameter to test.
       * \return true if \em paramname is a text param and false if not.
       */
       bool isTextParam(const std::string &paramname);

      /**
       * \brief is the \em paramid a textparam.
       * The parameters that is defined as text params is inserted into
       * the table \em text_data in the kvalobs database.
       *
       * \param paramid The parameter to test.
       * \return true if \em paramid is a text param and false if not.
       */
      static bool isTextParam(int paramid);

      /**
       * \brief the name of the decoder.
       *
       * This is an virtual function that must be implemented by the
       * decoder.
       *
       * \note The name of the decoder must be unique. ie. It cant be two 
       * decoders in the system with the same name.
       *
       * \return The name of the decoder.
       */
      virtual std::string name()const =0;

      /**
       * \brief call this funtction to decode a message.
       * This is a virtual function that must be implemented by 
       * the decoder.
       *
       * The caller call this function to decode a message.
       *
       * The message to be decoded was given as the \em obs_ parameter  
       * and the the type of the message was given as the obsType_ parameter 
       * to the constructor:  
       *
       *  DecoderBase(dnmi::db::Connection   &con_,const ParamList        &params,const std::list<kvalobs::kvTypes> &typeList,const std::string &obsType_,const std::string &obs_,int                    decoderId_)
       *
       * The observation message and the observation type (format) is 
       * avalable to the decoder as the protected variables \em obs and 
       * \em  obsType.
       *
       * \param[out] msg a status message on return.
       * \return The decoding status after decoding the message.
       */
      virtual DecodeResult execute(std::string &msg)=0;

      /**
       * \brief Create a logger that logs to a file.
       *
       * The log message will be logged to the file:
       *  $KVALOBS/var/log/decoders/DecoderBase::name()/logname.log
       *
       * The newly created logger will have the default loglevel set to
       * milog::DEBUG.
       *
       * \param logname The name of the log file.
       */
      void createLogger(const std::string &logname);
      
      /**
       * \brief remove a logger previously cretated with createLogger.
       *
       * \param logname The logger to be removed.
       */
      void removeLogger(const std::string &logname);
      
      /**
       *\brief change the log level for a logger.
       *
       * The default loglevel to a newly created logger is milog::DEBUG.
       *
       * \param logname The logger we want to change the level for.
       * \param loglevel The new loglevel to the logger \em logname.
       */
      void loglevel(const std::string &logname, milog::LogLevel loglevel);

      
      /**
       * \brief Load a decoder specific configurationfile.
       * 
       * The file loaded is $KVALOBS/etc/decode/name()/'sid'-'tid'.conf
       *
       * \param sid stationid
       * \param tid typeid
       *
       * \return true on success and false otherwise.
       */
      bool loadConf(int sid, int tid, 
		    kvalobs::decoder::ConfParser &parser);

      /**
       * Return the configuration section for the specific driver
       * from the kvalobs.conf. The driver specific part is in
       * the section to kvDataInputd and has the same name as
       * returned by DecoderBase::name.
       * \return The ConfSection if one is defined or 0 if non is defined in
       * the configuration file.
       */
      miutil::conf::ConfSection *myConfSection();
      /**
       * \brief Load the observation program.
       */
      bool loadObsPgmParamInfo( int stationid, int typeid_, 
                                const miutil::miTime &obstime,
                                ObsPgmParamInfo &paramInfo
                                ) const;

      std::string logdir()const;
      void logdir( const std::string &logdir );
    };

    /**
     * Helper class to manage temporary id loggers
     * for a thread.
     *
     * On construction an logger is created. On destruction
     * the logger is removed.
     *
     * The filename of the logger is nN-tT.log.
     * Where N is the stationid ant T is the typeid.
     */
    class IdlogHelper {
       IdlogHelper();
       IdlogHelper( const IdlogHelper &);
       IdlogHelper operator=(const IdlogHelper &);
       kvalobs::decoder::DecoderBase *decoder;
       std::string logid_;

    public:
       IdlogHelper( int stationid, int typeid_,
                    kvalobs::decoder::DecoderBase *decoder_ )
       : decoder( decoder_ )
       {
          std::ostringstream ost;
          ost << "n" << stationid << "-t" << typeid_;
          logid_ = ost.str();

          if( decoder )
             decoder->createLogger( logid_ );
       }

       ~IdlogHelper() {
          if( decoder )
             decoder->removeLogger( logid_ );
       }

       std::string logid()const{ return logid_;}
    };
    
    /** @} */
  }
}
      
#endif


