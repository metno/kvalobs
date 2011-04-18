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
NODENAME=$(uname -n)
KVSTART=$KVBIN/kvstart
KVSTOP=$KVBIN/kvstop
LOG=`$KVCONFIG --localstatedir`/log/kvalobs/kvrestart.log
silent=false
force=false

function use( )
{
	local ret=1
	if [ $# -ge 1 ]; then
		ret = $1
	fi
	
	echo " kvrestart [-s] [-f] [-h] progname"
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
    echo " -s Ingen output til skjerm."
    echo " -f Tving oppstart av programmet hvis det er stoppet."
    echo " -h Skriv denne hjelpe skjermen."
    echo " "
    
    exit $ret
}

function log()
{
	local runTime=$(date +'%Y-%m-%d %H:%M:%S')
	
	if [ "$silent" = "false" ]; then
		echo $1
	fi
	
	if [ -f $LOG ]; then
		size=$(stat -c '%s' $LOG)
	
		if [ $size -gt 10240 ]; then
			mv $LOG $LOG.tmp
			tail -n 10 $LOG.tmp > $LOG
			newSize=$(stat -c '%s' $LOG)
			runTime=$(date +'%Y-%m-%d %H:%M:%S')
			echo "$runTime - Logfile truncated - size before truncate $size new size $newSize." >> $LOG
			rm -f $LOG.tmp
		fi
	fi

	echo "$runTime - $1" >> $LOG
}


if [ -e ${KVCONF}/kv_ctl.conf ]; then
    . ${KVCONF}/kv_ctl.conf
else
    log "Filen ${KVCONF}/kv_ctl.conf finnes ikke."
    use
fi

if [ "$USER" != "$KVUSER" -o "$LOGNAME" != "$KVUSER"]; then
   log "Bare kvalobs brukeren '$KVUSER' kan starte/stoppe kvalobs komponenter."
   log "Du er logget inn som bruker '$USER'."
   exit 1
fi


#echo "KVBIN=$KVBIN"
#echo "KVPID=$KVPID"
#echo "TIMEOUT=$TIMEOUT"


while getopts sfh f
do
  case $f in
    s) silent=true;;
    f) force=true;;
    h) use 0;;
    \?) use 1;;
  esac
done

shift $((OPTIND-1))                                                                                                                                      

if [ $# -lt 1 ]; then                                                                                                                                    
    use 1                                                                                                                                      
fi

progname=$1

if [ $silent = false ]; then
	echo " "
	echo "  Stopper '$progname' dette kan ta noe tid!"
	echo "  Hvis det ikke skjer noe på MER enn $TIMEOUT sekund"
	echo "  bruk CTRL-C for å avbryte!"
	echo " "
fi

found=false;
for PROG in $STOP_PROGS ; do
	if [ "z$progname" != "z" ]; then
		if ! echo $PROG | grep $progname > /dev/null 2>&1 ; then  
			continue
		fi
	fi

	found=true
	
	if [ $force = false -a -f "$KVPID/$PROG-$NODENAME.stopped" ]; then
	    runTime=$(date +'%Y-%m-%d %H:%M:%S')
		log "'$PROG' er stoppet av administrator."
		continue
	fi

   	$KVSTOP $PROG > /dev/null 2>&1
      		
   	if [ $? -eq 0 ]; then 
   		$KVSTART $PROG > /dev/null 2>&1

   		if [ $? -eq 0 ]; then
   			runTime=$(date +'%Y-%m-%d %H:%M:%S')
   			log "RESTARTET '$PROG'."
   		else
   		    runTime=$(date +'%Y-%m-%d %H:%M:%S') 
   			log "FEILET - Kan ikke starte $PROG."
   		fi
	else 
	    runTime=$(date +'%Y-%m-%d %H:%M:%S')
		log "FEILET - Kan ikke stoppe $PROG."
	fi
done

if [ $found = false ]; then
	log "$progname er ikke spesifisert i STOP_PROGS ${KVCONF}/kv_ctl.conf."
fi

exit 0
