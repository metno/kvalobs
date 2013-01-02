/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvSubscriberCollection.cc,v 1.2.6.3 2007/09/27 09:02:39 paule Exp $                                                       

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
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <iomanip>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/thread.hpp>
#include <dnmithread/mtcout.h>
#include <milog/milog.h>
#include <corbahelper/corbaApp.h>
#include <fileutil/dir.h>
#include <kvalobs/kvPath.h>
#include <kvalobs/kvDbGate.h>
#include "kvSubscriberCollection.h"
#include "KeyValSubscriberTransaction.h"
#include "ServiceApp.h"

using namespace std;
using namespace miutil;
using namespace kvalobs;


namespace {
const std::string SubScriberID("'KvServiceSubcriberIDS'");

struct DbConnectionHolder {
    dnmi::db::Connection *con;
    ServiceApp &app;

    DbConnectionHolder( ServiceApp &app_, dnmi::db::Connection *con_ )
    : app( app_), con( con_ ){
        LOGDEBUG("DbConnectionHolder: thread new db connection.")
    }

    ~DbConnectionHolder() {
        LOGDEBUG("DbConnectionHolder: thread release db connection.")
                app.releaseDbConnection( con );
    }

};


boost::thread_specific_ptr<DbConnectionHolder> dbConectionHolder;



}


KvSubscriberCollection::
KvSubscriberCollection( ServiceApp &app_ )
: app( app_ ), isInitialized( false )
{
}


KvSubscriberCollection::
~KvSubscriberCollection()
{
    LOGDEBUG("KvSubscriberCollection::deleted!");
}



dnmi::db::Connection*
KvSubscriberCollection::
getDbConnection()
{
    DbConnectionHolder *dbHolder=dbConectionHolder.get();

    if( ! dbHolder ) {
        dnmi::db::Connection *con = app.getNewDbConnection();
        if( con ) {
            dbConectionHolder.reset( new DbConnectionHolder( app, con ) );
        }
        return con;
    } else {
        return dbHolder->con;
    }
}

void
KvSubscriberCollection::
releaseThisThreadsDbConnection( )
{
    DbConnectionHolder *dbHolder=dbConectionHolder.get();

    if( dbHolder ) {
        dbConectionHolder.reset(0);
    }
}

void
KvSubscriberCollection::
updateSubscriberInDb(
        const std::string &subid,
        const std::string &content )
{
    dnmi::db::Connection *con = getDbConnection();

    if( ! con ) {
        LOGERROR("KvSubscriberCollection: No db connection. Cant read subscribers from the database.");
        return;
    }

    //Get all subscribers stored in the KeyVal table in the database.
    KeyValSubscriberTransaction tran( subid, content );

    try {
        con->perform( tran );
        if( ! tran.isOk() ) {
            LOGERROR("KvSubscriberCollection: Insert/Update subscriber <" << subid << "> FAILED."
                    << endl << "Reason: " << tran.getMsg() );
            return;
        } else {
            LOGDEBUG("KvSubscriberCollection: Inserted/Updated subscriber <" << subid << ">."
                    << endl << "Msg: " << tran.getMsg() );
        }
    }
    catch( const std::exception &ex ) {
        LOGERROR("KvSubscriberCollection: Insert/Update subscriber <" << subid << "> FAILED."
                << endl << "Reason: (exception)" << ex.what() << endl
                << tran.getMsg() );
        return;
    }
}



bool
KvSubscriberCollection::
createSubscriberid(
        KvSubscriberBasePtr p,
        const std::string &servicename
)
{
    int                COUNT_MAX=1000;
    int                count=0;
    std::ostringstream ost;
    std::string        subid;
    time_t             t;
    char               tBuf[30];
    struct tm          tTm;
    std::set<KvSubscriberBasePtr, KvSubscriberBasePtrOps>::iterator it;

    // The chema used to create a uniqe subscriberid:
    // 'kvalobs_service_servicename_datotid_helper'
    // helper is used too garanti uniqnes if datotid and servicename
    // is equal.

    if(time(&t)<0){
        LOGERROR("createSubscriberid: time() failed!");
        return false;
    }

    if(!gmtime_r( &t, &tTm)){
        LOGERROR("createSubscriberid: gmtime_r() failed!");
        return false;
    }

    sprintf(tBuf, "%04d%02d%02dT%02d%02d%02d",
            tTm.tm_year+1900, tTm.tm_mon+1, tTm.tm_mday,
            tTm.tm_hour, tTm.tm_min, tTm.tm_sec);

    for(count=0;
            count<COUNT_MAX && subid.empty();
            count++){
        ost.str("");   //reset ost.
        ost << servicename << "_" << tBuf << "_" << count;

        subid=ost.str();

        for(it=subscribers_.begin(); it!=subscribers_.end(); it++){
            if((*it)->subscriberid()==subid)
                break;
        }

        if(it!=subscribers_.end())
            subid.erase();
    }

    if(subid.empty()){
        LOGERROR("createSubscriberid: cant create subscriberid!\n");
        return false;
    }

    p->subscriberid(subid);
    subscribers_.insert(p);

    LOGDEBUG("#subscribers_: " << subscribers_.size());

    return true;
}



void 
KvSubscriberCollection::
removeSubscriberid(KvSubscriberBasePtr p)
{
    std::set<KvSubscriberBasePtr, KvSubscriberBasePtrOps>::iterator it;

    it=subscribers_.find(p);

    if(it!=subscribers_.end()){
        subscribers_.erase(it);
        LOGDEBUG("KvSubscriberCollection::removeSubscriberid: "
                << p->subscriberid()
                << endl);
        removeSubscriberFromDb(p->subscriberid());

        return;
    }

    LOGDEBUG("KvSubscriberCollection::removeSubscriberid: "
            << p->subscriberid()
            << " is not among the subscribers!");
}

