/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: App.h,v 1.8.6.5 2007/09/27 09:02:37 paule Exp $                                                       

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
#ifndef __norcom2kv_app_h__
#define __norcom2kv_app_h__

#include <string>
#include <map>
#include <kvskel/datasource.hh>
#include <kvalobs/kvapp.h>
#include "FInfo.h"
#include "kvDataSrcList.h"

class App : public KvApp
{
  CKvalObs::CDataSource::Data_var refData;
  std::string synopdir_;
  std::string tmpdir_;
  std::string data2kvdir_;
  bool        test_;
  std::string logdir_;
  TKvDataSrcList refDataList;
  bool          debug_;

  void initLogger(const std::string &ll, const std::string &tl);

 public:
  App(int argn, char **argv, const char *options[0][2]=0);
  virtual ~App();

  CKvalObs::CDataSource::Data_ptr 
    lookUpKvData(bool forceNS, 
		 bool &usedNS,
		 const std::string &kvpath_);


   CKvalObs::CDataSource::Result 
     *sendDataToKvalobs(const std::string &message, 
			const std::string &obsType,
			std::string &sendtTo);

   bool saveFInfoList(const std::string &name, const FInfoList &infoList);
   bool readFInfoList(const std::string &name, FInfoList &infoList);

   bool inShutdown()const;
   
   void doShutdown();


	bool debug()const{ return debug_; }

   /**
    * \brief Check if  \a dir is a directory! Exit if fail!!!
    *
    * If \a rwaccess is false we need only read access to dir.
    * If \a rwaccess is true we need read/write access to the 
    * directory dir.
    *
    * \param dir The path to check.
    * \param rwaccess Must have write access.
    * \return dir with a '/' appended if neccessary.
    */
   std::string checkdir(const std::string &dir, bool rwaccess=false);


   /**
    * getKvalobsServer, return the name of the main kvalobs server.
    * The main kvalobs server is the name of the server in CORBA nameserver
    * that is the production server.
    */
   std::string getKvalobsServer()const;
   

   /**
    * remove the kvpath() from the beginning of the path.
    *
    * If \a kvalobs is true replace kvpath() with the string $KVALOBS.
    *
    * This is a fine function to use to get shorter path messages in 
    * logfiles.
    */
   std::string relpath(const std::string &path, bool kvalobs=true);
   std::string kvdir()const { return getKvalobsPath();}
   std::string logdir()const { return logdir_;}
   bool        test()const{ return test_;}
   std::string tmpdir()const {return tmpdir_;}
   std::string data2kvdir()const {return data2kvdir_;}
   std::string synopdir()const{ return synopdir_;}
};


#endif
