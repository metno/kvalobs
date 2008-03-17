/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvapp.h,v 1.1.2.2 2007/09/27 09:02:30 paule Exp $                                                       

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
#ifndef __kvapp_h__
#define __kvapp_h__

#include <exception>
#include <corbahelper/corbaApp.h>


namespace miutil{
  namespace conf{
      class ConfSection;
  }
}

/**
 * \addtogroup kvinternalhelpers
 * @{
 */

//For tenkeboksen. 
class LookUpException : public std::exception{
  std::string reason;
 public:
  LookUpException(const std::string &reason_):reason(reason_){
  }
  ~LookUpException()throw(){}
  
  const char *what()const throw(){ return reason.c_str();}
};
//


/**
 * \brief This class encapsulate functions that is common for 
 * most applications in kvalobs. 
 */
class KvApp : public CorbaHelper::CorbaApp
{
  std::string pidfile;
  static miutil::conf::ConfSection *conf;
  static std::string confFile;
  
  /**
   * kvPathInCorbaNS, this variable holds the path in CORBA
   * nameserver that shall be used. This path can be given
   * either with the environment variable KVPATH_IN_NS or on the
   * command line with the option --cnspath. Default value is
   * 'kvalobs'. The --cnspath override all other values.
   */
  std::string kvPathInCorbaNS;  

 public:
  KvApp(int argn, char **argv, const char *options[0][2]=0);
  virtual ~KvApp();

  ///Inherited from CorbaApp.
  virtual bool isOk()const;

	/**
	 * \brief Returns the kvalobs server that is configured.
	 * This is the path in the CORBA nameserver where we look 
	 * up the server.
	 */
	std::string kvserver()const { 
		std::string s= kvPathInCorbaNS;
		   
		if(!s.empty() && s[s.length()-1]=='/')
			s.erase(s.length()-1);
			
		return s; 
	}

  virtual void useMessage(std::ostream &os);
  void printUseMsgAndExit(int exitStatus);
  
  /**
   * \brief put the objref into the CORBA nameserver. 
   *
   * The kvPathInCorbaNS is prepended to the name. With this 
   * we can have several kvalobs servers running at the same time. 
   * Default value is /kvalobs.
   *
   * \param objref A CORBA object referances.
   * \param name The name the objref should be known as.
   * \return true on success and false otherwise.
   */
  bool putRefInNS(CORBA::Object_ptr objref, 
		  const std::string &name);

  /**
   * \brief looks up a name in the CORBA nameserver. 
   * 
   * The kvPathInCorbaNS is prepended to the name. With this we 
   * can have several kvalobs servers running at the same time. 
   * Default value is /kvalobs.
   *
   * \param name The name the CORBA object referance we want is known as.
   * \return A CORBA object referance. This could be a NULL referance.
   */
  CORBA::Object_ptr getRefInNS(const std::string &name);

  /**
   * \brief looks up a name in the CORBA nameserver.
   * 
   * Look up the name in the path given with \em path.
   * \param name The name the CORBA object referance we want is known as.
   * \param path look for the name in this path in the CORBA nameserver.
   * \return A CORBA object referance. This could be a NULL referance.
   */
  CORBA::Object_ptr getRefInNS(const std::string &name, const std::string &path);

  /**
   *  \brief get a pointer to the configuration data.
   *  
   * \note  WARNING: dont delete the returned pointer.
   */
  static miutil::conf::ConfSection* getConfiguration();
  
  /**
   * \brief get the name of the configuration file that is used.
   *
   * The configuration file must be in $KVALOBS/etc path. Where 
   * KVALOBS is a environment variable that must bes set.
   */
  static std::string getConfFile(const std::string &ifNotSetReturn="kvalobs.conf");

  /**
   * \brief set the name of the configuration file.
   *
   * \note IMPORTENT: this function must be called before an instance of
   *       this class is created.
   * \verbatim
     Example
     
     class MyApp : public KvApp{
      //Your stuff.
     } 
     
     int
     main(int argn, char **argv){
       KvApp::setConfFile( my confile ); //MUST be before the instatiation of MyApp 
       MyApp app(); 
     } 
     \endverbatim
   */
  static void        setConfFile(const std::string &filename);
  void createPidFile(const std::string &progname);
  void deletePidFile();

  /* I tenkeboksen
  template<class T>
  typename T::_ptr_type lookUpObject(bool forceNS, 
                            bool &usedNS);
  */

  /**
   * \brief creates a string that can be used to connect to the database. 
   *
   * The function use the configuration file
   * $KVALOBS/etc/kvalobs.conf if it exist or it may get the different 
   * values from the environment. It will always look at the configuration
   * file first.
   *
   * The environment variables that is used is:
   *
   *    KVDBUSER, the user we shall connect to the databse as. 
   *              Default 'kvalobs'.
   *    KVDB,     the database to connect to. Default 'kvalobs'.
   *    PGHOST,   where is the databse. Default 'EMPTY STRING' this
   *              mean that we shall use the databse default.
   *    PGPORT,   at which port shall we connect to the database. Default
   *              'EMPTY STRING', this mean use the databse default.
   *
   * \param dbname, overides the above schema.
   * \param kvdbuser, overides the above schema.
   * \param host, overides the above schema.
   * \param port, overides the above schema.
   * \return the connection string.
   */

  static std::string createConnectString(const std::string &dbname="",
					 const std::string &kvdbuser="",
					 const std::string &host="",
					 const std::string &port="");

};

/** @} */

#endif
