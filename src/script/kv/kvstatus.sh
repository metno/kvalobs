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
KVPID=`$KVCONFIG --localstatedir`/run/kvalobs
KVCONF=`$KVCONFIG --sysconfdir`/kvalobs
LIBDIR=$(kvconfig --pkglibdir)

if [ ! -f "$LIBDIR/tool_funcs.sh" ]; then
	echo "Cant load: $LIBDIR/tool_funcs.sh"
	exit 1
fi

. $LIBDIR/tool_funcs.sh

NODENAME=$(uname -n)

res=0
has_ip_alias=`ipalias_status` || res=$? 
case "$has_ip_alias" in
	true) echo "This node '$NODENAME' is the current kvalobs master!"
		  ;;
    test) echo "This node '$NODENAME' is an kvalobs test machine!"
          ;;
    *) echo
       echo "  This node '$NODENAME' is NOT the kvalobs master "
       echo "  or an test machine." 
	   echo 
	   exit 1
esac

if [ -e ${KVCONF}/kv_ctl.conf ]; then
    . ${KVCONF}/kv_ctl.conf
else
    echo "Missing file:  ${KVCONF}/kv_ctl.conf"
    exit 1
fi


progname=

if [ $# -ge 1 ]; then
	progname=$1
fi

function isrunning()
{
    prog=$1
   
    if [ -f $KVPID/$prog-$NODENAME.pid ]; then 
		PID=`cat $KVPID/$prog-$NODENAME.pid`
		#echo "PID: $PROG: $PID"
		kill  -0 $PID > /dev/null 2>&1

		if [ $? -eq 0 ]; then
	    	PIDS=`pgrep $prog 2>/dev/null`
	    	running=`echo $PIDS | grep $PID`
	    
	    	if [ ! -z "$running" ]; then
				return 0
	    	else
				rm -f $KVPID/$prog-$NODENAME.pid
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

for PROG in $START_PROGS ; do
	if [ "z$progname" != "z" ]; then
		if ! echo $PROG | grep $progname > /dev/null 2>&1 ; then  
			continue
		fi
	fi
	
	echo -n "$PROG .... "
	
	isrunning $PROG
 
    if [ $? -eq 0 ]; then
		echo "OK"
    else
    	echo "not running"
		rm -f $KVPID/$PROG-$NODENAME.pid
    fi
done

exit 0
