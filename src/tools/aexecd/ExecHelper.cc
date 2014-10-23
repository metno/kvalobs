#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <iostream>
#include <stdio.h>
#include <sstream>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "miutil/msleep.h"
#include "ExecHelper.h"

namespace pt = boost::posix_time;

using namespace std;

namespace {
/*
 * The code for system is mostly copied from the book
 * "Advanced Programming in the UNIX environment 3. ed."
 * and adapted our use.
 */

int
system(const char *cmdstring, miutil::SimpleSocket *pSocket, int timeout )
{
    pid_t               pid;
    int                 status;
    struct sigaction    ignore, saveintr, savequit;
    sigset_t            chldmask, savemask;

    if (cmdstring == NULL)
        return 1;      /* always a command processor with UNIX */

    ignore.sa_handler = SIG_IGN;    /* ignore SIGINT and SIGQUIT */
    sigemptyset(&ignore.sa_mask);
    ignore.sa_flags = 0;
    if (sigaction(SIGINT, &ignore, &saveintr) < 0)
        return -1;
    if (sigaction(SIGQUIT, &ignore, &savequit) < 0)
        return -1;
    sigemptyset(&chldmask);         /* now block SIGCHLD */
    sigaddset(&chldmask, SIGCHLD);
    if (sigprocmask(SIG_BLOCK, &chldmask, &savemask) < 0)
        return -1;

    cout << "---- BEGIN PROGRAM LOG ---------------------------------------------------------" << endl;
    cout << "---- " << cmdstring << endl;
    cout << "--------------------------------------------------------------------------------" << endl;
    if ((pid = fork()) < 0) {
        status = -1;    /* probably out of processes */
    } else if (pid == 0) {          /* child */
        pSocket->close();
        /* restore previous signal actions & reset signal mask */
        sigaction(SIGINT, &saveintr, NULL);
        sigaction(SIGQUIT, &savequit, NULL);
        sigprocmask(SIG_SETMASK, &savemask, NULL);

        //We should probably try to close all files except cin, cout and cerr.
        execl("/bin/sh", "sh", "-c", cmdstring, (char *)0);
        _exit(127);     /* exec error */
    } else {                        /* parent */
        int wret;
        pt::ptime endtime( pt::pos_infin );
        pt::ptime now;
        ostringstream opid;
        opid << "PID: " << pid << "\n";
        pSocket->writeln( opid.str() );

        if( timeout > 0 )
            endtime = pt::microsec_clock::universal_time() + pt::seconds( timeout );

        while( pid != 0 ) {
            wret = waitpid( pid, &status, WNOHANG );

            if( wret == 0 ) {
                now = pt::microsec_clock::universal_time();
                if( now <= endtime ) {
                    miutil::msleep( 10 ); //sleep for 1/100 of a second.
                    continue;
                } else {
                    kill( pid, SIGKILL );
                    while( waitpid(pid, &status, 0 ) < 0 ) {
                        if (errno == EINTR) {
                            continue;
                        } else {
                            status = -1; //error other than EINTR from waitpid().
                            break;
                        }
                    }
                    status = -2; //timeout
                    pid = 0;
                }
            } else if( wret < 0 ) {
                if (errno != EINTR) {
                    status = -1;
                    break;
                }
            } else { //wret > 0 == pid
                break;
            }
        }
    }

    cout << "---- END PROGRAM LOG -----------------------------------------------------------" << endl;

    /* restore previous signal actions & reset signal mask */
    if (sigaction(SIGINT, &saveintr, NULL) < 0)
        return -1;

    if (sigaction(SIGQUIT, &savequit, NULL) < 0)
        return -1;

    if (sigprocmask(SIG_SETMASK, &savemask, NULL) < 0)
        return -1;

    return status;
}
}

namespace exechelper {
void
exec( miutil::SimpleSocket *pSocket,
        const std::string &cmdstring, int timeout )
{
    pid_t pid;
    std::string  sRet;
    char  sPid[100];
    int   ret;
    pt::ptime startAt;
    pt::ptime stopAt;
    pt::time_duration elapsedTime;

    if( cmdstring.empty() ) {
        sRet="ERROR: NOCMD\n";
        pSocket->writeln(sRet);
        delete pSocket;
        return;
    }

    startAt = pt::microsec_clock::universal_time();

    ret = system( cmdstring.c_str(), pSocket, timeout  );

    stopAt = pt::microsec_clock::universal_time();

    elapsedTime = stopAt - startAt;

    std::ostringstream ost;

    if( ret == -1 ) {
        ost << "ERROR: errno: " << errno <<". " << strerror( errno );
    } else  if( ret == -2 ) {
        ost << "TIMEOUT: " << timeout;
    } else {
        ret = WEXITSTATUS( ret );
        if( ret == 127 ) {
            ost << "ERROR: Could not execute command '" << cmdstring << "'.";
        } else {
            ost.setf(ios::fixed, ios::floatfield );

            ost << "EXITCODE: " << ret << " " << elapsedTime.total_microseconds();
        }
    }
    ost << "\n";
    std::cout << ost.str() << std::endl;

    pSocket->writeln( ost.str() );

    delete pSocket;
    exit( 0 );
}

}


