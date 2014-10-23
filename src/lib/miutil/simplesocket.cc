#include <stdio.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <iostream>
#include <sys/time.h>
#include "simplesocket.h"

namespace miutil {


void
SimpleSocket::
setfd(int fd_)
{
    if(fd_<=0)
        return;

    if(fd>0)
        close();

    fd=fd_;
    mConnected=true;
}

SimpleSocket::
SimpleSocket()
:fd(0), mConnected(false), timeout_( false )
{
}

SimpleSocket::
SimpleSocket(int fd_)
:fd(fd_), mConnected(fd_>0), timeout_( false )
{
}

SimpleSocket::
~SimpleSocket()
{ 
    if(mConnected) 
        close();
}

void       
SimpleSocket::
close()
{
    if(mConnected){
        //std::cerr << "SimpleSocket::close() \n";
        ::close(fd);
    }

    fd=0;
    mConnected=false;
}


int        
SimpleSocket::
getCh(char &ch, bool &timedout, int timeoutInSec)
{
    if(!mConnected){
        //std::cerr << "SimpleSocket::getCh: NOT CONNECTED!\n";
        return -1;
    }

    return socketGetCh(fd, ch, timedout, timeoutInSec);
}

int        
SimpleSocket::
getChExt(char &ch, bool &timedout, int timeoutInMilliSec)
{
    if(!mConnected){
        //std::cerr << "SimpleSocket::getCh: NOT CONNECTED!\n";
        return -1;
    }

    return socketGetChExt(fd, ch, timedout, timeoutInMilliSec);
}


int        
SimpleSocket::
writeBuf(const std::string &str)
{
    if(!mConnected)
        return -1;

    return socketWriteBuf(fd, str);
}


int
SimpleSocket::
writeln(const std::string &str)
{
    if(!mConnected)
        return -1;

    return writeString(fd, str);
}

int
SimpleSocket::
readln(std::string &str, int timeoutInSecBetweenReceivedBytes)
{
    str="";

    if(mConnected==0)
        return -1;

    return readString(fd, str, timeoutInSecBetweenReceivedBytes);
}

int
SimpleSocket::
readlnExt(std::string &str,
        int timeoutInMilliSecBetweenReceivedBytes)
{
    str="";

    if(mConnected==0)
        return -1;

    return readStringExt(fd, str, timeoutInMilliSecBetweenReceivedBytes);
}



bool       
SimpleSocket::
setLinger(bool enable, int timeoutInSec)
{
    if(mConnected==0)
        return false;

    return socketSetLinger(fd, enable, timeoutInSec);
}


bool
SimpleSocket::
getLinger(bool &enabled, int &timeout)
{
    struct linger li;
    socklen_t     len;

    if(mConnected==0)
        return false;


    if(getsockopt(fd,
            SOL_SOCKET,
            SO_LINGER,
            (void *) &li,
            &len)==0){
        enabled=li.l_onoff;
        timeout=li.l_linger;

        //    std::cerr << "SimpleSocket::getLinger: " << (enabled!=0?"TRUE":"FALSE")
        //      << "  timeout: " << timeout << " len=" << len <<"\n";
        return true;
    }

    return false;
}


SimpleSocketClient::
SimpleSocketClient()
:mErrorType(socket_status_codes::NOT_CONNECTED_ERR)
{
}

SimpleSocketClient::
SimpleSocketClient(const std::string &servername, int port)
:SimpleSocket()
{
    mErrorType=socket_status_codes::NOT_CONNECTED_ERR;
    mConnected=connect(servername, port);
}

SimpleSocketClient::
~SimpleSocketClient()
{
}

bool
SimpleSocketClient::
connect(const std::string &servername, int port)
{
    int fd_;

    close();

    mErrorType=setupConnection(servername, port, fd_);

    //std::cout << "SimpleSocketClient::connect: " << mErrorType << "\n"; 

    if(mErrorType==socket_status_codes::NO_ERR)
        setfd(fd_);

    return connected();
}

SimpleSocketServer::
SimpleSocketServer():port(-1)
{
}

SimpleSocketServer::
~SimpleSocketServer()
{
    close();
}

bool 
SimpleSocketServer::
bind(int port_)
{
    int			sockfd;
    struct sockaddr_in	serv_addr;
    int                 flag=1;

    port=-1;

    close();
    mErrorType=socket_status_codes::NO_ERR;

    /*
     * Open a TCP socket (an Internet stream socket).
     */

    if((sockfd=socket(AF_INET, SOCK_STREAM, 0))<0)
    {
        mErrorType=socket_status_codes::SOCKET_CREATE_ERR;
        return false;
    }

    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
            (char *)&flag, sizeof(int))        !=0)
    {
        mErrorType=socket_status_codes::SET_SOCKOPT_ERR;
        ::close(sockfd);
        return false;
    }


    /*
     * Bind our local address so that the client can send to us.
     */

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port_);

    if(::bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
        mErrorType=socket_status_codes::SOCKET_BIND_ERR;
        ::close(sockfd);
        return false;
    }

    setfd(sockfd);

    port=port_;
    return true;
}

