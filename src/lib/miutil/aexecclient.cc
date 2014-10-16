#include <stdio.h>
#include <sstream>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include "aexecclient.h"
#include "simplesocket.h"

using namespace std;

namespace miutil {

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
        string exitcode=buf.substr( i+1 );
        boost::trim_if( exitcode , boost::is_any_of("\t\n\r "));

        try {
            return boost::lexical_cast<int>( exitcode );
        }
        catch( ... ) {
            errMsg = "FORMATERROR: EXITCODE: '"+buf.substr(i+1)+"'.";
            return -1;
        }
    } else if( what == "TIMEOUT" ) {
        string t=buf.substr( i+1 );
        boost::trim_if( t , boost::is_any_of("\t\n\r "));
        errMsg = "Timeout: "+ t +".";
        timeout_ = true;
        return -2;
    } else {
        errMsg = "FORMATERROR: Unexpected result '" + buf +"'.";
        return -1;
    }

}
}
