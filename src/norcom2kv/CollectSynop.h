/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: CollectSynop.h,v 1.8.6.3 2007/09/27 09:02:37 paule Exp $                                                       

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
#ifndef __CollectSynop_h__
#define __CollectSynop_h__


#include <string>
#include <map>
#include <list>
#include "App.h"
#include "FInfo.h"
#include "File.h"
#include "WMORaport.h"



typedef std::list<File>                   FileList;
typedef std::list<File>::iterator        IFileList;
typedef std::list<File>::const_iterator CIFileList;

class CollectSynop
{
    CollectSynop(CollectSynop&);
    CollectSynop& operator=(const CollectSynop&);
    CollectSynop();
    
    App                             &app;
    FInfoList                       fileInfoList;
    
    bool checkForNewObservations();
    void collectObservations();
    std::string getNewObsPart(const std::string &obs, 
			      IFInfoList &it);

    void doNewObs(const std::string &obsFileName,
		  const std::string &newObs);

    void sendWMORaport(const WMORaport &raport);
    void tryToSendSavedObservations();



    bool getFileList(FileList &fileList,
		     const std::string &path,
		     const std::string &pattern="");

    bool sendMessageToKvalobs(const std::string &msg, 
			      const std::string &obsType,
			      bool &kvServerIsUp,
			      bool &tryToResend)const;
 
    /**
     * 
     * \return the file name on success and an empty string otherwise.
     */
    std::string writeFile(const std::string &dir, 
			  const std::string &fname,
			  bool  fnameIsTemplate,
			  const std::string &content);
      
    bool readFile(const std::string &file, std::string &content)const;


    /**
     * copy a synopfile to the tmpdir. Check if the source file
     * has changed after the copy, if it has changed remove the
     * copy and set the collected flag to false and the seen count to 0.
     *
     * If we cant copy the file, remove the file from the InfoList. We
     * assume the file has been deleted.
     *
     * \param infoList A referance to the infolist.
     * \param it an iterator to an element in infoList.
     * \return An iteretor to infoList.end() if the file referenced by 
     *         it is deleted and the iterator it otherwise.
     */
    IFInfoList copyFile(FInfoList &infoList, IFInfoList it); 
    
public:
    CollectSynop(App &app); 
    ~CollectSynop();
    
    int run();
};

#endif
