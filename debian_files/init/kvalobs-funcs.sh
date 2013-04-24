#!/bin/bash

primary_host="kvalobs1 kvalobs2"
ipalias="/usr/local/bin/alias_kvalobs"

if [ ! -x "$ipalias" ]; then
	ipalias="/home/borgem/alias_kvalobsnew"
fi

#if [ "$TEST" = "true" ]; then
#	ipalias=true
#fi

#logfile=/var/log/kvalobs/kvalobs-init.log

if [ -f "/etc/kvalobs/kvalobs-init.conf" ]; then
	. /etc/kvalobs/kvalobs-init.conf
fi

#if [ -n "$PRIMARY_HOST" ]; then
#	primary_host="$PRIMARY_HOST"
#fi

#if [ -n "$KVALOBS_IPALIAS" ]; then
#	ipalias="$KVALOBS_IPALIAS"
#fi

log()
{
    [  "x$logfile" = "x" ] &&  echo "$@"

    echo "$@" >> $logfile
}


has_ip_alias()
{
    local me=`uname -n`
    local is_me=""
    local res=0

    for host in $primary_host; do 
		if [ "$me" = "$host" ];then
	    	is_me=$host
		fi
    done
    
    if [ -z "$is_me" ]; then
		echo "undef"
		return 0
    fi

    if [ ! -x "$ipalias" ]; then
		log "Cant find ipalias '$ipalias'"
		return 1
    fi

    if [ "x$logfile" = "x" ]; then
		$ipalias start > /dev/null 2>&1 || res=$? 
    else
		$ipalias start >> $logfile 2>&1 || res=$?  
    fi

    if [ "$res" = "2" -o "$res" = "0" ]; then
		log "This machine has the kvalobs ip alias." 
		echo "true"
    elif [ "$res" = "1" ]; then
		log "Another machine has the kvalobs ip alias."
		echo "false"
    else
		log "FAILED - kvalobs ip alias."
		return 1
    fi
}

stop_ip_alias()
{
    local res=0
    local mylog

    if [ -z "$logfile" ]; then
		mylog="/dev/null"
    else
		mylog="$logfile"
    fi
   
    if [ ! -x "$ipalias" ]; then
		echo "Cant find ipalias '$ipalias'" >> $mylog
		return 1
    fi

    $ipalias stop >> $mylog 2>&1 || res=$? 
    
    case "$res" in
		0) echo "kvalobs - IP alias deleted on this machine." >> $mylog ;;
		1) echo "kvalobs - IP alias not running on this machine." >> $mylog ;;
		*) echo "kvalobs - IP alias ($res)" >> $mylog 
           return 1 ;;
    esac
}


#This function assume that we run as user kvalobs
start_as_master()
{
    local mylog
    local res=0


    if [ -z "$logfile" ]; then
		mylog="/dev/null"
    else
		mylog="$logfile"
    fi
    
    echo "USER: $USER" >>  $mylog
    echo "HOME: $HOME" >>  $mylog
    echo "PATH: $PATH" >>  $mylog
    
    pgrepctl-kvalobs --start-as-master >> $mylog 2>&1 || res=$?

    if [ "$res" = "0" ]; then
		echo "kvalobs - Started postgresql on this server as master."  | tee -a $mylog
    else
		echo "kvalobs - Failed starting postgresql on this server as master." | tee -a $mylog
		return 1
    fi

    kvstart || res=$?
    
    if [ "$res" = "0" ]; then
		echo "kvalobs - started."  | tee -a $mylog
    else
		echo "kvalobs - Failed to start kvalobs." | tee -a $mylog
		return 1
    fi
}

#This function assume that we run as user kvalobs
start_as_slave()
{
    local mylog
    local res=0

    if [ -z "$logfile" ]; then
		mylog="/dev/null"
    else
		mylog=$logfile
    fi
    
    pgrepctl-kvalobs --start-as-slave >> $mylog 2>&1 || res=$?

    if [ "$res" = "0" ]; then
		echo "kvalobs - Started postgresql on this server as an slave." | tee -a $mylog
    else
		echo "kvalobs - Failed starting postgresql on this server as an slave." | tee -a $mylog
		return 1
    fi
}

stop_kvalobs()
{
    local mylog
    local res=0
    local cmd

    if [ -z "$logfile" ]; then
		mylog="/dev/null"
    else
		mylog=$logfile
    fi

    if [ $# -le 1 ]; then
		cmd="cluster"
    else
		cmd="$1"
    fi
    
    kvstop || res=$?

    res=0

    case "$1" in
		slave) pgrepctl-kvalobs --stop >> $mylog 2>&1 || res=$? ;;
		cluster) pgrepctl-kvalobs --stop-cluster >> $mylog 2>&1 || res=$? ;;
		*) return 1 ;;
    esac

    if [ "$res" = "0" ]; then
		echo "kvalobs - Stopped kvalobs. ($cmd)" | tee -a $mylog
    else
		echo "kvalobs - Failed to stop kvalobs. ($cmd)" | tee -a $mylog
    fi
}

