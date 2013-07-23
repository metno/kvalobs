/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvDataIteratorImpl.cc,v 1.5.6.15 2007/09/27 09:02:39 paule Exp $                                                       

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
#include <milog/milog.h>
#include <miutil/timeconvert.h>
#include <cstring>
#include "kvDataIteratorImpl.h"
#include <kvalobs/kvDbGate.h>


using namespace std;
using namespace miutil;
using namespace kvalobs;
using namespace CKvalObs::CService;
using namespace milog;

DataIteratorImpl::DataIteratorImpl(dnmi::db::Connection *dbCon_,
                                   WhichDataList *whichData_,
                                   ServiceApp &app_):
                                   dbCon(dbCon_),
                                   whichData(whichData_),
                                   iData(0),
                                   startTimeOfGetData(boost::posix_time::second_clock::universal_time()),app(app_)
{
}

DataIteratorImpl::~DataIteratorImpl()
{

   LOGDEBUG("DTOR: DataIteratorImpl::~DataIteratorImpl...\n");
   boost::mutex::scoped_lock lock( mutex );

   if(whichData)
      delete whichData;

   {
	   boost::mutex::scoped_lock lock( mutex );

	   if(dbCon)
		   app.releaseDbConnection(dbCon);
   }

   LOGDEBUG("DTOR: DataIteratorImpl::~DataIteratorImpl ... 1 ...\n");
}


void  
DataIteratorImpl::destroy()
{
   // We deactivate and release th dbConnection here. The cleanup thread will release the resources
   // and remove it from the reaperObjList.
   boost::mutex::scoped_lock lock( mutex );

   LOGDEBUG("DataIteratorImpl::destroy: called!\n");

   if(dbCon) {
      app.releaseDbConnection(dbCon);
      dbCon = 0;
   }

   deactivate();

   LOGDEBUG("DataIteratorImpl::destroy: leaving!\n");
}


/**
* next, get called too all data specified in the WhichData is retrived from
* the database. Each call on 'next' get at most 12 hours of data. The stations
* is proccessed in the order they are in the WhichData list.
*
* COMMENT
* The retrival from the database is done with multiple select statement, where
* each select retrives an 12 hours period. It is posible we should have used a
* coursor in a transaction instead. That would have given a snapshot of the
* data at the time of the call of 'getData' and would maybe performe better.
*
* COMMENT 2
* To get better performance we always allocate a more elements in
* the lists. The diffrent list is increased with the OBSLIST_DELTA,
* DATALIST_DELTA and TEXTDATALIST_DELTA.
*
* When we are finished whit each list we must reset the length to the
* number of elements inserted.
*/

