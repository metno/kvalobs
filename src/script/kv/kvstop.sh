#!/bin/bash
#  Kvalobs - Free Quality Control Software for Meteorological Observations 
#
#  Copyright (C) 2007 met.no
#
#  Contact information:
#  Norwegian Meteorological Institute
#  Box 43 Blindern
#  0313 OSLO
#  NORWAY
#  email: kvalobs-dev@met.no
#
#  This file is part of KVALOBS
#
#  KVALOBS is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public License as 
#  published by the Free Software Foundation; either version 2 
#  of the License, or (at your option) any later version.
#  
#  KVALOBS is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  General Public License for more details.
#  
#  You should have received a copy of the GNU General Public License along 
#  with KVALOBS; if not, write to the Free Software Foundation Inc., 
#  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA


KVCONFIG=__KVCONFIG__

KVBIN=`$KVCONFIG --bindir`
KVPID=`$KVCONFIG --localstatedir`/kvalobs/run
KVCONF=`$KVCONFIG --sysconfdir`/kvalobs
NODENAME=$(uname -n)

if [ -e ${KVCONF}/kv_ctl.conf ]; then
    . ${KVCONF}/kv_ctl.conf
else
    echo "Missing file:  ${KVCONF}/kv_ctl.conf"
    exit 1
fi

if [ "$USER" != "$KVUSER"  -a "$USER" != "root" ]; then
   echo "Only the '$KVUSER' user my start kvalobs."
   echo "You are loggd in as user '$USER'"
   exit 1
fi



function use()
{
    echo ""
    echo " kvstop [-l] [-a] [-h]"
    echo ""
    echo " kvstop er et program for � stoppe program i kvalobs"
    echo " systemet. Man kan enten stoppe alle programmene eller"
    echo " velge et program fra en liste som skal stoppes."
    echo ""
    echo "  -l velg i en liste et program som skal stoppes"
    echo "  -a stop alle programmene"
    echo "  -h skriv ut denne hjelp teksten og avslutt!"
    echo ""
    exit 1
}

killallopt=0
listopt=0
OPTERR=0
getopts "alh" opt
optret=$?

while [ $optret -eq 0 ]; do
    case $opt in
	a) killallopt=1;;
	l) listopt=1;;
	h) use;;
	\?) echo "Ugyldig option: $OPTARG"; use;;
	*) echo "Uventet, dette skal ikke skje!"; use;;
    esac
    getopts "ahl" opt
    optret=$?
done

echo "listopt: $listopt"
echo "killallopt: $killallopt"
echo "KVPID=$KVPID"
echo "TIMEOUT=$TIMEOUT"
echo "node: $NODENAME"

#inlist tar to parametere
function inlist()
{
    cmd=$1
    list=$2

    for c in $list; do
	if [ $c = $cmd ]; then
	    echo $c
	    return 0
	fi
    done
    
    echo ""
    return 1
}

function isrunning()
{
    prog=$1
   
    if [ -f $KVPID/$prog-$NODENAME.pid ]; then 
	PID=`cat $KVPID/$prog-$NODENAME.pid`
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

        rm -f $KVPID/$prog-$NODENAME.pid
   fi
    
   return 1
}


function killprog()
{
	local wasruning=false
    prog=$1
    echo -n "$prog ....."
    
    if [ -f $KVPID/$prog-$NODENAME.pid ]; then 
		PID=`cat $KVPID/$prog-$NODENAME.pid`
		#echo "PID: $PROG: $PID"
		kill $PID > /dev/null 2>&1
	
		n=0
		isrunning $prog
			
        while [ $? -eq 0 -a $n -lt $TIMEOUT  ]; do
	    	let n=n+1
	    	sleep 1
	    	wasrunning=true
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
	       		echo "failed!"
	       		return 1
	    	else
	       		echo "Ok! (hard)"
	    	fi
        else 
        	if [ "$wasrunning"="true" ]; then
        		echo "Stopped!"
        	else
	    		echo "Not running!"
	    	fi
       	fi
    else
       echo "Not running!"
    fi

    return 0
}


function findprog()
{
    progno=$1
    proglist=$2
    
    nn=1

    for prog in $proglist; do
	if [ $nn -eq $progno ]; then
	    echo "$prog"
	    return 0;
	fi
	let nn=nn+1
    done

    echo ""
    return 1
}



function select_prog_to_stop()
{
    cmd=""
    lineno=0
    list=""
    while [ -z $cmd  ]; do
	clear
	echo ""
	echo "Velg et program � stoppe!"
	echo "-------------------------"
	lineno=0
	list=""
	for PROG in $STOP_PROGS ; do
	    let lineno=lineno+1
	    echo "  $lineno: $PROG"
	    list=$(echo "$list $lineno")
	done
	
	echo "  q: quit"
	echo "-------------------------"
	read -p "Stop programmet [1-$lineno]: " cmd theRest
	
	if [ $cmd = "q" ]; then
	    echo "Quit!"
	    exit 1
	fi
	
	cmd=$(inlist $cmd "$list")
    done
    
    prog=$(findprog $cmd "$STOP_PROGS")
    
    if [ -z $prog ]; then
	echo "ERROR: fant ikke programmet i \$STOP_PROGS"
	echo "I filen:  ${KVALOBS}/etc/kv_ctl.conf"
	exit 1
    fi

    echo ""
    
    killprog $prog

    echo ""
}

if [ $listopt -ne 0 ]; then
    select_prog_to_stop
    exit 0
fi





echo " "
echo " "
echo "  Stopper kvalobs dette kan ta noe tid!"
echo "  Hvis det ikke skjer noe p� MER enn $TIMEOUT sekund"
echo "  bruk CTRL-C for � avbryte!"
echo " "
echo " "

for PROG in $STOP_PROGS ; do
      killprog $PROG
done

exit 0







