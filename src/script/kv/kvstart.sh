#!/bin/dash
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
KVPID=`$KVCONFIG --rundir`
KVCONF=`$KVCONFIG --sysconfdir`/kvalobs
LIBDIR=`$KVCONFIG --pkglibdir`
LOGDIR=`$KVCONFIG --logdir`

if [ ! -f "$LIBDIR/tool_funcs.sh" ]; then
	echo "Cant load: $LIBDIR/tool_funcs.sh"
	exit 1
fi

. $LIBDIR/tool_funcs.sh

NODENAME=$(uname -n)

is_master=false
res=0
has_ip_alias=`ipalias_status` || res=$? 

case "$has_ip_alias" in
   true) echo "This node '$NODENAME' is the current kvalobs master!"
         is_master=true
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

if [ "$USER" != "$KVUSER" ]; then
   echo "Only the '$KVUSER' user my start kvalobs."
   echo "You are logged in as user '$USER'"
   exit 1
fi

if [ $is_master = "true" -o -f "$KVCONF/stinfosys.conf"  ]; then
   kv_get_stinfosys_params -n
   if [ $? -ne 0 ]; then
	echo "Failed to look up the 'parameter definitions' from stinfosys."
	echo "Check the log '$LOGDIR/kv_get_stinfosys_params.log' for more information."
   fi
else 
   echo "WARNING the configurationfile ${KVCONF}/stinfosys.conf" 
   echo "is missing so kv_get_stinfosys_params cant be run to generate"
   echo "the file $KVCONF/stinfosys_params.csv."
   echo "This file is used by kvDataInputd to decide"
   echo "if a parameter is for the 'data' or for the 'text_data' table."
   echo "For more information run 'kv_get_stinfosys_params -h'."
fi

progname=

if [ $# -ge 1 ]; then
	progname=$1
fi

echo "KVBIN=$KVBIN"
echo "KVPID=$KVPID"
echo "TIMEOUT=$TIMEOUT"

yes_no()
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


if [ -e ${KVCONF}/kvname ]; then
    KVNAME=`cat ${KVCONF}/kvname`
    echo "Den som har stanset kvalobs er: $KVNAME"
   echo "ønsker du å starte kvalobs?"
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
	if [ "z$progname" != "z" ]; then
		if ! echo $PROG | grep "^$progname" > /dev/null 2>&1 ; then  
			continue
		fi
	fi
	
	echo -n "Starter $PROG ...."
	
	rm -f $KVPID/$PROG-$NODENAME.stopped
   isProgRunning $PROG
 
    if [ $? -eq 0 ]; then
	echo "running"
    else
	rm -f $KVPID/$PROG-$NODENAME.pid

	$KVBIN/$PROG > /dev/null  2>&1 &
	
	n=0

        while [ $n -lt $TIMEOUT  -a ! -f "$KVPID/$PROG-$NODENAME.pid" ]; do
	      n=$((n+1))
	      sleep 1
	done
 
	if [ -f "$KVPID/$PROG-$NODENAME.pid" ]; then
	    echo "Ok!"
	else
	    echo "Failed!"
	fi
    fi
done

if [ -e ${KVCONF}/kvname ]; then
    rm  ${KVCONF}/kvname
fi

exit 0