CORBA::Boolean  
DataIteratorImpl::next(CKvalObs::CService::ObsDataList_out obsDataList)
{
   const int              OBSLIST_DELTA=12;
   const int              DATALIST_DELTA=20;
   const int              TEXTDATALIST_DELTA=10;
   list<kvData>               dataList;
   list<kvData>::iterator     it;
   list<kvTextData>           textDataList;
   list<kvTextData>::iterator tit;
   boost::posix_time::ptime                 thisTime;
   CORBA::Long            obsi=0;
   CORBA::Long            datai=0;
   char                   *sTmp;
   char                   buf[20];
   CORBA::Long            tmpIData;
   bool                   active;
   //ObsDataList          obsDataList;

   boost::mutex::scoped_lock lock( mutex );

   IsRunningHelper isRunning(*this, active );

   LogContext context("service/DataIterator");

   LOGDEBUG("next: called ..." );

   if( !dbCon ) {
	   LOGERROR( "next:  No db connection (returning false)." );
	   return false;
   }

   //Check if we are deactivated. If so just return false.
   if( ! active ) {
      LOGDEBUG( "next: deactivated ( returning false)");
      return false;
   }

   obsDataList =new CKvalObs::CService::ObsDataList(OBSLIST_DELTA);

   do{
      tmpIData=iData;

      if(iData>=whichData->length()){
         LOGDEBUG("next: End of data reached (return false)!");
         return false;
      }

      try{
         //findData, increments iData when necesary!
         if(!findData(dataList, textDataList, (*whichData)[iData])){
            LOGDEBUG("next: Cant find data (return false)!");
            //CODE:
            //We have a problem with the connection to the database.
            //We return false. false is used to mark the end of stream. So
            //the caller sees this as an end of stream, that is wrong. We
            //may add an exception here later to tell the caller that we
            //had problems. Anyway, the caller must react the same way and
            //call destroy on the iterator.
            return false;
         }
      }
      catch(InvalidWhichData &ex){
         LOGDEBUG("next: EXCEPTION: " << endl <<  ex.what() << endl);
         return false;
      }
      catch(...){
         LOGDEBUG("next: UNKNOWN EXCEPTION: \n");
         return false;
      }

   }while(dataList.empty() && textDataList.empty());

   it=filterData(dataList.begin(), dataList,(*whichData)[tmpIData]);

   if(it!=dataList.end()){
      obsDataList->length(OBSLIST_DELTA);
      thisTime=it->obstime();
      (*obsDataList)[0].stationid=it->stationID();
   }

   obsi=0;
   datai=0;

   while(it!=dataList.end()){
      if(it->obstime()!=thisTime){
         //New record

         thisTime=it->obstime();

         //Set the length to the number of elements we
         //have put in the list.
         //Note that when we get here the datai is incremented
         //by one.
         (*obsDataList)[obsi].dataList.length(datai);

         obsi++;
         datai=0;
         //Do we need to extend the obslist.
         if(obsi>=obsDataList->length())
            obsDataList->length(obsDataList->length()+OBSLIST_DELTA);

         (*obsDataList)[obsi].stationid=it->stationID();
         LOGDEBUG("next: obsDataList[" << obsi-1 << "].dataList.length()="
                  << (*obsDataList)[obsi-1].dataList.length() << endl);
      }

      //Check if we need to extend the list.
      if((*obsDataList)[obsi].dataList.length()<=datai)
         (*obsDataList)[obsi].dataList.length(
               (*obsDataList)[obsi].dataList.length()+DATALIST_DELTA
         );

      (*obsDataList)[obsi].dataList[datai].stationID=it->stationID();
      (*obsDataList)[obsi].dataList[datai].obstime=to_kvalobs_string_without_decimal_secound(it->obstime()).c_str();
      (*obsDataList)[obsi].dataList[datai].original=it->original();
      (*obsDataList)[obsi].dataList[datai].paramID=it->paramID();
      (*obsDataList)[obsi].dataList[datai].tbtime=to_kvalobs_string_without_decimal_secound(it->tbtime()).c_str();
      (*obsDataList)[obsi].dataList[datai].typeID_=it->typeID();

      sprintf(buf, "%d", it->sensor());
      sTmp=CORBA::string_alloc(strlen(buf)+1);

      if(sTmp){
         strcpy(sTmp, buf);
         (*obsDataList)[obsi].dataList[datai].sensor=sTmp;
      }

      (*obsDataList)[obsi].dataList[datai].level=it->level();
      (*obsDataList)[obsi].dataList[datai].corrected=it->corrected();
      (*obsDataList)[obsi].dataList[datai].controlinfo=it->controlinfo().flagstring().c_str();
      (*obsDataList)[obsi].dataList[datai].useinfo=it->useinfo().flagstring().c_str();
      (*obsDataList)[obsi].dataList[datai].cfailed=it->cfailed().c_str();

      datai++;

      //Move to the next data in dataList.
      it++;
      it=filterData(it, dataList, (*whichData)[tmpIData]);
   }

   if(datai>0){ //datai is 0 only when there is no data.
      //Set the length to the number of elements in the lists.
      //Remeber to set the length of the datalist for the last obsi index.
      (*obsDataList)[obsi].dataList.length(datai);
      obsDataList->length(obsi+1);
   }

   LOGDEBUG("next: obsDataList->length()="
         << obsDataList->length() << endl);

   ostringstream ds;
   for( tit=textDataList.begin(); tit != textDataList.end(); ++tit )
      ds << tit->stationID() << "," << tit->obstime() << "," << tit->typeID() << ","
      << tit->paramID() << "," << tit->original() << endl;

   LOGDEBUG( "text_data: " << endl << ds.str() );

   tit=textDataList.begin();

   if(tit==textDataList.end()){
      LOGDEBUG("No <textdata>!");
      return true;
   }



   thisTime=tit->obstime();
   datai=0;
   TextDataElemList textData(TEXTDATALIST_DELTA);

   while(tit!=textDataList.end()){
      if(tit->obstime()!=thisTime){
         thisTime = tit->obstime();
         textData.length(datai);
         datai=0;
         insertTextData(obsDataList, textData);
      }

      if(textData.length()<=datai)
         textData.length(textData.length()+TEXTDATALIST_DELTA);

      textData[datai].stationID=tit->stationID();
      textData[datai].obstime=to_kvalobs_string_without_decimal_secound(tit->obstime()).c_str();
      textData[datai].original=tit->original().c_str();
      textData[datai].paramID=tit->paramID();
      textData[datai].tbtime=to_kvalobs_string_without_decimal_secound(tit->tbtime()).c_str();
      textData[datai].typeID_=tit->typeID();

      tit++;
      datai++;
   }

   textData.length(datai);
   insertTextData(obsDataList, textData);

   return true;
}

list<kvData>::iterator
DataIteratorImpl::filterData(list<kvData>::iterator start,
                             list<kvData> &dataList,
                             const CKvalObs::CService::WhichData &wData)
{
   for(;start!=dataList.end(); start++){
      if(wData.status==CKvalObs::CService::All){
         return start;
      }else if(wData.status==CKvalObs::CService::OnlyOk
            && start->cfailed().empty()){
         return start;
      }else if(wData.status==CKvalObs::CService::OnlyFailed &&
            !start->cfailed().empty()){
         return start;
      }
   }

   return dataList.end();
}