void 
KvSubscriberCollection::
removeSubscriberid(const std::string &subscriberid)
{
    std::set<KvSubscriberBasePtr, KvSubscriberBasePtrOps>::iterator it;

    LOGDEBUG("removeSubscriberid(" << subscriberid <<").....!\n");

    for(it=subscribers_.begin();
            it!=subscribers_.end();
            it++){

        if((*it)->subscriberid()==subscriberid){
            LOGDEBUG("KvSubscriberCollection::removeSubscriberid: "
                    << (*it)->subscriberid()
                    << endl);
            subscribers_.erase(it);
            removeSubscriberFromDb(subscriberid);
            return;
        }
    }

    LOGDEBUG("KvSubscriberCollection::removeSubscriberid: "
            << subscriberid
            << " is not among the subscribers. (FATAL?)");

}




bool
KvSubscriberCollection::
addDataNotifySubscriber( KvDataNotifySubscriberPtr p )
{
    //This method is called from kvServiceImpl only.
    //We release the database connection before
    //we return.
    boost::mutex::scoped_lock lock(mutex);

    readAllSubscribersFromDb();

    if(!createSubscriberid(p, "datanotify"))
        return false;

    allStationsDataNotifySubscribers.push_back(p);
    writeSubscriberFromDb(p->subscriberid());
    releaseThisThreadsDbConnection();
    return true;
}

bool
KvSubscriberCollection::
addDataNotifySubscriber( KvDataNotifySubscriberPtr p, long stationid )
{
    //This method is called from kvServiceImpl only.
    //We release the database connection before
    //we return.
    boost::mutex::scoped_lock lock(mutex);

    readAllSubscribersFromDb();

    std::string subscriberid;
    bool subscriberidCreated=false;

    if(p->subscriberid().empty()){
        if(!createSubscriberid(p, "datanotify"))
            return false;

        subscriberidCreated=true;
    }

    try{
        stationDataNotifySubscribers.insert(make_pair(stationid, p));
    }
    catch(...){
        if(subscriberidCreated)
            removeSubscriberid(p);

        return false;
    }

    writeSubscriberFromDb(p->subscriberid());
    releaseThisThreadsDbConnection();
    return true;
}




void 
KvSubscriberCollection::
removeDataNotifySubscriber(const std::string &subscriberid)
{
    list<KvDataNotifySubscriberPtr>::iterator it;

    LOGDEBUG("removeDataNotifySubscriberid(" <<subscriberid <<").....!\n");

    it=allStationsDataNotifySubscribers.begin();

    while(it!=allStationsDataNotifySubscribers.end()){
        if((*it)->subscriberid()==subscriberid){
            allStationsDataNotifySubscribers.erase(it);
            it=allStationsDataNotifySubscribers.begin();
        }else
            it++;
    }

    multimap<long, KvDataNotifySubscriberPtr>::iterator it1,ittmp;

    it1=stationDataNotifySubscribers.begin();

    while(it1!=stationDataNotifySubscribers.end()){
        ittmp=it1;
        it1++;

        if(ittmp->second->subscriberid()==subscriberid){
            stationDataNotifySubscribers.erase(ittmp);
        }
    }
}

void 
KvSubscriberCollection::
removeHintSubscriber(const std::string &subscriberid)
{
    std::list<KvHintSubscriberPtr>::iterator it;

    it=hintSubscriberList.begin();

    for(;it!=hintSubscriberList.end(); it++){
        if((*it)->subscriberid()==subscriberid){
            hintSubscriberList.erase(it);
            return;
        }
    }
}


void 
KvSubscriberCollection::
removeDataSubscriber(const std::string &subscriberid)
{
    list<KvDataSubscriberPtr>::iterator it;

    LOGDEBUG("removeDataSubscriberid(" <<subscriberid <<").....!\n");

    it=allStationsDataSubscribers.begin();

    while(it!=allStationsDataSubscribers.end()){
        if((*it)->subscriberid()==subscriberid){
            allStationsDataSubscribers.erase(it);
            it=allStationsDataSubscribers.begin();
        }else
            it++;
    }

    multimap<long, KvDataSubscriberPtr>::iterator it1,ittmp;

    it1=stationDataSubscribers.begin();

    while(it1!=stationDataSubscribers.end()){
        ittmp=it1;
        it1++;

        if(ittmp->second->subscriberid()==subscriberid){
            stationDataSubscribers.erase(ittmp);
        }
    }

}

void 
KvSubscriberCollection::
doRemoveSubscriber(const std::string &subscriberid)
{
    removeDataNotifySubscriber(subscriberid);
    removeDataSubscriber(subscriberid);
    removeHintSubscriber(subscriberid);
    removeSubscriberid(subscriberid);
    removeSubscriberFromDb(subscriberid);
}


void
KvSubscriberCollection::
removeSubscriber(const std::string &subscriberid)
{
    boost::mutex::scoped_lock lock(mutex);
    readAllSubscribersFromDb();
    doRemoveSubscriber( subscriberid );
}

/*
 * Walk through all the subscribers lists and find all subscribers that
 * we cant connect to and remove them from our list of subscribers.
 */
