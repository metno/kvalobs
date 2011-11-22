/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: kvDbGate.cc,v 1.14.2.4 2007/09/27 09:02:30 paule Exp $

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

#ifndef __KVALOBS_DATAINSERTTRANSACTION_H__
#define __KVALOBS_DATAINSERTTRANSACTION_H__

#include <string>
#include <list>
#include <stdexcept>
#include <kvdb/kvdb.h>
#include <kvalobs/kvDbBase.h>

namespace kvalobs {


/**
 * DataInsertTransaction is an helper class to insert data elements into
 * the database. The data must be of a type derived from kvDbBase.
 *
 * It has three mode of operation defined by the Action parameter to the constructor.
 *   - INSERTONLY.
 *     In this mode all data is inserted into the database. If the database
 *     all ready have data with the same key in the database a ModeException
 *     is thrown with the index of the first offending data element. The index is
 *     counted from 0.
 *   - INSERT_OR_UPDATE
 *     If the key for a data element is all ready in the database it is updated.
 *   - INSERT_OR_REPLACE
 *     If the key for a data element is all ready in the database it is replaced, ie
 *     the data element in the database is deleted an the new element is inserted.
 *
 * The difference between INSERT_OR_UPDATE and INSERT_OR_REPLACE is that
 * INSERT_AND_REPLACE is dependent on the class that is inserted. For a kvData
 * element it means that both original and \em tbtime is updated for the key. In
 * INSERT_OR_UPDATE only the elements corrected, contrilinfo, useinfo and cfailed
 * is updated.
 *
 * On failure is an exception thrown. Possible exception is all from kvdb.h, ModeException and
 * std::logic_error.
 */
class DataInsertTransaction : public dnmi::db::Transaction
{
public:
   typedef enum{ INSERTONLY, INSERT_OR_UPDATE, INSERT_OR_REPLACE } Action;

   class ModeException : public std::logic_error
   {
   public:
      int index;
      ModeException( int index, const std::string &msg ) :
         std::logic_error( msg ), index( index ) {}
   };

private:
   DataInsertTransaction& operator=( const DataInsertTransaction & );
   DataInsertTransaction( const DataInsertTransaction & );
   DataInsertTransaction();

protected:
   bool deleteElems;
   int savedExcetion;
   std::string errorCode;
   int index;
   const std::list<kvalobs::kvDbBase*> *elems;
   std::string tblName;
   Action action;
   bool fastExit;
   std::string lastError;
   dnmi::db::Connection *conection;

   /**
    * Try to insert the data elemnt into the table given with \em tblName or
    * data->tableName(). The function return true if the data element is successfuly
    * inserted into the database and false if the 'key' for the datalement all ready is
    * in the databse.
    *
    * On all other errors an exception is thrown.
    *
    * @param data The data to insert.
    * @return false if the data element (key) all ready is in the databse. True if the
    * data element is inserted.
    * @throw an exception on all other errors.
    */
   bool insert( kvalobs::kvDbBase *data );
   void update( kvalobs::kvDbBase *data );
   void replace( kvalobs::kvDbBase *data );

   void reThrow();

public:

   template<class T>
      DataInsertTransaction( const std::list<T>& li, Action action_ = INSERTONLY,
                             const std::string &tblName_ = "" )
                             : elems( 0 ), tblName( tblName_ ), action( action_ ) {
         typename std::list<T>::const_iterator it = li.begin();

         if( it == li.end() )
            return;

         elems = new std::list<kvalobs::kvDbBase*>();
         deleteElems = true;

         for( ; it != li.end(); it++ ){
            kvalobs::kvDbBase *dat = const_cast<T*> (&(*it));

            elems->push_back( dat );
         }
      }

   DataInsertTransaction( const std::list<kvalobs::kvDbBase*> &elem,
                          Action action = INSERTONLY,
                          const std::string &tblName_="" );
   ~DataInsertTransaction();

   virtual bool run();
   virtual bool operator()( dnmi::db::Connection *conection );
   virtual void onAbort( const std::string &driverid,
                         const std::string &errorMessage,
                         const std::string &errorCode );
   virtual void onSuccess();
   virtual void onRetry();
   virtual void onMaxRetry( const std::string &lastError );

};

}

#endif /* __KVALOBS_DATAINSERTTRANSACTION_H__ */
