/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvSubscriberCollection.h,v 1.2.6.2 2007/09/27 09:02:39 paule Exp $                                                       

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
#ifndef __kvSubscriberCollection_h__
#define __kvSubscriberCollection_h__

#include <boost/thread/thread.hpp>
#include <set>
#include <map>
#include <list>
#include <vector>
#include "kvDataNotifySubscriber.h"
#include "kvDataSubscriber.h"
#include "kvHintSubscriber.h"
#include "kvSubscriberBase.h"
#include <kvskel/kvService.hh>
#include <stdio.h>
#include <puTools/miTime.h>
#include <fstream>

class KvSubscriberCollection{
  std::set<KvSubscriberBasePtr, KvSubscriberBasePtrOps> subscribers_;

  //Subscribers that listen on dataNotify for all stations.
  std::list<KvDataNotifySubscriberPtr> allStationsDataNotifySubscribers;

  //Subscribers that listen on dataNotify for a limited set of stations.
  //The key in the multimap is the stationid.
  std::multimap<long, KvDataNotifySubscriberPtr> stationDataNotifySubscribers;


  //Subscribers that listen on data for all stations.
  std::list<KvDataSubscriberPtr> allStationsDataSubscribers;

  //Subscribers that listen on data for a limited set of stations.
  //The key in the multimap is the stationid.
  std::multimap<long, KvDataSubscriberPtr> stationDataSubscribers;
                        
  std::list<KvHintSubscriberPtr> hintSubscriberList;


  void removeDataNotifySubscriber(const std::string &subscriberid);

  void removeDataSubscriber(const std::string &subscriberid);

  void removeHintSubscriber(const std::string &subscriberid);

  //The following tree functions manipulate the the list subscribersid_
  
  /**
   * createSubscriberid creates an uniqe subscriberid.
   *
   * \return false on failure and true on success.
   */
  bool createSubscriberid(KvSubscriberBasePtr p, 
			  const std::string &servicename);

  void removeSubscriberid(KvSubscriberBasePtr p);
  void removeSubscriberid(const std::string &subscriberid);

  bool writeDataNotifyFile(const std::string &subscriberid);
  bool writeDataFile(const std::string &subscriberid);
  bool writeKvHintFile(const std::string &subscriberid);

  std::ofstream *writeFileHeader(
      const std::string &subscriberid, 
      const kvalobs::KvDataSubscriberInfo *subscriberInfo,
      const std::string &corbaref);

  bool readSubscriberFile(const std::string &fname);
  bool writeSubscriberFile(const std::string &subscriberid);
  bool updateSubscriberFile(const std::string &subscriberid,
			     const miutil::miTime &timeForLastCall);
  bool removeSubscriberFile(const std::string &subscriberid); 


  bool addDataNotifyFromFileFile(
                const std::string &subid,
		const std::string &cref,
		const std::string &statusid,
		const std::vector<std::string> &qcIdList,
		const std::vector<std::string> &stationList);

  bool addDataFromFile(const std::string &subid,
		       const std::string &cref,
		       const std::string &statusid,
		       const std::vector<std::string> &qcIdList,
		       const std::vector<std::string> &stationList);

  bool addKvHintFromFile(const std::string &subid,
			   const std::string &icref);

  kvalobs::KvDataSubscriberInfo createKvDataInfo(
                          const std::string              &statusid,
			  const std::vector<std::string> &qcIdList);
    
  boost::mutex mutex;
  std::string subPath; //The path to the directory where we shall save
                       //subscriber information. 
                       //Default: $KAVLOBS/var/kvalobs/manager/subscriber 

 public:
  KvSubscriberCollection();
  KvSubscriberCollection(const std::string &subscriberpath);
  ~KvSubscriberCollection();

  
  void removeDeadSubscribers(int durationInSeconds);

  
  /**
   * This function adds a DataNotify subscriber that listen for all stations.
   * 
   * \return false on failure and true on success.
   */
    bool addDataNotifySubscriber(KvDataNotifySubscriberPtr p);

  /**
   * This function add a DataNotify subscriber to the list of stastions
   * that listen only for data from a limited set of stations.
   *
   * \return false on failure and true on success.
   */
    bool addDataNotifySubscriber(KvDataNotifySubscriberPtr p, 
				 long stationid);

  /**
   * forAllDataNotifySubcribers, calls the function 'func' on the object
   * 'obj' for all subscribers that liten for data for the station
   * given with the 'stationid'.
   */
  void forAllDataNotifySubscribers(DataNotifySubscriberFuncBase &obj, 
				   long stationid);



  /**
   * This function adds a Data subscriber that listen for all stations.
   * 
   * \return false on failure and true on success.
   */
    bool addDataSubscriber(KvDataSubscriberPtr p);

  /**
   * This function add a Data subscriber to the list of stastions
   * that listen only for data from a limited set of stations.
   *
   * \return false on failure and true on success.
   */
    bool addDataSubscriber(KvDataSubscriberPtr p, 
			   long stationid);

    
  /**
   * forAllDataSubcribers, calls the function 'func' on the object
   * 'obj' for all subscribers that liten for data for the station
   * given with the 'stationid'.
   */
  void forAllDataSubscribers(DataSubscriberFuncBase &obj, 
			     long stationid);




    /**
     * addKvHintSubsriber, add a kvHintSubscriber to the list 
     * kvHintSubscriberList.
     *
     * \param sub the kvHintSubscriber callback.
     * \return true on success, false otherwise.
     */
  bool addKvHintSubscriber(KvHintSubscriberPtr sub);
  
  void sendKvHintUp();
  void sendKvHintDown();



  /**
   * Removes a subscriber from the lists of subscribers.
   */

  void removeSubscriber(const std::string &subscriberid);

  bool hasDataSubscribers()const;
  bool hasDataNotifySubscribers()const;

  void readAllSubscribersFromFile();

  friend std::ostream& operator<<(std::ostream& os, 
				  const KvSubscriberCollection &c);

};


std::ostream& 
operator<<(std::ostream& os,  const KvSubscriberCollection &c);

#endif