void 
KvSubscriberCollection::
removeDeadSubscribers(int durationInSeconds)
{
    std::list<std::string> subList;

        boost::mutex::scoped_lock lock(mutex);

        readAllSubscribersFromDb();

        for(std::list<KvDataNotifySubscriberPtr>::iterator
                it=allStationsDataNotifySubscribers.begin();
                it!=allStationsDataNotifySubscribers.end();
                it++){
            if((*it)->removeThisSubscriber(durationInSeconds)){
                subList.push_back((*it)->subscriberid());
            }
        }


        for(std::multimap<long, KvDataNotifySubscriberPtr>::iterator
                it=stationDataNotifySubscribers.begin();
                it!=stationDataNotifySubscribers.end();
                it++){
            if(it->second->removeThisSubscriber(durationInSeconds)){
                subList.push_back(it->second->subscriberid());
            }
        }

        for(std::list<KvDataSubscriberPtr>::iterator
                it=allStationsDataSubscribers.begin();
                it!=allStationsDataSubscribers.end();
                it++){
            if((*it)->removeThisSubscriber(durationInSeconds)){
                subList.push_back((*it)->subscriberid());
            }
        }

        for(std::multimap<long, KvDataSubscriberPtr>::iterator
                it=stationDataSubscribers.begin();
                it!=stationDataSubscribers.end();
                it++){
            if(it->second->removeThisSubscriber(durationInSeconds)){
                subList.push_back(it->second->subscriberid());
            }
        }

    for(std::list<std::string>::iterator it=subList.begin();
            it!=subList.end();
            it++){
        doRemoveSubscriber(*it);
    }
}


void 
KvSubscriberCollection::
forAllDataNotifySubscribers(
        DataNotifySubscriberFuncBase &obj,
        long                         stationid
)
{
    list<KvDataNotifySubscriberPtr>::iterator it;

    boost::mutex::scoped_lock lock(mutex);
    readAllSubscribersFromDb();

    it=allStationsDataNotifySubscribers.begin();

    for(;it!=allStationsDataNotifySubscribers.end(); it++){
        obj.func(*it);
    }

    multimap<long, KvDataNotifySubscriberPtr>::iterator itu, itl;

    itu=stationDataNotifySubscribers.lower_bound(stationid);

    if(itu!=stationDataNotifySubscribers.end()){
        itl=stationDataNotifySubscribers.upper_bound(stationid);

        for(;itu!=itl; itu++){
            obj.func(itu->second);
        }
    }
}







bool 
KvSubscriberCollection::
addDataSubscriber(KvDataSubscriberPtr p)
{
    //This method is called from kvServiceImpl only.
    //We release the database connection before
    //we return.
    boost::mutex::scoped_lock lock(mutex);

    readAllSubscribersFromDb();

    if(!createSubscriberid(p, "data"))
        return false;

    allStationsDataSubscribers.push_back(p);

    writeSubscriberFromDb(p->subscriberid());
    releaseThisThreadsDbConnection();
    return true;

}


bool 
KvSubscriberCollection::
addDataSubscriber(KvDataSubscriberPtr p, long stationid)
{
    //This method is called from kvServiceImpl only.
    //We release the database connection before
    //we return.
    boost::mutex::scoped_lock lock(mutex);

    readAllSubscribersFromDb();

    multimap<long, KvDataSubscriberPtr>::iterator itu, itl;
    bool subscriberidCreated=false;

    if(p->subscriberid().empty()){
        if(!createSubscriberid(p, "data"))
            return false;

        subscriberidCreated=true;
    }

    try{
        stationDataSubscribers.insert(make_pair(stationid, p));
    }
    catch(...){
        if(subscriberidCreated)
            removeSubscriberid(p);

        return false;
    }

    writeSubscriberFromDb(p->subscriberid());
    releaseThisThreadsDbConnection();
    return true;
}

bool  
KvSubscriberCollection::
addKvHintSubscriber(KvHintSubscriberPtr sub )
{
    //This method is called from kvServiceImpl only.
    //We release the database connection before
    //we return.
    using namespace CKvalObs::CService;

    boost::mutex::scoped_lock lock( mutex );
    readAllSubscribersFromDb();

    if(!createSubscriberid(sub, "kvHint"))
        return false;


    hintSubscriberList.push_back(sub);
    writeSubscriberFromDb(sub->subscriberid());
    releaseThisThreadsDbConnection();

    return true;
}



void 
KvSubscriberCollection::
sendKvHintUp()
{
    using namespace CKvalObs::CService;

    std::list<KvHintSubscriberPtr>::iterator it;
    kvHintSubscriber_var sub;

    boost::mutex::scoped_lock lock( mutex );

    readAllSubscribersFromDb();

    it=hintSubscriberList.begin();

    while(it!=hintSubscriberList.end()){
        try{
            sub=(*it)->subscriber();
            sub->kvUp();
            it++;
        }
        catch(...){
            std::list<KvHintSubscriberPtr>::iterator itTmp=it;
            it++;
            LOGINFO("SUBSCRIBER NOT LISTNING: Deletes the subscriber <"
                    << (*itTmp)->subscriberid() << "> from the list of\n" <<
                    "kvHint subscribers.");
            doRemoveSubscriber((*itTmp)->subscriberid());
        }
    }
}

void 
KvSubscriberCollection::
sendKvHintDown()
{
    using namespace CKvalObs::CService; 
    kvHintSubscriber_var sub;
    std::list<KvHintSubscriberPtr>::iterator it;
    boost::mutex::scoped_lock lock( mutex );

    readAllSubscribersFromDb();

    it=hintSubscriberList.begin();

    for(;it!=hintSubscriberList.end(); it++){
        try{
            sub=(*it)->subscriber();
            sub->kvDown();
        }
        catch(...){
            LOGINFO("SUBSCRIBER NOT LISTNING: Deletes the subscriber <"
                    << (*it)->subscriberid() << "> from the list of\n" <<
                    "kvHint subscribers.");
            doRemoveSubscriber((*it)->subscriberid());
        }
    }

}


