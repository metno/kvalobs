/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: DataReceiverSimpleKro.cc,v 1.2.6.4 2007/09/27 09:02:20 paule Exp $                                                       

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
//#include <KvAppSimple.h>
#include <DataReceiverSimpleKro.h>
//#include <kv2kro.h>
#include <iostream>
#include <kvcpp/KvAppSimple.h>
#include <kvalobs/kvapp.h>
#include <list>
#include <kvalobs/kvStation.h>
#include <fstream>
#include <stdio.h>
#include <kvalobs/kvPath.h>
#include <milog/milog.h>

using namespace std;
using namespace kvservice;



    /**
     * next, this function is called for every data set!
     *
     * \datalist the data.
     * \return true if we shall continue. False if you want too
     *         stop retriving data from kvalobs.
     */

MyGetDataReceiver::MyGetDataReceiver(const std::string& fileFromTime, const std::string& fileObsDataList ){
    m_fileFromTime=fileFromTime;
    m_fileObsDataList=fileObsDataList;
}

MyGetDataReceiver::MyGetDataReceiver(){

}


bool MyGetDataReceiver::next(KvObsDataList &datalist){
  cerr << "next: data from kvalobs\n";
  
  //printObsDataList   ( KvObsDataList&   dataList );
  
  //it er en iterator. Ireratorer brukes for � traversere 
  //en container, i dette tilfellet KvDataList. KvDataList 
  //er en liste av kvData element.
  IKvObsDataList it;
  
  for(it=datalist.begin();    //sett it til starten av listen
      it!=datalist.end();     //Er vi kommet til slutten
      it++){               //Flytt it til neste element i listen.
    
    //cerr skriver til standard error stream.
    //Skriv ut stationid, paramid, corrected og useinfo fra 
    //kvData elementet som it peker p�.
    if(it->dataList().size()>0){
      cout << "stationID: " << it->stationid() 
	   << " obstime: " << it->dataList().front().obstime()
	   << " parameters: " << it->dataList().size() << endl;
    }
    
    kv2kro::printObsDataList( datalist );
    cerr << "--------------------------\n";

  }
}


/********************************/ 
kv2kro::kv2kro(int argn, char **argv, miutil::conf::ConfSection *conf)
  :KvAppSimple(argn, argv, conf)
{
  kvPath_=kvPath();

  //subidKvHint = this->subscribeKvHint();
  //if(subidKvHint.empty()){
  //  CERR("ERROR: KvDataView::KvDataView: can't subscribe on kvHint!\n");
  //}else{
  //  CERR("INFO: KvDataView::KvDataView: subscribed on kvHint!\n"
  //       << "  subscriber id (" << subidKvHint << ")!\n");
  //}

  //KvDataSubscribeInfoHelper info;

  //subidKvData = this->subscribeData(info);

  //if(subidKvData.empty()){
  //  CERR("ERROR: KvDataView::KvDataView: can't subscribe on data!\n");
  //}else{
  //  CERR("INFO: KvDataView::KvDataView: subscribed on data!\n"
  //       << "  subscriber id (" << subidKvData << ")!\n");
  //}


  if(startup()>0){
      cerr<<"ERROR in the startup of the program kv2kro"<<endl;
  }

  createPidFile("DataReceiverSimpleKro");
}

/********************************/ 
void  kv2kro::onKvHintEvent(bool up){
    if(up){
      cerr << "KvUpEvent received!" << endl;
    }else{
      cerr << "KvDownEvent received!" << endl;
    }
  }

void  kv2kro::onKvDataNotifyEvent(KvWhatListPtr what){
    cerr << "KvDataNotifyEvent received!" << endl;
  }

 void  kv2kro::onShutdown(){
    cerr << "Terminating .....!" << endl;
  }
