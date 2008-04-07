#! /bin/sh
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

if [ -e ${KVALOBS}/etc/kv_ctl.conf ]; then
    . ${KVALOBS}/etc/kv_ctl.conf
else
    echo "Missing file:  ${KVALOBS}/etc/kv_ctl.conf"
    exit 1
fi

KVBIN=`$KVCONFIG --bindir`
KVPID=`$KVCONFIG --localstatedir`/kvalobs/run

echo "KVBIN=$KVBIN"
echo "KVPID=$KVPID"
echo "TIMEOUT=$TIMEOUT"


function isrunning()
{
    prog=$1
   
    if [ -f $KVPID/$prog.pid ]; then 
	PID=`cat $KVPID/$prog.pid`
	#echo "PID: $PROG: $PID"
	kill  -0 $PID > /dev/null 2>&1

	if [ $? -eq 0 ]; then
	    PIDS=`pgrep $prog 2>/dev/null`
	    running=`echo $PIDS | grep $PID`
	    
	    if [ ! -z "$running" ]; then
		return 0
	    else
		rm -f $KVPID/$prog.pid
	    fi	
        fi
   fi
    
   return 1
}

function yes_no()
{
    echo -n "[j/n] : "
    stty raw         # Get one Character 
    readchar=`dd if=/dev/tty bs=1 count=1 2>/dev/null`
    stty -raw
    echo

    if [ $readchar != 'j' -a $readchar != 'J' ]; then
         return 0        
    fi
    return 1
}


if [ -e ${HOME}/bin/kvname ]; then
    KVNAME=`cat ${HOME}/bin/kvname`
    echo "Den som har stanset kvalobs er: $KVNAME"
   echo "Ønsker du å starte kvalobs?"
   yes_no
   if [ $? -ne 0 ] ; then
        echo "OK, du har valgt å starte kvalobs"
   else
      echo "OK, du vil ikke starte kvalobs - kvalobs blir ikke startet"
      exit 0
   fi
fi

echo " "
echo " "
echo "  Starter kvalobs dette kan ta noe tid!"
echo "  Hvis det ikke skjer noe på MER enn $TIMEOUT sekund"
echo "  bruk CTRL-C for å avbryte!"
echo " "
echo " "

for PROG in $START_PROGS ; do
    echo -n "Starter $PROG ...."

    isrunning $PROG
 
    if [ $? -eq 0 ]; then
	echo "running"
    else
	rm -f $KVPID/$PROG.pid

	$KVBIN/$PROG > /dev/null  2>&1 &
	
	n=0

        while [ $n -lt $TIMEOUT  -a ! -f "$KVPID/$PROG.pid" ]; do
	      let n=n+1
	      sleep 1
	done
 
	if [ -f "$KVPID/$PROG.pid" ]; then
	    echo "Ok!"
	else
	    echo "Failed!"
	fi
    fi
done

if [ -e ${HOME}/bin/kvname ]; then
    rm  ${HOME}/bin/kvname
fi
exit 0