void 
KvSubscriberCollection::
forAllDataSubscribers( DataSubscriberFuncBase &obj,
        long stationid)
{
    list<KvDataSubscriberPtr>::iterator it;

    boost::mutex::scoped_lock lock(mutex);

    readAllSubscribersFromDb();

    it=allStationsDataSubscribers.begin();

    for(;it!=allStationsDataSubscribers.end(); it++){
        obj.func(*it);
    }

    multimap<long, KvDataSubscriberPtr>::iterator itu, itl;

    itl=stationDataSubscribers.lower_bound(stationid);

    if(itl!=stationDataSubscribers.end()){
        itu=stationDataSubscribers.upper_bound(stationid);

        for(;itl!=itu; itl++){
            obj.func(itl->second);
        }
    }

}



bool 
KvSubscriberCollection::
hasDataSubscribers()
{
    boost::mutex::scoped_lock lock( mutex );
    readAllSubscribersFromDb();
    if(allStationsDataSubscribers.empty() &&
            stationDataSubscribers.empty())
        return false;

    return true;
}

bool 
KvSubscriberCollection::
hasDataNotifySubscribers()
{
    boost::mutex::scoped_lock lock( mutex );
    readAllSubscribersFromDb();
    if(allStationsDataNotifySubscribers.empty() &&
            stationDataNotifySubscribers.empty())
        return false;

    return true;
}


std::ostringstream*
KvSubscriberCollection::
writeHeader( const std::string &subscriberid,
        const kvalobs::KvDataSubscriberInfo *si,
        const std::string &corbaref)
{
    ostringstream *fs;
    miTime tNow(miTime::nowTime());

    try{
        fs=new ostringstream();
    }catch(...){
        LOGERROR("OUT OF MEM: cant create an instance of ofstream!");
        return 0;
    }

    /*
     * OBS, OBS
     * The 'Last Call: ' line must match the 'Last Call: ' line in the function
     * updateSubscriberFile. 
     */

    (*fs) << "Last call: " 
          << tNow.isoTime() << endl;

    (*fs) << "Created: "  << tNow.isoTime() << endl
            << "Subid: " << subscriberid << endl
            << "CORBA ref: " << corbaref << endl
            << "StatusId: ";


    if(si){
        switch(si->status()){
        case CKvalObs::CService::All:        (*fs) << "All";        break;
        case CKvalObs::CService::OnlyFailed: (*fs) << "OnlyFailed"; break;
        case CKvalObs::CService::OnlyOk:     (*fs) << "OnlyOk";     break;
        }
    }


    (*fs) << endl;
    (*fs) << "QcId:";

    if(si){
        if(!si->qcAll()){
            for(int i=0; i<si->qc().length(); i++){
                switch(si->qc()[i]){
                case CKvalObs::CService::QC1:  (*fs) << " QC1";  break;
                case CKvalObs::CService::QC2d: (*fs) << " QC2d"; break;
                case CKvalObs::CService::QC2m: (*fs) << " QC2m"; break;
                case CKvalObs::CService::HQC:  (*fs) << " HQC";  break;
                }
            }
        }
    }

    (*fs) << endl;
    (*fs) << "Stations:";

    return fs;
}

//bool
//KvSubscriberCollection::
//updateSubscriberFile(
//        const std::string &subscriberid,
//        const miutil::miTime &timeForLastCall
//)
//{
//
//    dnmi::db::Connection *con = getDbConnection();
//
//    if( ! con ) {
//        LOGERROR("KvSubscriberCollection: No db connection. Cant read subscribers from the database.");
//        return false;
//    }
//
//    //Update lastCall for a subscriber stored in the KeyVal table in the database.
//    KeyValSubscriberTransaction tran( subscriberid, timeForLastCall );
//
//    try {
//        con->perform( tran );
//        if( ! tran.isOk() ) {
//            LOGERROR("KvSubscriberCollection: Update last call for subscriber <" << subscriberid <<"> FAILED."
//                    << endl << "Reason: " << tran.getMsg() );
//            return false;
//        } else {
//                LOGDEBUG("KvSubscriberCollection: Updated last call for subscriber <" << subscriberid <<"> to " << timeForLastCall.isoTime() << "."
//                        << endl << "Msg: " << tran.getMsg() );
//        }
//    }
//    catch( const std::exception &ex ) {
//        LOGERROR("KvSubscriberCollection: Read all from subscribers from the database FAILED."
//                << endl << "Reason: (exception)" << ex.what() << endl
//                << tran.getMsg() );
//        return false;
//    }
//
//    return true;
//}


