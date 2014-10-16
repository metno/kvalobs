#ifndef __KVEXEC_CLIENT_H__
#define __KVEXEC_CLIENT_H__

#include <unistd.h>
#include <string>
#include "miutil/simplesocket.h"

namespace miutil {

class AExecClient
{
    AExecClient(const AExecClient &);
    AExecClient& operator=(const AExecClient &);
    AExecClient();

    int         port;
    std::string host;
    std::string errMsg;
    bool timeout_;
    SimpleSocketClient sock;

 public:
    AExecClient(const std::string &host_, int port_)
     :port(port_),host(host_), timeout_( false ), sock( host, port )
	{
	}
      
    pid_t exec(const std::string &command,
	       const std::string &logfile,
	       int               timeoutInSecondBetweenChar=10);


    /**
     * Return the exit code from the command if >= 0.
     * -1 on failure.
     * The error message is in errMsg;
     */
    int wait( int timeoutInSecondBetweenChar=60 );

    bool timeout()const { return timeout_; }
    std::string getErrMsg()const { return errMsg;} 
};
}
#endif
