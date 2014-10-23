#include <stdio.h>
#include <sstream>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include "aexecclient.h"
#include "simplesocket.h"

using namespace std;

namespace pt = boost::posix_time;

namespace miutil {

int
AExecClient::
getExitCode( const std::string &buf_ )
{
    string buf=buf_;
    string tmp;
    long elapsedTime=LONG_MIN;
    int exitCode=INT_MIN;

    try {
        string::size_type i;
        i = buf.find_first_not_of( "0123456789" );

        if( i == string::npos )
            return boost::lexical_cast<int>( buf );

        tmp = buf.substr( 0, i);
        exitCode = boost::lexical_cast<int>( tmp );

        tmp = buf.substr( i );
        boost::trim_if( tmp , boost::is_any_of("\t\n\r ") );

        if( tmp.empty() )
            return exitCode;

        elapsedTime = boost::lexical_cast<long>( tmp );
        executionTime = pt::microseconds( elapsedTime );
        return exitCode;
    }
    catch( ... ) {
        if( exitCode != INT_MIN )
            return exitCode;

        errMsg = "FORMATERROR: EXITCODE: '"+buf_+"'. expecting 'exitcode executiontime'";
        return -1;
    }
}

pid_t 
AExecClient::
exec(const std::string &command,
     const std::string &logfile,
     int               timeoutInSecond)
{
    char sint[100];
    ostringstream cmd;
    string buf;
    int         nBytes;
    int         n;
    unsigned long pid;
    bool separator=false;
    string::reverse_iterator rit;


    if(port<0)
        return 0;

    if(command.empty())
    {
        errMsg="NOCMD, there is no command to execute.";
        return 0;
    }

    if( !sock.connected() ){
        std::ostringstream ost;
        ost <<"NOCONNECT, Can't connect to server <"<< host << ":"<<port <<">.";
        errMsg = ost.str();

        return 0;
    }

    cmd << "[";

    if(!logfile.empty()) {
        cmd << "LOG:" << logfile;
        separator = true;
    }

    if( separator )
        cmd << ";";

    cmd << "TIMEOUT:" << timeoutInSecond;
    cmd << "]";
    cmd << command << "\n";;

    nBytes=sock.writeln( cmd.str() );

    if(nBytes<=0)
    {
        errMsg="NOWRITE, cant connect to server for writing.";
        return 0;
    }

    sock.readln(buf, timeoutInSecond);

    if(buf.empty())
    {
        errMsg="NORESULT, cant get any response from server.";
        return 0;
    }

    rit=buf.rbegin();

    if(*rit!='\n')
    {
        errMsg="INVALIDFORMAT, the response from the server was invalid formatted.";
        return 0;
    }

    if(sscanf(buf.c_str(), "PID: %ld", &pid)!=1)
    {
        errMsg="AEXECDERR: ";
        errMsg+=buf;
        errMsg.erase(errMsg.length()-1); //Fjern linjeskift.
        return 0;
    }

    return (pid_t) pid;
}

int
AExecClient::
wait( int timeoutInSecondBetweenChar )
{
    string buf;
    timeout_ = false;
    executionTime = pt::time_duration( pt::not_a_date_time );

    if( !sock.connected() ) {
        std::ostringstream ost;
        ost <<"NOCONNECT, not connect to server <"<< host << ":"<<port <<">.";
        errMsg = ost.str();

        return -1;
    }

    int ret = sock.readln(buf, timeoutInSecondBetweenChar );

    if( ret < 0 ) {
        timeout_ = true;
        return -1;
    }

    if( buf.empty() ) {
        errMsg = "UNEXPECTED: No result from the <aexecd>.";
        return -1;
    }

    int i=buf.find(":");

    if( i == string::npos ) {
        errMsg = "UNEXPECTED: aexecd, format error.";
        return -1;
    }

    string what = buf.substr( 0, i );

    if( what == "ERROR") {
        errMsg = buf.substr( i+1 );
        return -1;
    } else if( what == "EXITCODE" ){
        string tmp=buf.substr( i+1 );
        boost::trim_if( tmp , boost::is_any_of("\t\n\r "));
        return getExitCode( tmp );
    } else {
        errMsg = "FORMATERROR: Unexpected result '" + buf +"'.";
        return -1;
    }
}
}