void
KvSubscriberCollection::
readAllSubscribersFromDb()
{
    if( ! isInitialized ) {
        dnmi::db::Connection *con = getDbConnection();

        LOGDEBUG("@@@@@@ readAllSubscribersFromDb 1 ");

        if( ! con ) {
            LOGERROR("KvSubscriberCollection: No db connection. Cant read subscribers from the database.");
            return;
        }

        LOGDEBUG("@@@@@@ readAllSubscribersFromDb 2 ");
        //Get all subscribers stored in the KeyVal table in the database.
        KeyValSubscriberTransaction tran("", KeyValSubscriberTransaction::GET_SUBSCRIBER );

        try {
            con->perform( tran );
            if( ! tran.isOk() ) {
                LOGERROR("KvSubscriberCollection: Read all from subscribers from the database FAILED."
                        << endl << "Reason: " << tran.getMsg() );
                return;
            } else {
                LOGDEBUG("KvSubscriberCollection: Read all from subscribers from the database OK."
                        << endl << "Msg: " << tran.getMsg() );
            }
        }
        catch( const std::exception &ex ) {
            LOGERROR("KvSubscriberCollection: Read all from subscribers from the database FAILED."
                    << endl << "Reason: (exception)" << ex.what() << endl
                    << tran.getMsg() );
            return;
        }

        list<kvKeyVal> allSubscribers = tran.getKeyVals();

        for( list<kvKeyVal>::iterator it = allSubscribers.begin();
                it != allSubscribers.end(); ++it ) {

            //I dont check the return value from readSubscriberFromFile
            //because the errors are reported in the function. And it doesn't
            //matter for the controll flow here.
            readSubscriberFromDb(it->key(), it->val() );
        }
        isInitialized = true;
    }
}


bool 
KvSubscriberCollection::
readSubscriberFromDb(
        const std::string &subscriberid,
        const std::string &content
)
{
    std::string::size_type i;
    std::string  buf;
    std::string key;
    std::string val;
    istringstream fs( content );
    miTime   lastCall;
    miTime   created;
    std::string subid;
    std::string cref;
    std::string statusid;
    vector<std::string> qcIdList;
    vector<std::string> stationList;
    bool hasStationKey=false;
    bool allStations=false;
    bool hasQcIdKey=false;
    string subtype;   //Subscriber type 
    bool   res;

    while(!fs.eof()){
        if(!getline(fs, buf))
            continue;

        boost::trim(buf);

        if(buf.empty())
            continue;

        i=buf.find(":");

        if(i==string::npos){
            LOGERROR("FORMAT ERROR: readSubscriber <"
                    << subscriberid << "> failed!");
            return false;
        }

        key=buf.substr(0, i);
        val=buf.substr(i+1);

        boost::trim(val);

        if(key.find("Last call")!=string::npos){
            lastCall=miTime(val);
        }else if(key.find("Created")!=string::npos){
            created=miTime(val);
        }else if(key.find("Subid")!=string::npos){
            subid=val;
        }else if(key.find("CORBA ref")!=string::npos){
            cref=val;
        }else if(key.find("StatusId")!=string::npos){
            statusid=val;
        }else if(key.find("QcId")!=string::npos){
            hasQcIdKey=true;
            boost::split(qcIdList, val, boost::algorithm::is_space(), boost::algorithm::token_compress_on);
        }else if(key.find("Stations")!=string::npos){
            hasStationKey=true;

            if(val.empty()){
                //Threat as "None"
                allStations=false;
            }else if(val.find("All")){
                allStations=true;
            }else if(val.find("None")){
                allStations=false;
            }else{
                allStations=false;
                boost::split(stationList, val, boost::algorithm::is_space(), boost::algorithm::token_compress_on);
            }
        }else{
            LOGWARN("Uknown key <" << key << ">: readSubscriber <"
                    << subscriberid << ">!");
        }


    }

    if(subid.empty() || cref.empty()){
        LOGERROR("Format error: readSubscriber <" << subscriberid << "> "
                << "missing key <Subid> or <CORBA ref>!");
        return false;
    }


    i=subid.find("_");

    if(i==string::npos){
        LOGERROR("Ivalid subscriberid [missing subscriber type]: " << subid <<
                ">,  readSubscriber <" << subscriberid << ">!");
        return false;
    }

    subtype=subid.substr(0, i);

    boost::trim( subtype );

    if(subtype=="datanotify"){
        if(!hasStationKey){
            LOGERROR("Missing key <Stations> for datanotify subscriber: " <<
                    "readSubscriber <" << subid << ">!");
            return false;
        }

        res=addDataNotifyFromDb(subid,
                cref,
                statusid,
                qcIdList,
                stationList);

    }else if(subtype=="data"){
        if(!hasStationKey){
            LOGERROR("Missing key <Stations> for data subscriber: " <<
                    "readSubscriber <" << subid << ">!");
            return false;
        }

        res=addDataFromDb(subid,
                cref,
                statusid,
                qcIdList,
                stationList);
    }else if(subtype=="kvHint"){
        res=addKvHintFromDb(subid, cref);
    }else{
        LOGERROR("Unknown subscriber type: " << subtype <<
                "\nsubscriberid:          " << subid   <<
                "\nreadSubscriber:    " << subscriberid      );
        return false;
    }

    if(res){
        LOGINFO("Added subscriber <" << subtype
                << ">  id <" << subid << ">!");
        return true;
    }else{
        LOGERROR("Could not add subscriber <" << subtype
                << "> id <" << subid << ">!");
        return false;
    }
}

bool 
KvSubscriberCollection::
writeSubscriberFromDb(const std::string &subid)
{

    string::size_type i;
    string subtype;   //Subscriber type 


    i=subid.find( "_" );

    if(i==string::npos){
        LOGERROR("Inavlid subscriberid [missing subscriber type]: " << subid );
        return false;
    }

    subtype=subid.substr( 0, i );

    boost::trim( subtype );

    if(subtype=="datanotify"){
        return writeDataNotifyToDb(subid);
    }else if(subtype=="data"){
        return writeDataToDb(subid);
    }else if(subtype=="kvHint"){
        return writeKvHintToDb(subid);
    }else{
        LOGERROR("Unknown subscriber type: " << subtype <<
                " [subscriberid: " << subid << "]");
        return false;
    }

    //Not reached!!!!!!
    return false;
}