/********************************/    
void kv2kro::onKvDataEvent(KvObsDataListPtr data){
    cerr << "KvDataEvent received!" << endl;

    //it er en iterator. Ireratorer brukes for � traversere 
    //en container, i dette tilfellet KvDataList. KvDataList 
    //er en liste av kvData element.
    IKvObsDataList it=data->begin();


    cerr << "newData: data from kvalobs\n";
    
    
    for(it=data->begin();    //sett it til starten av listen
	it!=data->end();     //Er vi kommet til slutten
	it++){               //Flytt it til neste element i listen.
	
	//cerr skriver til standard error stream.
	//Skriv ut stationid, paramid, corrected og useinfo fra 
	//kvData elementet som it peker p�.
      
      if(it->dataList().size()>0)
	cout << "stationID: " << it->stationid() 
	     << " obstime: " << it->dataList().front().obstime()
	     << " parameters: " << it->dataList().size() << endl;
    }

    cerr << "Inittializing success!" << endl;
    printObsDataList( *data );

    cerr << "--------------------------\n";
    return;
}



/********************************/
int kv2kro::startup(){
  notifyid=subscribeDataNotify(KvDataSubscribeInfoHelper());
    
    if(notifyid.empty()){
      cerr << "Cant subscribe to KvDataNotify!" << endl;
      return false;
    }else{
      cerr << "Subscribe on KvDataNotify!" << endl;
    }

    dataid=subscribeData(KvDataSubscribeInfoHelper());

    if(dataid.empty()){
      cerr << "Cant subscribe to KvData!" << endl;
      return false;
    }else{
      cerr << "Subscribe on KvData!" << endl;
    }

    hintid=subscribeKvHint();

    if(hintid.empty()){
      cerr << "Cant subscribe to KvHint!" << endl;
      return false;
    }else{
      cerr << "Subscribe on KvHint!" << endl;
    }

   

  

  miutil::miTime toTime = miutil::miTime::nowTime();
  miutil::miTime fromTime = lastFromTime(m_fileFromTime);

  cout<<"toTime= "<<toTime.isoTime()<<endl;
  cout<<"fromTime= "<<fromTime.isoTime()<<endl;

  WhichDataHelper whichData;
  whichData.addStation(0, fromTime);

  MyGetDataReceiver dataReceiver;

  if(!this->getKvData(dataReceiver, whichData)){
    cerr << "Cant connect to kvalobs!";
    return 1;
  }

  
  return 0;
}



/*****************************************/
void kv2kro::printObsDataList( KvObsDataList&   dataList ){
  
  string fileFromTime = "fromTime";
  string datadir=".";
  string fileDataList = datadir + "/" + "obsDataList";
  string fileTextDataList= datadir + "/" + "textDataList";
  
  miutil::miTime toTime = miutil::miTime::nowTime();
  
  string filename = fileDataList + "." + toTime.isoDate() + "_" 
                    + toTime.isoClock();
  string pfilename = filename + ".new";

  string textfilename = fileTextDataList + "." + toTime.isoDate() + "_" 
                    + toTime.isoClock();
  string ptextfilename = textfilename + ".new";

  ofstream fout( pfilename.c_str() );
  //ofstream tfout( ptextfilename.c_str() );
  ofstream tfout;


  bool readText=false;
 
  for(IKvObsDataList dit=dataList.begin();
      dit!=dataList.end();
      dit++){
    KvObsData::kvDataList::iterator it=dit->dataList().begin();
    
    if(dit->dataList().size()>0){
      miutil::miTime obstime= it->obstime();
      miutil::miTime tbtime= it->tbtime();

      if( obstime < toTime )
	toTime = obstime;
      if( tbtime  < toTime )
	toTime = tbtime;
      
          
      for(;
	  it!=dit->dataList().end();
	  it++){
	char c= it->sensor();
	fout << it->stationID() << "|" << to_kvalobs_string(it->obstime()) << "|" << it->original()
	   << "|" << it->paramID()<< "|" <<  to_kvalobs_string(it->tbtime()) << "|"
	   << it->typeID() << "|" << c << "|" << it->level() << "|" 
	   << it->corrected() << "|" << it->controlinfo().flagstring() 
	   << "|" << it->useinfo().flagstring() << "|" << it->cfailed() 
	   << endl;
      }
    }


    KvObsData::kvTextDataList::iterator i=dit->textDataList().begin();
    if(dit->textDataList().size()>0){

      if( ! tfout.is_open() ){
	readText =true;
        tfout.open( ptextfilename.c_str() );
      }
      
      miutil::miTime obstime= i->obstime();
      miutil::miTime tbtime= i->tbtime();

      if( obstime < toTime )
	toTime = obstime;
      if( tbtime  < toTime )
	toTime = tbtime;
 
       
      for(;
	  i !=dit->textDataList().end();
	  i ++){
	 tfout << i->stationID() << "|" << to_kvalobs_string(i->obstime()) << "|" << i->original();
	 tfout << "|" << i->paramID()<< "|" <<  to_kvalobs_string(i->tbtime()) << "|";
	 tfout << i->typeID() << endl;
      }
    }

  }

  fout.close();
  tfout.close();
  
  if( rename(  pfilename.c_str(), filename.c_str() )){
    cerr<<"can not rename file "<<pfilename<<endl;
  }

  if( readText && rename(  ptextfilename.c_str(), textfilename.c_str() )){
    cerr<<"can not rename file"<<ptextfilename<<endl;
  }
  
  string sysarg = "./obsInsert " + filename;
  system(sysarg.c_str());

  if( readText ){
      sysarg = "./obsInsert " + textfilename;
      system(sysarg.c_str());
  }
  
  toTime.addHour(-6);
  storeToFile( fileFromTime, toTime );
  cout<<"**  printObsDataList is finished **"<<endl;
}



