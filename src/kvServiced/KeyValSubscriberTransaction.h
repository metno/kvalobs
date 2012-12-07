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

#ifndef __KEYVALSUBSCRIBERTRANSACTION_H__
#define __KEYVALSUBSCRIBERTRANSACTION_H__

#include <string>
#include <list>
#include <puTools/miTime.h>
#include <kvdb/kvdb.h>
#include <kvalobs/kvKeyVal.h>

class KeyValSubscriberTransaction: public dnmi::db::Transaction
{
public:
    const std::string SubScriberID;
    typedef enum
    {
        INSERT_OR_UPDATE, UPDATE_LAST_CALL, DELETE_SUBSCRIBER,
        GET_SUBSCRIBER
    } Action;


private:
    KeyValSubscriberTransaction& operator=(const KeyValSubscriberTransaction &);
    KeyValSubscriberTransaction(const KeyValSubscriberTransaction &);
    KeyValSubscriberTransaction();

protected:
    Action action;
    std::string subscriberid;
    std::string content;
    miutil::miTime lastCall;
    std::ostringstream err;
    std::string msg;
    std::list<kvalobs::kvKeyVal> keyVals;
    dnmi::db::Connection *con;
    bool ok;

    bool insert( const std::string &val );
    bool update( const std::string &val );
    bool get( std::list<kvalobs::kvKeyVal> &vals, const std::string &subid );
    bool insertOrUpdate();
    bool updateLastCall();
    bool removeSubscriber();

public:

    KeyValSubscriberTransaction( const std::string &subid, const miutil::miTime &lastCall_ );

    KeyValSubscriberTransaction( const std::string &subid, const std::string &content_ );

    /**
     * Use this constructor to get or delete a subscriberid
     * or all if the subscriberid is empty. Valid values
     * for act is DELETE_SUBSCRIBER and GET_SUBSCRIBER.
     */
    KeyValSubscriberTransaction( const std::string &subid, Action act=GET_SUBSCRIBER );
    ~KeyValSubscriberTransaction();

    virtual bool operator()(dnmi::db::Connection *conection);
    virtual void onAbort(const std::string &driverid,
            const std::string &errorMessage, const std::string &errorCode);
    virtual void onSuccess();
    virtual void onRetry();
    virtual void onMaxRetry(const std::string &lastError);

    std::string getMsg()const { return msg; }
    std::list<kvalobs::kvKeyVal> getKeyVals()const { return keyVals; }
    bool isOk()const { return ok; }
};





#endif