bool
KvSubscriberCollection::
writeDataNotifyToDb(const std::string &subid)
{
    CorbaHelper::CorbaApp *cApp=CorbaHelper::CorbaApp::getCorbaApp();
    CKvalObs::CService::kvDataNotifySubscriber_var subscriber;
    ostringstream *fs=0;
    std::list<KvDataNotifySubscriberPtr>::iterator itAllStations;
    std::multimap<long, KvDataNotifySubscriberPtr>::iterator it; 
    string  corbaRef;
    kvalobs::KvDataSubscriberInfo info;

    if(!cApp){
        LOGFATAL("Cant obtain a referance to CorbaHelper::CorbaApp.");
        return false;
    }

    for(itAllStations=allStationsDataNotifySubscribers.begin();
            itAllStations!=allStationsDataNotifySubscribers.end();
            itAllStations++){

        if((*itAllStations)->subscriberid()==subid)
            break;
    }

    if(itAllStations!=allStationsDataNotifySubscribers.end()){
        subscriber=(*itAllStations)->subscriber();
        corbaRef=cApp->corbaRef(subscriber);
        info=(*itAllStations)->subscriberInfo();

        fs=writeHeader(subid, &info, corbaRef);

        if(fs){
            (*fs) << "All\n";
            updateSubscriberInDb( subid, fs->str() );
            delete fs;
            return true;
        }else
            return false;
    }

    for(it=stationDataNotifySubscribers.begin();
            it!=stationDataNotifySubscribers.end();
            it++){

        if(it->second->subscriberid()==subid){
            if(!fs){
                subscriber=it->second->subscriber();
                corbaRef=cApp->corbaRef(subscriber);
                info=it->second->subscriberInfo();

                fs=writeHeader(subid, &info, corbaRef);

                if(!fs)
                    return false;
                else
                    (*fs) << it->first;
            }else{
                (*fs) << " " << it->first;
            }
        }
    }

    if(fs){
        (*fs) << std::endl;
        updateSubscriberInDb( subid, fs->str() );
        delete fs;
    }

    return true;
}

bool
KvSubscriberCollection::
writeDataToDb(const std::string &subid)
{
    CorbaHelper::CorbaApp *cApp=CorbaHelper::CorbaApp::getCorbaApp();
    CKvalObs::CService::kvDataSubscriber_var subscriber;
    ostringstream *fs=0;
    std::list<KvDataSubscriberPtr>::iterator itAllStations;
    std::multimap<long, KvDataSubscriberPtr>::iterator it; 
    string  corbaRef;
    kvalobs::KvDataSubscriberInfo info;

    if(!cApp){
        LOGFATAL("Cant obtain a referance to CorbaHelper::CorbaApp.");
        return false;
    }

    for(itAllStations=allStationsDataSubscribers.begin();
            itAllStations!=allStationsDataSubscribers.end();
            itAllStations++){

        if((*itAllStations)->subscriberid()==subid)
            break;
    }

    if(itAllStations!=allStationsDataSubscribers.end()){
        subscriber=(*itAllStations)->subscriber();
        corbaRef=cApp->corbaRef(subscriber);
        info=(*itAllStations)->subscriberInfo();

        fs=writeHeader(subid, &info, corbaRef);

        if(fs){
            (*fs) << "All\n";
            updateSubscriberInDb( subid, fs->str() );
            delete fs;
            return true;
        }else
            return false;
    }

    for(it=stationDataSubscribers.begin();
            it!=stationDataSubscribers.end();
            it++){

        if(it->second->subscriberid()==subid){
            if(!fs){
                subscriber=it->second->subscriber();
                corbaRef=cApp->corbaRef(subscriber);
                info=it->second->subscriberInfo();

                fs=writeHeader(subid, &info, corbaRef);

                if(!fs)
                    return false;
                else
                    (*fs) << it->first;
            }else{
                (*fs) << " " << it->first;
            }
        }
    }

    if(fs){
        (*fs) << std::endl;
        updateSubscriberInDb( subid, fs->str() );
        delete fs;
    }

    return true;;
}

bool
KvSubscriberCollection::
writeKvHintToDb(const std::string &subid)
{
    CorbaHelper::CorbaApp *cApp=CorbaHelper::CorbaApp::getCorbaApp();
    CKvalObs::CService::kvHintSubscriber_var subscriber;
    std::list<KvHintSubscriberPtr>::iterator it;
    ostringstream *fs=0;
    string  corbaRef;

    if(!cApp){
        LOGFATAL("Cant obtain a referance to CorbaHelper::CorbaApp.");
        return false;
    }

    for(it=hintSubscriberList.begin();
            it!=hintSubscriberList.end();
            it++){

        if((*it)->subscriberid()==subid){
            subscriber=(*it)->subscriber();
            corbaRef=cApp->corbaRef(subscriber);
            fs=writeHeader(subid, 0, corbaRef);

            if(fs){
                (*fs) << "None\n";
                updateSubscriberInDb( subid, fs->str() );
                delete fs;
                return true;
            }else{
                return false;
            }
        }
    }   

    return false;
}

