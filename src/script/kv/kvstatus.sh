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

#KVCONFIG=__KVCONFIG__
KVCONFIG=/usr/bin/kvconfig

KVBIN=`$KVCONFIG --bindir`
KVPID=`$KVCONFIG --rundir`
KVCONF=`$KVCONFIG --sysconfdir`/kvalobs
LIBDIR=`$KVCONFIG --pkglibdir`

if [ ! -f "$LIBDIR/tool_funcs.sh" ]; then
	echo "Cant load: $LIBDIR/tool_funcs.sh" >&2
	exit 1
fi

. $LIBDIR/tool_funcs.sh

NODENAME=$(uname -n)

#status codes
STATUS_OK=" .... OK"
STATUS_FAILED=" .... not running"
STATUS_STOPPED=" .... stopped"
SCRIPT=false

use( )
{
	local ret=1
	if [ $# -ge 1 ]; then
		ret = $1
	fi

  echo " kvstatus [-h] [-s] [progname]"
  echo " "
  echo " Tester om alle prosessene i kvalobs kjører. Kan også teste om"
	echo " et enkelt program 'progname' kjører."
	echo " Resultatet er en liste på formen: "
	echo " "
	echo " progname .... status"
	echo " "
	echo " Hvor progname er en av programmene definert i START_PROGS i"
	echo " filen ${KVCONF}/kv_ctl.conf. "
  echo " "
	echo " status er gitt med: OK, not running, stopped."
	echo "  - OK, programmet kjører."
	echo "  - not rinning, programmet kjører ikke, mest sansynlig"
	echo "    så har det krasjet."
	echo "  - stopped, programmet er stoppet av operatør med kvstop."
	echo " "
	echo " Options: "
	echo "  -h Skriv denne hjelpe skjermen."
	echo "  -s En mer skript venlig status liste på formen: "
	echo "     progname status"
	echo "     Og status er: OK, FAILED, STOPPED"
	echo "     "
	echo " Exit koden er 0 hvis alt er ok, dvs alle programmene kjøre."
	echo " Exit kode 2 hvis noen av programmene ikke kjører (kræsjet)."
	echo " Exit kode 1 hvis andre feil har oppstått"
	echo " "
  exit $ret
}

res=0
has_ip_alias=`ipalias_status` || res=$?
case "$has_ip_alias" in
	true) echo "This node '$NODENAME' is the current kvalobs master!" >&2
		  ;;
    test) echo "This node '$NODENAME' is an kvalobs test machine!" >&2
          ;;
    *) echo >&2
       echo "  This node '$NODENAME' is NOT the kvalobs master " >&2
       echo "  or an test machine."  >&2
	   echo  >&2
	   exit 1
esac

if [ -e ${KVCONF}/kv_ctl.conf ]; then
    . ${KVCONF}/kv_ctl.conf
else
    echo "Missing file:  ${KVCONF}/kv_ctl.conf" >&2
    exit 1
fi

while getopts sh f
do
  case $f in
    s) STATUS_OK=" OK"
       STATUS_FAILED=" FAILED"
		   STATUS_STOPPED=" STOPPED"
			 ;;
    h) use 0;;
    \?) use 1;;
  esac
done
shift `expr $OPTIND - 1`

progname=

if [ $# -ge 1 ]; then
	progname=$1
fi

ret=0

for PROG in $START_PROGS ; do
	if [ "z$progname" != "z" ]; then
		if ! echo $PROG | grep "^$progname" > /dev/null 2>&1 ; then
			continue
		fi
	fi

	echo -n " -- $PROG"

	isProgRunning $PROG

  if [ $? -eq 0 ]; then
		echo "$STATUS_OK"
  elif [ -f "$KVPID/$PROG-$NODENAME.stopped" ]; then
		echo "$STATUS_STOPPED"
		rm -f $KVPID/$PROG-$NODENAME.pid
	else
  	echo "$STATUS_FAILED"
		rm -f $KVPID/$PROG-$NODENAME.pid
		ret=2
  fi
done

exit $ret
