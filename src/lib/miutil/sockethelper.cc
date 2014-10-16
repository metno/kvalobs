#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <sys/time.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <iostream>
#include <stdio.h>
#include "sockethelper.h"

namespace miutil {

int
socketGetCh(int fd, char &ch, bool &timedout, int timeoutInSec)
{
    fd_set  r, w, x;
    timeval tv;
    int     rc;
    int     ret;

    //std::cerr << "sockethelper: socketGetCh: kalt\n";

    timedout=false;

    FD_ZERO(&r);
    FD_ZERO(&w);
    FD_ZERO(&x);

    FD_SET(fd, &r);

    do
    {
        do
        {
            tv.tv_sec=timeoutInSec;
            tv.tv_usec=0;
            ret=::select(fd+1, &r, &w, &x, &tv);
        }while(ret<0 && (errno==EINTR || errno==EAGAIN));

        if(ret<0){
            std::cerr << "sockethelper: socketGetCh: select returned -1 errno!=EINTR!\n";
            return -1;
        }else if(ret==0){
            std::cerr << "sockethelper: socketGetCh: select timed out!\n";
            timedout=true;
            return 0;
        }

        rc=read(fd, &ch, 1);

        if(rc==1)
            return 1;

        if(rc==0){
            std::cerr << "sockethelper: socketGetCh: EOF\n";
            return 0;	/* EOF, no data read */
        }

        if(errno==EINTR || errno==EAGAIN)
            continue;
        else{
            perror("sockethelper: socketGetCh: ");
            std::cerr << "sockethelper: socketGetCh: errno=" << errno << "\n";
            return -1;
        }
    }while(true);
    return -1;
}

int
socketGetChExt(int fd, char &ch, bool &timedout, int timeoutInMillimSec)
{
    fd_set  r, w, x;
    timeval tv;
    int     rc;
    int     ret;

    //std::cerr << "sockethelper: socketGetCh: kalt\n";

    timedout=false;

    FD_ZERO(&r);
    FD_ZERO(&w);
    FD_ZERO(&x);

    FD_SET(fd, &r);

    do
    {
        do
        {
            tv.tv_sec=(int)timeoutInMillimSec/1000;
            tv.tv_usec=(timeoutInMillimSec%1000)*1000;
            ret=::select(fd+1, &r, &w, &x, &tv);
        }while(ret<0 && (errno==EINTR || errno==EAGAIN));

        if(ret<0){
            std::cerr << "sockethelper: socketGetCh: select returned -1 errno!=EINTR!\n";
            return -1;
        }else if(ret==0){
            std::cerr << "sockethelper: socketGetCh: select timed out!\n";
            timedout=true;
            return 0;
        }

        rc=read(fd, &ch, 1);

        if(rc==1)
            return 1;

        if(rc==0){
            std::cerr << "sockethelper: socketGetCh: EOF\n";
            return 0;	/* EOF, no data read */
        }

        if(errno==EINTR || errno==EAGAIN)
            continue;
        else{
            perror("sockethelper: socketGetCh: ");
            std::cerr << "sockethelper: socketGetCh: errno=" << errno << "\n";
            return -1;
        }
    }while(true);
    return -1;
}




int
readString(int fd, std::string &str, int timeoutInSec)
{
    fd_set  r, w, x;
    timeval tv;
    int	    n=1;
    int     rc;
    char    c;
    int     ret;
    bool    more=true;


    FD_ZERO(&r);
    FD_ZERO(&w);
    FD_ZERO(&x);

    FD_SET(fd, &r);
    str="";

    do
    {
        do
        {
            tv.tv_sec=timeoutInSec;
            tv.tv_usec=0;
            ret=::select(fd+1, &r, &w, &x, &tv);
        }while(ret<0 && errno==EINTR);

        if(ret<0)
            return -1;
        else if(ret==0)
            return n;

        if((rc=read(fd, &c, 1))== 1)
        {
            str+=c;
            n++;

            if((c=='\n')||(c=='\0')||(c==0))
                more=false;
        }else if(rc==0)
        {
            if (n == 1)
                return 0;	/* EOF, no data read */
            else
                more=false;		/* EOF, some data was read */
        }else
        {
            if(errno==EINTR || errno==EAGAIN)
                continue;
            else
                return -1;
        }
    }while(more);

    return n;
}

int
readStringExt(int fd, std::string &str, int timeoutInMillimSec)
{
    fd_set  r, w, x;
    timeval tv;
    int	    n=1;
    int     rc;
    char    c;
    int     ret;
    bool    more=true;


    FD_ZERO(&r);
    FD_ZERO(&w);
    FD_ZERO(&x);

    FD_SET(fd, &r);
    str="";

    do
    {
        do
        {
            tv.tv_sec=(int)timeoutInMillimSec/1000;
            tv.tv_usec=(timeoutInMillimSec%1000)*1000;
            ret=::select(fd+1, &r, &w, &x, &tv);
        }while(ret<0 && errno==EINTR);

        if(ret<0)
            return -1;
        else if(ret==0)
            return n;

        if((rc=read(fd, &c, 1))== 1)
        {
            str+=c;
            n++;

            if((c=='\n')||(c=='\0')||(c==0))
                more=false;
        }else if(rc==0)
        {
            if (n == 1)
                return 0;	/* EOF, no data read */
            else
                more=false;		/* EOF, some data was read */
        }else
        {
            if(errno==EINTR || errno==EAGAIN)
                continue;
            else
                return -1;
        }
    }while(more);

    return n;
}


int
writeString(int fd, const std::string &str)
{
    int	nleft, nwritten, ntot;
    int   n;
    char  buf[512];
    bool  writeNewline=false;
    bool  retry;
    std::string::const_iterator it;
    std::string::const_reverse_iterator rit;

    if(str.length()==0)
        return 0;

    rit=str.rbegin();

    if(*rit!='\n')
        writeNewline=true;

    ntot=0;
    it=str.begin();

    while(it!=str.end())
    {
        for(n=0; n<512 && it!=str.end(); n++, it++)
            buf[n]=*it;

        nleft=n;
        while(nleft>0)
        {
            nwritten = write(fd, buf, nleft);

            if(nwritten>0)
            {
                ntot+=nwritten;
                nleft-=nwritten;
            }else if(errno==EINTR || errno==EAGAIN)
                continue;
            else{
                perror("sockethelper: writeString: ");
                return ntot;		/* error */
            }
        }
    }

    if(writeNewline)
    {
        buf[0]='\n';

        do
        {
            nwritten=write(fd, buf, 1);

            if(nwritten==1)
            {
                ntot++;
                retry=false;
            }else
                retry=true;
        }while(retry&&(errno==EINTR || errno==EAGAIN));
    }


    return ntot;
}


int
socketWriteBuf(int fd, const std::string &str)
{
    int	nleft, nwritten, ntot;
    int   n;
    char  buf[512];
    bool  retry;
    std::string::const_iterator it;

    if(str.length()==0)
        return 0;

    ntot=0;
    it=str.begin();

    while(it!=str.end())
    {
        for(n=0; n<512 && it!=str.end(); n++, it++)
            buf[n]=*it;

        nleft=n;
        while(nleft>0)
        {
            nwritten = write(fd, buf, nleft);

            if(nwritten>0)
            {
                ntot+=nwritten;
                nleft-=nwritten;
            }else if(errno==EINTR || errno==EAGAIN){
                //std::cerr << "sockethelper: writeBuf: errno="
                //	      << (errno==EINTR?"EINTR\n":"EAGAIN\n");
                continue;
            }else if(errno==EPIPE || errno==ECONNRESET){
                //std::cerr << "sockethelper: writeBuf: socket closed by peer.\n";
                //std::cerr << "sockethelper: writeBuf: errno="
                //      << (errno==EPIPE?"EPIPE\n":"ECONNRESET\n");
                return 0;
            }else{
                //std::cerr << "sockethelper: writeBuf: errno=" << errno << "\n";
                //perror("sockethelper: writeBuf: ");
                return ntot;		/* error */
            }
        }
    }

    return ntot;
}

bool
socketSetLinger(int sd, bool enable, int timeoutInSec)
{
    struct linger li;

    if(enable){
        li.l_onoff=1;
        li.l_linger=timeoutInSec;
    }else{
        li.l_onoff=0;
        li.l_linger=timeoutInSec;;
    }

    if(setsockopt(sd,
            SOL_SOCKET,
            SO_LINGER,
            (char *) &li,
            sizeof(linger))==0)
        return true;

    return false;
}



socket_status_codes::ErrorTypes
setupConnection(const std::string &servname, int port, int& fd)
{
    int err, enable;
    struct sockaddr_in saddr;
    struct hostent *ptrh;
    int sd;


    fd=-1;

    // create a TCP socket
    sd=socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(sd<0)
        return socket_status_codes::SOCKET_CREATE_ERR;

    // initialize socket address 
    memset((char *) &saddr, 0, sizeof(struct sockaddr_in));
    saddr.sin_family=AF_INET;
    saddr.sin_port=htons((u_short) port);

#if 0    
    // set socket to KEEPALIVE
    enable=1;
    err=setsockopt(sd, 
            SOL_SOCKET,
            SO_KEEPALIVE,
            (char *) &enable,
            sizeof(int));
    if(err){
        close(sd);
        return SET_SOCKOPT_ERR;
    }
#endif

    err=setsockopt(sd, 
            SOL_SOCKET,
            SO_REUSEADDR,
            (char *) &enable,
            sizeof(int));

    if(err){
        close(sd);
        return socket_status_codes::SET_SOCKOPT_ERR;
    }
    //convert "servname" to IP address
    ptrh=gethostbyname(servname.c_str());

    if(ptrh==NULL){
        close(sd);
        return socket_status_codes::GET_HOSTBYNAME_ERR;
    }

    memcpy(&saddr.sin_addr, ptrh->h_addr, ptrh->h_length);

    // connect to the specific server
    err=connect(sd, (struct sockaddr *) &saddr, sizeof(saddr));

    if(err<0){
        perror("setupConnection: error: ");
        close(sd);
        return socket_status_codes::CONNECT_ERR;
    }

    fd=sd;

    return socket_status_codes::NO_ERR;
}

}
