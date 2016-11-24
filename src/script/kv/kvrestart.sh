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
NODENAME=$(uname -n)
KVSTART=$KVBIN/kvstart
KVSTOP=$KVBIN/kvstop
logfile=`$KVCONFIG --localstatedir`/log/kvalobs/kvrestart.log
LIBDIR=`$KVCONFIG --pkglibdir`

if [ ! -f "$LIBDIR/tool_funcs.sh" ]; then
	echo "Cant load: $LIBDIR/tool_funcs.sh"
	exit 1
fi

#SILENT er brukt av log i tool_funcs
SILENT=true
DOLOG=false
. $LIBDIR/tool_funcs.sh

SILENT=false
force=false
only_start_if_stopped=false
DOLOG=true

use( )
{
	local ret=1
	if [ $# -ge 1 ]; then
		ret = $1
	fi
	
    echo " kvrestart [-s] [-f] [-h] [-r] [-n] progname"
    echo " "
    echo " Omstart av programmet angitt med 'progname'."
    echo " Hvis filen $KVPID/'progname'-$NODENAME.stopped eksisterer"
    echo " vil ikke programmet bli restartet. Det antas at programmet"
    echo " er stoppet av system administrator. Filen lages av 'kvstop'"
    echo " og slettes av kvstart. Denne oppførselen kan overstyres av -f."
    echo " "
    echo " 'progname' må være en av programmene definert i START_PROGS i filen"
    echo " ${KVCONF}/kv_ctl.conf. "
    echo " "
    echo " kvrestart skriver til logfilen $LOG."
    echo " "
    echo " -r Start programmet dersom det ikke kjører, ikke stop det først."
    echo " -s Ingen output til skjerm."
    echo " -n Ikke log noe dersom det ikke er problemer"
    echo " -f Tving oppstart av programmet hvis det er stoppet."
    echo " -h Skriv denne hjelpe skjermen."
    echo " "
    
    exit $ret
}


if [ -e ${KVCONF}/kv_ctl.conf ]; then
    . ${KVCONF}/kv_ctl.conf
else
    logerror "Filen ${KVCONF}/kv_ctl.conf finnes ikke."
    use
fi

if [ "$USER" != "$KVUSER" -a "$LOGNAME" != "$KVUSER" ]; then
   logerror "Bare kvalobs brukeren '$KVUSER' kan starte/stoppe kvalobs komponenter."
   logerror "Du er logget inn som bruker '$USER'."
   exit 1
fi


#echo "KVBIN=$KVBIN"
#echo "KVPID=$KVPID"
#echo "TIMEOUT=$TIMEOUT"


while getopts sfrnh f
do
  case $f in
    s) SILENT=true;;
    f) force=true;;
    r) only_start_if_stopped=true;;
    n) DOLOG=false ;;
    h) use 0;;
    \?) use 1;;
  esac
done


#Exit if the machines do NOT hold the ipalias or is an test machine.
ipalias_status > /dev/null || exit 0 


shift $((OPTIND-1))                                                                                                                                      

if [ $# -lt 1 ]; then                                                                                                                                    
    use 1                                                                                                                                      
fi

progname=$1

if [ $SILENT = false -a $only_start_if_stopped = false ]; then
	echo " "
	echo "  Stopper '$progname' dette kan ta noe tid!"
	echo "  Hvis det ikke skjer noe på MER enn $TIMEOUT sekund"
	echo "  bruk CTRL-C for å avbryte!"
	echo " "
fi



found=false;
for PROG in $STOP_PROGS ; do
	if [ "z$progname" != "z" ]; then
      if ! echo $PROG | grep "^$progname" > /dev/null 2>&1 ; then  
			continue
		fi
	fi

	found=true

	if [ $force = false -a -f "$KVPID/$PROG-$NODENAME.stopped" ]; then
		log "'$PROG' er stoppet av administrator."
		continue
	fi

	if [ $only_start_if_stopped = false ]; then
      $KVSTOP $PROG > /dev/null 2>&1
   else
      #Set the exit status to $? to 0.
	   true
   fi
      		
   if [ $? -eq 0 ]; then 
      running=false
      if isProgRunning $PROG ; then
         running=true
      fi      

      $KVSTART $PROG > /dev/null 2>&1

      if [ $? -eq 0 ]; then
         if [ $only_start_if_stopped = true ]; then
            if [ $running = false ]; then
               loginfo "STARTET '$PROG'."
            else 
               log "RUNNING '$PROG'." $DOLOG
            fi
         else
            loginfo "RESTARTET '$PROG'."
         fi
      else
         logerror "FEILET - Kan ikke starte $PROG."
      fi
	else 
	   logerror "FEILET - Kan ikke stoppe $PROG."
	fi
done

if [ $found = false ]; then
	logerror "$progname er ikke spesifisert i STOP_PROGS ${KVCONF}/kv_ctl.conf."
fi

exit 0