bool 
SimpleSocketServer::
listen(int backlog)
{
    if(!connected())
        return false;

    if(::listen(fd, backlog)==-1)
    {
        mErrorType=socket_status_codes::SOCKET_LISTEN_ERR;
        return false;
    }

    return true;
}

SimpleSocket* 
SimpleSocketServer::
accept(bool &interupted, int timeout)
{
    int			newsockfd;
    socklen_t           clilen;
    struct sockaddr_in	cli_addr;
    int                 flag=1;
    int                 nRetry=0;
    SimpleSocket        *s;

    mErrorType=socket_status_codes::NO_ERR;
    interupted=false;

    if(!mConnected)
    {
        mErrorType=socket_status_codes::NOT_CONNECTED_ERR;
        return 0;
    }

    clilen=sizeof(cli_addr);

    do
    {

        //#ifdef __IRIX_6__

        /* For IRIX, og sikkert andre system, returnerer ikke
         * accept når et signal mottas, men blir
         * hengende til en forbindelse er opprettet eller at
         * det oppstår en feil på socketen vi lytter på.
         * Vi kan omgå dette problemet med å gjøre en read select
         * på filedeskriptoren vi aksepterer forbindelser over. Dette
         * vil returnerer 0 når en forbindelse kan bli opprettet med accept.
         * Samtidig vil select avbrytes når et signal mottas  og errno blir
         * satt til EINTR. Dette vil virke for alle system også de som
         * avbryter accept ved signal.
         */

        fd_set  r, w, x;
        int     ret;
        timeval tv;

        if(timeout>0){
            tv.tv_sec=timeout;
        }else{
            tv.tv_sec=0;
        }

        tv.tv_usec=0;


        FD_ZERO(&r);
        FD_ZERO(&w);
        FD_ZERO(&x);

        FD_SET(fd, &r);

        ret=::select(fd+1, &r, &w, &x, &tv);

        if(ret<0){
            if(errno==EINTR || errno==EAGAIN){
                interupted=true;
                return 0;
            }

            mErrorType=socket_status_codes::SOCKET_ACCEPT_ERR;

            return 0;

        }else if(ret==0){
            //Timedout
            interupted=true;

            return 0;
        }
        //#endif

        newsockfd=::accept(fd,
                (struct sockaddr *) &cli_addr,
                &clilen);

        if(newsockfd<0)
        {
            perror("newsockfd ...");
            if(nRetry<10 && (errno==ENETDOWN ||
                    errno==EPROTO   ||
                    errno==ENOPROTOOPT ||
                    errno==EHOSTDOWN   ||
                    errno==EHOSTUNREACH ||
                    errno==EOPNOTSUPP   ||
                    errno==ENETUNREACH)
            )
            {
                sleep(1);
                nRetry++;
            }else if(errno==EINTR || errno==EAGAIN)
            {
                interupted=true;
                return 0;
            }else
            {
                mErrorType=socket_status_codes::SOCKET_ACCEPT_ERR;
                return 0;
            }
        }
    }while(newsockfd<0 && nRetry<10);

    if(setsockopt(newsockfd, SOL_SOCKET, SO_REUSEADDR,
            (char *)&flag, sizeof(int)) !=0)
    {
        mErrorType=socket_status_codes::SET_SOCKOPT_ERR;
        ::close(newsockfd);
        return 0;
    }

    try
    {
        s=new SimpleSocket(newsockfd);
    }
    catch(...)
    {
        ::close(newsockfd);
        mErrorType=socket_status_codes::NO_MEM;
        s=0;
    }

    return s;
}

}
