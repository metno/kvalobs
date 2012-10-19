/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: ConnectionCache.h,v 1.3.2.2 2007/09/27 09:02:16 paule Exp $                                                       

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
#ifndef __ConnectionCache_h__
#define __ConnectionCache_h__

//#include <boost/thread/thread.hpp>
#include <boost/thread.hpp>
#include <boost/thread/condition.hpp>
#include <map>
#include <kvdb/dbdrivermgr.h>

/**
 * \addtogroup kvDatainputd
 * @{
 */

/**
 * \brief A class to be used as a connection cache.
 *
 * It maintains a list of dnmi::db::Connection that is open and in use.
 * When a thread is not using a connection anymore it is marked as
 * free and can be reused later.
 */
class ConnectionCache
{
  typedef std::map<dnmi::db::Connection*,bool>           Connections;
  typedef std::map<dnmi::db::Connection*,bool>::iterator IConnections;
  typedef boost::mutex::scoped_lock                      Lock;

  boost::mutex     m;;
  boost::condition cond;
  int              nFree;

  Connections connection;

 public:
  ConnectionCache():nFree(0){};
  ~ConnectionCache();

  /**
   * \brief Add a connection to the cache.
   *
   * \param con the connection to add to the cache.
   */
  void addConnection(dnmi::db::Connection* con);

  /**
   * \brief find a unused connection in the cache and return it.
   *
   * \return A pointer to a Connection if there is one ready. 0 if
   * all Connection's in the cache is busy. When the Connection is not
   * needed anymore it must be marked as ready for reuse by a call to
   * freeConnection(dnmi::db::Connection* con).
   *
   * \see freeConnection(dnmi::db::Connection* con)
   */
  dnmi::db::Connection* findFreeConnection();
  
  /**
   * \brief  return a Connection back to the cache so it can be reused.
   *
   * \param con a Connection to be marked as ready. The Connection must 
   * previously have been obtained from a call to findFreeConnection().
   *
   * \see findFreeConnection().
   */ 
  bool                  freeConnection(dnmi::db::Connection* con);

};
  
/** @} */

#endif