/**
* findData does the jobb with the retrival of the data from the database.
* It is called from next.
*
* The function chuncs the data in 12 hours periods.
*
* The function also increment the iData variable, when the next
* station in the WhichDataList shall be proccessed.
*/
bool
DataIteratorImpl::findData(list<kvData> &data, 
                           std::list<kvalobs::kvTextData> &textData,
                           const CKvalObs::CService::WhichData &wData)
{
   kvDbGate gate(dbCon);
   boost::posix_time::ptime stime;
   boost::posix_time::ptime etime;
   boost::posix_time::ptime tmpTime;
   bool     ret=true;

   LogContext context("findData");

   textData.clear();
   data.clear();

   LOGDEBUG("stationid: " << wData.stationid << " currentEndTime: "
            << currentEndTime << " endTime: " << endTime << " iData: " <<
            iData);

   if(currentEndTime.is_not_a_date_time()){
      stime = boost::posix_time::time_from_string_nothrow(std::string(wData.fromObsTime));

      if( strlen( wData.toObsTime ) > 0 )
          endTime = boost::posix_time::time_from_string_nothrow(std::string(wData.toObsTime));

      if(stime.is_not_a_date_time() || endTime.is_not_a_date_time()){
         if(stime.is_not_a_date_time()){
            ostringstream os;
            os << "Invalid timespec (fromObsTime): ";

            if(wData.fromObsTime)
               os <<  wData.fromObsTime;
            else
               os << "(NULL POINTER)";

            os << "!";

            throw InvalidWhichData(os.str());
         }

         if(endTime.is_not_a_date_time()){
            endTime=startTimeOfGetData;
         }
      }
   }else{
      stime=currentEndTime;
   }

   tmpTime=stime;
   tmpTime += boost::posix_time::hours(12);

   if(tmpTime<endTime){
      currentEndTime=tmpTime;
      etime=currentEndTime;

      //We adjust the etime with one second, so that in effect
      //we get data which is in the period [stime,currendEndTime>. This since
      //the  kvQueries::selectData get the data in the period [stime,etime], but
      //we will not have the endpoint twice.
      etime -= boost::posix_time::seconds(1);
   }else{
      currentEndTime=boost::posix_time::ptime(); //set currentEndTime to not_a_date_time.
      etime=endTime;
      iData++;
   }


   LOGDEBUG("select(" << wData.stationid << ", " << stime << ", " << etime);

   if(gate.select(data, kvQueries::selectData(wData.stationid, stime, etime))){
      LOGDEBUG("data: nElements=" << data.size());
   }else{
      LOGERROR("Error fetching <data>! stationid: " << wData.stationid <<
               " timeinterval:" << stime << " - " << etime );
      ret=false;
   }

   if(gate.select(textData,
                  kvQueries::selectTextData(wData.stationid,
                                            stime,
                                            etime))){
      LOGDEBUG("textData: nElements=" << textData.size());
   }else{
      LOGWARN("Error fetching <textData>! stationid: " << wData.stationid <<
              " timeinterval:" << stime << " - " << etime );
   }


   LOGDEBUG("DataIteratorImpl::findData: return " << (ret?"TRUE":"FALSE")<<endl);
   return ret;
}


void
DataIteratorImpl::  
insertTextData(CKvalObs::CService::ObsDataList *obsDataList, 
               const TextDataElemList           &textData)
{
   CORBA::Long i=0;
   boost::posix_time::ptime  obsTime;
   boost::posix_time::ptime  textTime;

   if(textData.length()==0)
      return;

   textTime=boost::posix_time::time_from_string_nothrow(std::string(textData[0].obstime));

   if(textTime.is_not_a_date_time())//Should never happend
      return;

   for(i=0; i<obsDataList->length(); i++){
      if((*obsDataList)[i].dataList.length()==0) //Should never happend.
         continue;

      obsTime=boost::posix_time::time_from_string_nothrow(std::string((*obsDataList)[i].dataList[0].obstime));

      if(obsTime.is_not_a_date_time()) //Should never happend.
         continue;

      if(obsTime>=textTime)
         break;
   }

   if(obsTime==textTime){
      (*obsDataList)[i].textDataList=textData;
      return;
   }

   //We have an textData list that does not have an ordinary dataList.
   //This should be strange, but it is not invalid

   obsDataList->length(obsDataList->length()+1);

   for(CORBA::Long idx=obsDataList->length()-1; idx > i; idx--)
      (*obsDataList)[idx]=(*obsDataList)[idx-1];

   (*obsDataList)[i].stationid=textData[0].stationID;
   (*obsDataList)[i].dataList.length(0);
   (*obsDataList)[i].textDataList=textData;
}


/*
 * TODO: Implement the cleanup, ie move the cleanup of the database object and whichData
 * from the deactivate method to this method. Remember to add a call to cleanup in 
 * the method ServiceApp::cleanUpReaperObj()
 */ 
void 
DataIteratorImpl::
cleanUp()
{
   boost::mutex::scoped_lock lock( mutex );

   if( dbCon ) {
      app.releaseDbConnection(dbCon);
      dbCon = 0;
   }

   delete whichData;

   whichData=0;
}
