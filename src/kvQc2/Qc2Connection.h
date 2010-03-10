#ifndef _Qc2Connection_h
#define _Qc2Connection_h

#include <string>
#include <list>
#include <map>
#include <puTools/miTime.h>

#include <milog/milog.h>

#include <kvalobs/kvDbGate.h>
#include <set>
#include <kvdb/dbdrivermgr.h>

namespace
{
  
  ///Handles db connection and dsconnection after timeout

  class ConnectionHandler
  {
      Qc2App & app_;
      dnmi::db::Connection * con;
      int idleTime;
      static const int max_idle_time = 60;
    public:

      ConnectionHandler( Qc2App & app )
          : app_( app ), con( 0 ), idleTime( 0 )
      {}

      ~ConnectionHandler()
      {
        LOGDEBUG( "Closing the database connection before termination!" );
        if ( con )
          app_.releaseDbConnection( con );
      }

      
       // \returns a connection to db, either freshly generated or an old one
      
      dnmi::db::Connection * getConnection()
      {
        if ( ! con )
        {
          while ( ! app_.shutdown() )
          {
            con = app_.getNewDbConnection();
            if ( con )
            {
              LOGDEBUG( "Created a new connection to the database!" );
              break;
            }
            LOGINFO( "Can't create a connection to the database, retry in 5 seconds .." );
            sleep( 5 );
          }
        }
        idleTime = 0;
        return con;
      }

      
       // Signal that the db connection is not needed. If this signal is used
       // \c max_idle_time times in a row, the db connection will be released.
      
      void notNeeded()
      {
        if ( con and ++ idleTime >= max_idle_time )
        {
          LOGDEBUG( "Closing the database connection!" );
          app_.releaseDbConnection( con );
          con = 0;
          idleTime = 0;
        }
      }
  };

}

class Qc2Connection
{

  public:
    Qc2Connection( dnmi::db::Connection *con );

    bool dbOk() const
    {
      return connection_ok;
    }


    dnmi::db::Connection * getConnection()
    {
      return connection_;
    }

  private:
    bool connection_ok;
    kvalobs::kvDbGate dbGate;

    dnmi::db::Connection * connection_;

};

#endif
