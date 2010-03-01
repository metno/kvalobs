#! /bin/sh

NODENAME=$(uname -n)
VARDIR=$(kvconfig --localstatedir)/kvalobs
BINDIR=$(kvconfig --bindir)
RUNDIR=$VARDIR/run
KVALOBS=$HOME
KVSYNOPD_PIDFILE=$RUNDIR/kvsynopd-$NODENAME.pid
KVSERVICED_PIDFILE=$RUNDIR/kvServiced-$NODENAME.pid
SUBIDFILE=$VARDIR/kvsynop/datasubscriber.id
TIMEOUT=120


function isrunning()
{
    prog=$1
   
    if [ -f $RUNDIR/$prog-$NODENAME.pid ]; then 
	PID=`cat $RUNDIR/$prog-$NODENAME.pid`
	#echo "PID: $prog: $PID"
	#echo "	kill  -0 $PID"
	kill  -0 $PID > /dev/null 2>&1

	if [ $? -eq 0 ]; then
	    PIDS=`pgrep $prog 2>/dev/null`
	    running=`echo $PIDS | grep $PID`
	    
	    if [ ! -z "$running" ]; then
		return 0
	    fi
        fi
    fi
    
    return 1
}


function killprog()
{
    prog=$1
    
    if [ -f $RUNDIR/$prog-$NODENAME.pid ]; then 
	PID=`cat $RUNDIR/$prog-$NODENAME.pid`
	#echo "PID: $PROG: $PID"
	kill $PID > /dev/null 2>&1
	
	n=0
	isrunning $prog
	
        while [ $? -eq 0 -a $n -lt $TIMEOUT  ]; do
	    let n=n+1
	    sleep 1
	    isrunning $prog
	done
	
	isrunning $prog
	
	if [ $? -eq 0 ]; then
	    kill -9 $PID > /dev/null 2>&1
	    
	    n=0
	    isrunning $prog
	    
	    while [ $? -eq 0 -a $n -lt $TIMEOUT  ]; do
	      let n=n+1
	      sleep 1
	      isrunning $prog
	    done

	    isrunning $prog
	    
	    if [ $? -eq 1 ]; then
	       return 1
	    fi
	fi
    fi

    return 0
}



if [ ! -f $KVSERVICED_PIDFILE ]; then
    echo "KVALOBS is NOT running!"
    exit 0;
fi

kill -0 $(cat $KVSERVICED_PIDFILE)

if [ "$?" -ne 0 ]; then
    echo "kvServiced NOT running!"
    echo "kvalobs is DOWN!"
    exit 0
fi


if [ ! -f $KVSYNOP_PIDFILE ]; then
    echo "kvsynopd pidfile <$KVSYNOP_PIDFILE> missing. This is ok!"
    exit 0
fi


kill -0 $(cat $KVSYNOPD_PIDFILE)

if [ "$?" -ne 0 ]; then 
    echo "kvsynopd NOT running!!"

    exit 0
fi

if [ ! -f $SUBIDFILE ]; then
    echo "No subscriber file <$SUBIDFILE>! This is Ok!"
    exit 0
fi


SUBID=$(cat $VARDIR/kvsynop/datasubscriber.id)

if [ -z "$SUBID" ]; then
    echo "The subscriberfile <$SUBIDFILE> is empty!"
    exit 0
fi 

if [ ! -f $VARDIR/service/subscribers/$SUBID.sub ]; then
    date
    echo "kvServiced has forgotten us!!!????"
    dd=$(date '+%Y%m%dT%H%M%S')
    echo "Saving subscriberid <$SUBIDFILE> to <$SUBIDFILE.$dd>."
    mv $SUBIDFILE $SUBIDFILE.$dd
    echo "Trying to restart kvsynopd....."
    echo "Stopping kvsynopd ............."
    
    killprog kvsynopd
    
    echo "Starting kvsynopd ........."
   
    $BINDIR/kvsynopd -1 > /dev/null  2>&1 &
else
    echo "$(date) Ok!"
fi

exit 0



    