/*****************************************/
void kv2kro::storeToFile(  string& filename, miutil::miTime& toTime ){

 ofstream fout(filename.c_str());

 fout <<toTime.isoTime();

 //fout << setw(4) << setfill('0') << toTime.year()  << ':'
 //   << setw(2) << setfill('0') << toTime.month() << ':'
 //   << setw(2) << setfill('0') << toTime.day()   << ':'
 //   << toTime.isoClock();

  //int hour() const
  //int min() const
  //int sec() const

  return;
}

miutil::miTime kv2kro::lastFromTime(string& filename){

    miutil::miTime time;
    miutil::std::string str;

    cerr<<"filename= "<<filename<<endl;

    ifstream fin( filename.c_str() );
    if(!fin){
        cerr<<"the file "<<filename<<" does not exist"<<endl;
        time = miutil::miTime::nowTime();
        time.addDay(-1);
        cerr<<"return lastFromTime: no file"<<endl;
        return time;
    }


  //fin.exceptions(ios_base::badbit|ios_base::failbit|ios_base::eofbit);
  fin.exceptions(ios_base::badbit);

  try{
    cerr<<"try lastFromTime"<<endl;

    while(getline(fin,str)){
          str.trim();
          cerr<<"str= "<<str<<endl;
          if(!str.empty()){
              if( time.isValid(str) ){
                  time.setTime(str);
                  if(time.undef()){
                      cerr<<"time is undef"<<endl;
                      time = miutil::miTime::nowTime();
                      time.addDay(-1);
                  }
              }else{
                  cerr<<"time is not valid"<<endl;
                  time = miutil::miTime::nowTime();
                  time.addDay(-1);
              }
          }
    }

  }catch( std::ios_base::failure ){
     cerr<<"catch( std::ios_base::failure )"<<endl;
     time = miutil::miTime::nowTime();
     time.addDay(-1);

  }catch(...){
      cerr<<"catch(...)"<<endl;
      time = miutil::miTime::nowTime();
      time.addDay(-1);
  }

  cerr<<"return lastFromTime"<<endl;
  return time;

}



/*****************************************/
void
kv2kro::createPidFile(const std::string &progname)
{
  FILE *fd;

  pidfile=kvPath_;
  pidfile+="var/run/";
  pidfile+=progname;
  pidfile+=".pid";

  LOGINFO("Writing pid to file <" << pidfile << ">!");

  fd=fopen(pidfile.c_str(), "w");

  if(!fd){
      LOGWARN("Can't create pidfile <" << pidfile << ">!\n");
      pidfile.erase();
      return;
  }

  fprintf(fd, "%ld\n", (long)getpid());
  fclose(fd);
}


void
kv2kro::deletePidFile()
{
  if(pidfile.empty())
    return;

  LOGINFO("Deleting pidfile <" << pidfile << ">!");

  unlink(pidfile.c_str());
}




int
main(int argn, char **argv){

  string cstr;
  string path="";

  if( getenv("KVALOBS")){
    path= string(getenv("KVALOBS"));
    path += "/etc/";
  }

  path += "kv2kro.conf";

  kv2kro  app(argn, argv, KvAppSimple::readConf(path));
  app.run();
}
