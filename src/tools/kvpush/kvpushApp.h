/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvpushApp.h,v 1.1.2.2 2007/09/27 09:02:48 paule Exp $                                                       

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
#ifndef __kvpushApp_h__
#define __kvpushApp_h__

#include <db/db.h>
#include <db/dbdrivermgr.h>
#include <kvalobs/kvapp.h>
#include <puTools/miTime>
#include <list>
#include <string>
#include <exception>
#include "DataTblView.h"

typedef std::list<DataTblView>            DataTblViewList;
typedef DataTblViewList::iterator        IDataTblViewList;
typedef DataTblViewList::const_iterator CIDataTblViewList;


class GetOptEx : public std::exception{
	std::string msg;
	
	public:
		explicit GetOptEx(const char *msg_):msg(msg_){}
		explicit GetOptEx(const std::string &msg_):msg(msg_){}
		virtual ~GetOptEx()throw(){}
		const char* what() const throw() { return msg.c_str();}
};
		
struct Options{
	typedef std::list<int>        List;
	typedef List::iterator        IList;
	typedef List::const_iterator CIList;
	
	Options()
		:help(false), totime(miutil::miTime::nowTime()){
	}

	bool           help;
	List           stations;
	List           typeids;
	miutil::miTime fromtime;
	miutil::miTime totime;
};

std::ostream& operator<<(std::ostream &ost, const Options &opt);

class KvPushApp : public KvApp 
{
	dnmi::db::DriverManager dbMgr;	
	std::string             dbConnect;
	std::string             dbDriverId;
	
	/**
	 * \exception GetOptEx
	 */
	void readIntList(const char *opt, Options::List &list);
	
	/**
	 * \exception GetOptEx
	 */
	void readDate(const char *opt, miutil::miTime &date);
	
	/**
	 * Replace a substring in a string with new content.
	 * 
	 * \param src string to replace in.
	 * \param what replace this substring in \a src.
	 * \param whith the new content to replace \a what with in the \a src.
	 */	
	void replace(std::string &src, const std::string &what, const std::string &with);
	
	bool selectAllTypeids(const Options::List &stationList,  
								  const miutil::miTime &fromtime,
								  const miutil::miTime &totime,
								  dnmi::db::Connection *con);
								  
	bool selectAllStations(const Options::List &typeidList,  
								   const miutil::miTime &fromtime,
								   const miutil::miTime &totime,
								   dnmi::db::Connection *con);
								   
   bool selectFrom(const Options::List &stationidList,
   	              const Options::List &typeidList,  
						  const miutil::miTime &fromtime,
						  const miutil::miTime &totime,
						  dnmi::db::Connection *con);
						  
   bool selectData(const std::string &query,	
   					 const miutil::miTime &fromtime,
						 const miutil::miTime &totime,
						 dnmi::db::Connection *con);
						 
	bool updateWorkque(dnmi::db::Result *res, dnmi::db::Connection *con);
		
		
public:
	KvPushApp(int argn, char **argv, const char *options[0][2]=0);
	~KvPushApp();

	bool sendSignalToManager(int sid, int tid, const miutil::miTime &obstime);
		
	/**
	 * \exception GetOptEx
	 */
	void getOpt(int argn, char **argv, Options &opt);

	dnmi::db::Connection *getNewDbConnection();
	void                 releaseDbConnection(dnmi::db::Connection *con);
	
	bool selectDataAndUpdateWorkque(const Options &opt);
};

#endif