bool 
KvSubscriberCollection::
addDataNotifyFromDb(
        const std::string &subid,
        const std::string &cref,
        const std::string &statusid,
        const std::vector<std::string> &qcIdList,
        const std::vector<std::string> &stationList)
{
    CorbaHelper::CorbaApp *cApp=CorbaHelper::CorbaApp::getCorbaApp();
    CKvalObs::CService::kvDataNotifySubscriber_ptr cptrSub;
    CORBA::Object_var                              cptrCO;
    std::vector<std::string>::const_iterator  it;
    KvDataNotifySubscriberPtr                      subPtr;
    KvDataNotifySubscriber                         *p;
    long                                           stationid;
    int                                            nStations=0;

    if(!cApp){
        LOGFATAL("Cant obtain a referance to CorbaHelper::CorbaApp.");
        return false;
    }

    cptrCO=cApp->corbaRef(cref);

    if(CORBA::is_nil(cptrCO)){
        LOGERROR("CORBA: null refernce!");
        return false;
    }

    try{
        cptrSub=CKvalObs::CService::kvDataNotifySubscriber::_narrow(cptrCO);

        if(CORBA::is_nil(cptrSub)){
            LOGERROR("CORBA: null refernce! Wrong type.");
            CORBA::release(cptrSub);
            return false;
        }
    }
    catch(...){
        LOGERROR("EXCEPTION: CORBA: null refernce! Wrong type.");
        return false;
    }


    kvalobs::KvDataSubscriberInfo info=createKvDataInfo(statusid, qcIdList);


    try{
        p=new KvDataNotifySubscriber(info, cptrSub);
        subPtr.reset(p);
    }
    catch(...){
        CORBA::release(cptrSub);
        LOGERROR("OUT OF MEMMORY: KvSubscriberCollection::addDataNotifyFromFileFile!");
        return false;
    }

    subPtr->subscriberid(static_cast<std::string>(subid));

    if(stationList.empty()){
        allStationsDataNotifySubscribers.push_back(subPtr);
        subscribers_.insert(subPtr);
        return true;
    }

    for(it=stationList.begin();
            it!=stationList.end();
            it++){

        try{
            stationid=boost::lexical_cast<long>(*it);
        }
        catch(boost::bad_lexical_cast &ex){
            LOGERROR("NOT A NUMBER: " << *it);
            continue;
        }
        catch(...){
            LOGERROR("UNKNOWN Exception: trying stationid=boost::numeric_cast<long>(*it)!");
            continue;
        }

        try{
            stationDataNotifySubscribers.insert(make_pair(stationid, subPtr));
            nStations++;
        }
        catch(...){
            LOGWARN("OUT OF MEMMORY: cant add datanotify subscriber!");
        }
    }

    if(nStations>0){
        subscribers_.insert(subPtr);
        return true;
    }

    return false;
}

kvalobs::KvDataSubscriberInfo
KvSubscriberCollection::
createKvDataInfo(
        const std::string              &statusid,
        const std::vector<std::string> &qcIdList)
{
    CKvalObs::CService::QcIdList                   qcList;
    CKvalObs::CService::StatusId                   sid;
    std::vector<std::string>::const_iterator  it;
    CKvalObs::CService::QcId                       qcId;

    if(statusid=="All")
        sid=CKvalObs::CService::All;
    else if(statusid=="OnlyFailed")
        sid=CKvalObs::CService::OnlyFailed;
    else if(statusid=="OnlyOk")
        sid=CKvalObs::CService::OnlyOk;
    else{
        LOGERROR("UNKOWNN CKvalObs::CService::StatusId: <" << statusid
                << ">!");
        sid=CKvalObs::CService::All;
    }

    it=qcIdList.begin();

    for(CORBA::Long i=0;
            it!=qcIdList.end();
            it++, i++){

        if(*it=="QC1")
            qcId=CKvalObs::CService::QC1;
        else if(*it=="QC2d")
            qcId=CKvalObs::CService::QC2d;
        else if(*it=="QC2m")
            qcId=CKvalObs::CService::QC2m;
        else if(*it=="HQC")
            qcId=CKvalObs::CService::HQC;
        else{
            LOGERROR("UNKNOWN CKvalObs::CService::qcId: < " << *it << ">!");
            continue;
        }
        qcList.length(i+1);
        qcList[i]=qcId;
    }

    return kvalobs::KvDataSubscriberInfo(sid, qcList);

}


bool 
KvSubscriberCollection::
addDataFromDb(
        const std::string              &subid,
        const std::string              &cref,
        const std::string              &statusid,
        const std::vector<std::string> &qcIdList,
        const std::vector<std::string> &stationList)
{
    CorbaHelper::CorbaApp *cApp=CorbaHelper::CorbaApp::getCorbaApp();
    CKvalObs::CService::kvDataSubscriber_ptr       cptrSub;
    CORBA::Object_var                              cptrCO;
    std::vector<std::string>::const_iterator  it;
    KvDataSubscriberPtr                            subPtr;
    KvDataSubscriber                               *p;
    long                                           stationid;
    int                                            nStations=0;

    if(!cApp){
        LOGFATAL("Cant obtain a referance to CorbaHelper::CorbaApp.");
        return false;
    }

    cptrCO=cApp->corbaRef(cref);

    if(CORBA::is_nil(cptrCO)){
        LOGERROR("CORBA: null refernce!");
        return false;
    }

    try{
        cptrSub=CKvalObs::CService::kvDataSubscriber::_narrow(cptrCO);

        if(CORBA::is_nil(cptrSub)){
            LOGERROR("CORBA: null refernce! Wrong type.");
            CORBA::release(cptrSub);
            return false;
        }
    }
    catch(...){
        LOGERROR("EXCEPTION: CORBA: null refernce! Wrong type.");
        return false;
    }


    kvalobs::KvDataSubscriberInfo info=createKvDataInfo(statusid, qcIdList);


    try{
        p=new KvDataSubscriber(info, cptrSub);
        subPtr.reset(p);
    }
    catch(...){
        CORBA::release(cptrSub);
        LOGERROR("OUT OF MEMMORY: KvSubscriberCollection::addDataFromFileFile!");
        return false;
    }

    subPtr->subscriberid(static_cast<std::string>(subid));

    if(stationList.empty()){
        allStationsDataSubscribers.push_back(subPtr);
        subscribers_.insert(subPtr);
        return true;
    }

    for(it=stationList.begin();
            it!=stationList.end();
            it++){

        try{
            stationid=boost::lexical_cast<long>(*it);
        }
        catch(boost::bad_lexical_cast &ex){
            LOGERROR("NOT A NUMBER: " << *it);
            continue;
        }
        catch(...){
            LOGERROR("UNKNOWN Exception: trying stationid=boost::numeric_cast<long>(*it)!");
            continue;
        }

        try{
            stationDataSubscribers.insert(make_pair(stationid, subPtr));
            nStations++;
        }
        catch(...){
            LOGWARN("OUT OF MEMMORY: cant add data subscriber!");
        }
    }

    if(nStations>0){
        subscribers_.insert(subPtr);
        return true;
    }

    return false;
}

