#ifndef __signalnewfileapp_h__
#define __signalnewfileapp_h__

#include <exception>
#include "newfilePoa.hh"
#include "corbaApp.h"

class SignalNewFileApp : public CorbaHelper::CorbaApp
{
  micutil::newfilesignal_var refNewfile;
  std::string                aopath_;
  std::string                newfileApp;
  std::string                datadir;
  omni_mutex   mutex;

 public:

  class ConfigEx : public std::exception{
    std::string reason;
  public:
    explicit ConfigEx(const std::string &reason_)
      : reason(reason_){}
    
    virtual ~ConfigEx()throw(){};
    
    const char *what()const throw()
      { return reason.c_str();}
  };


  SignalNewFileApp(int argn, char **argv);

  /**
   * Use the CORBA nameserver to lookup name. The 'kvpath' i 
   * prepended to the name before the request i sendt to the 
   * nameserver. If kvpath is empty the function throws 
   * an ConfigEx exception or the path dont exist in the CORBA
   * nameserver.
   *
   * \param kvpath The path in the nameserver we shall lookup
   *               the 'name'.
   * \param name The name we are looking for.
   * \return A referance to the object when successfull and CORBA::_nil()
   *         on fail.
   * \exception ConfigEx.
   */
  CORBA::Object_ptr getRefInNS(const std::string &name,
			       const std::string &path="");

  /**
   * This function return either a cached referance to a 'Data'
   * Object or use the nameserver.
   *
   * \param forceNS, use only the nameserver.
   * \param usedNS, is set to true on return if the nameserver was used.
   * \param kvpath the path in the nameserver we shall lookup
   *               the KvDataSource.
   * \return A Data_ptr if successful, CORBA::_nil() on error.
   */
  micutil::newfilesignal_ptr 
    lookUpNewfilesignal(bool forceNS, 
			bool &usedNS);
  
  bool createFile(const std::string &station,
		  const std::string &data, 
		  const std::string &key);
  
  
};


#endif
