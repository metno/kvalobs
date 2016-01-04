/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: DataReceiverSimpleKro.h,v 1.1.6.2 2007/09/27 09:02:20 paule Exp $                                                       

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
#ifndef __kv2kro_h__
#define __kv2kro_h__

#include <kvcpp/kvservicetypes.h>
#include <kvcpp/KvAppSimple.h>
#include <kvcpp/KvGetDataReceiver.h>

using namespace kvalobs;
using namespace kvservice;

class MyGetDataReceiver : public kvservice::KvGetDataReceiver {
 public:

  /**
   * next, this function is called for every data set!
   *
   * \datalist the data.
   * \return true if we shall continue. False if you want too
   *         stop retriving data from kvalobs.
   */
  MyGetDataReceiver(const std::string& fileFromTime,
                    const std::string& fileObsDataList);
  MyGetDataReceiver();
  bool next(KvObsDataList &datalist);

 private:
  std::string m_fileFromTime;
  std::string m_fileObsDataList;

};

class kv2kro : public kvservice::KvAppSimple {

 protected:
  std::string subidKvData;
  std::string subidKvHint;

 public:
  kv2kro(int argn, char **argv, miutil::conf::ConfSection *conf);

  void onKvHintEvent(bool up);
  void onKvDataNotifyEvent(KvWhatListPtr what);
  void onShutdown();
  void onKvDataEvent(KvObsDataListPtr data);
  static void printObsDataList(KvObsDataList& dataList);

 private:
  std::string dataid;
  std::string notifyid;
  std::string hintid;
  std::string pidfile;
  std::string kvPath_;
  std::string m_fileFromTime;
  std::string m_fileObsDataList;

  miutil::miTime lastFromTime(std::string& filename);
  static void storeToFile(std::string& filename, miutil::miTime& toTime);

  int startup();

  void createPidFile(const std::string &progname);
  void deletePidFile();

};

#endif