bool 
KvSubscriberCollection::
addKvHintFromDb(const std::string &subid,
        const std::string &cref)
{
    CorbaHelper::CorbaApp *cApp=CorbaHelper::CorbaApp::getCorbaApp();
    CKvalObs::CService::kvHintSubscriber_ptr       cptrSub;
    CORBA::Object_var                              cptrCO;
    std::vector<std::string>::const_iterator  it;
    KvHintSubscriberPtr                            subPtr;
    KvHintSubscriber                               *p;

    if(!cApp){
        LOGFATAL("Cant obtain a referance to CorbaHelper::CorbaApp.");
        return false;
    }

    cptrCO=cApp->corbaRef(cref);

    if(CORBA::is_nil(cptrCO)){
        LOGERROR("CORBA: null refernce!");
        return false;
    }

    try{
        cptrSub=CKvalObs::CService::kvHintSubscriber::_narrow(cptrCO);

        if(CORBA::is_nil(cptrSub)){
            LOGERROR("CORBA: null refernce! Wrong type.");
            CORBA::release(cptrSub);
            return false;
        }
    }
    catch(...){
        LOGERROR("EXCEPTION: CORBA: null refernce! Wrong type.");
        return false;
    }

    try{
        p=new KvHintSubscriber(cptrSub);
        subPtr.reset(p);
    }
    catch(...){
        CORBA::release(cptrSub);
        LOGERROR("OUT OF MEMMORY: KvSubscriberCollection::addKvHintFromFileFile!");
        return false;
    }

    subPtr->subscriberid(static_cast<std::string>(subid));

    hintSubscriberList.push_back(subPtr);


    return true;
}


bool 
KvSubscriberCollection::
removeSubscriberFromDb(const std::string &subscriberid)
{
    dnmi::db::Connection *con = getDbConnection();

    if( ! con ) {
         LOGERROR("KvSubscriberCollection: No db connection. Cant read subscribers from the database.");
         return false;
    }

    KeyValSubscriberTransaction tran( subscriberid, KeyValSubscriberTransaction::DELETE_SUBSCRIBER );

    try{
        con->perform( tran );
        if( ! tran.isOk() ) {
            LOGERROR("KvSubscriberCollection: can't remove <" << subscriberid << "> from the database."
                        << endl << "Reason: " << tran.getMsg()  );
            return false;
        } else {
            LOGDEBUG("KvSubscriberCollection: Removed <" << subscriberid << "> from the database."
                      << endl << "Msg: " << tran.getMsg()  );
        }
        return true;
    }
    catch (const std::exception &e) {
        LOGERROR("KvSubscriberCollection: can't remove <" << subscriberid << "> from the database."
                << endl << "Reason: (exception): " << e.what() << endl << "Msg: " << tran.getMsg()  );
    }
    return true;
}



std::ostream& operator<<(std::ostream& os, 
        const KvSubscriberCollection &c)
{
    boost::mutex::scoped_lock lock(const_cast<KvSubscriberCollection&>(c).mutex);
    os << "\nKvSubscriberCollection: \n";
    os << "  allStationDataNotifySubscribers: Antall: "
            << c.allStationsDataNotifySubscribers.size()
            << endl;

    list<KvDataNotifySubscriberPtr>::const_iterator it;

    it=c.allStationsDataNotifySubscribers.begin();


    for(;it!=c.allStationsDataNotifySubscribers.end(); it++){
        os << "      subscriberid: " << (*it)->subscriberid()
               << "  " << (*it)->subscriberInfo() << endl;
    }

    os << "\n  stationsDataNotifySubscribers: Antall: "
            << c.stationDataNotifySubscribers.size()
            << endl;

    multimap<long, KvDataNotifySubscriberPtr>::const_iterator it1, itl, itu;

    it1=c.stationDataNotifySubscribers.begin();

    for(;it1!=c.stationDataNotifySubscribers.end(); it1++){
        os << "   stationid: " << it1->first << endl;

        itl=c.stationDataNotifySubscribers.lower_bound(it1->first);

        if(itl!=c.stationDataNotifySubscribers.end()){
            itu=c.stationDataNotifySubscribers.upper_bound(it1->first);

            for(;itl!=itu; itl++){
                os << "      subscriberid: " << itl->second->subscriberid()
		         << "  " <<  itl->second->subscriberInfo() << endl;
            }
        }
    }

    return os;
}

