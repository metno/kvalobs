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


KVCONF=`$KVCONFIG --sysconfdir`/kvalobs
KVLOG=`$KVCONFIG --logdir`
LOG_FILE=${KVLOG}/kv_get_stinfosys_params.log
STINFOSYS=${KVCONF}/stinfosys.conf
OUTFILE=$KVCONF/stinfosys_params.csv
TMP_OUTFILE=${OUTFILE}_tmp
ERROR_OUTFILE=${OUTFILE}_error




function use()
{
    echo ""
    echo " kv_get_stinfosys_params [-h] [-n] [-d] [-P pgport] [-U pguser] [-H pghost]"
    echo ""
    echo " kv_get_stinfosys_params connect to the database 'stinfosys' and"
    echo " reads the parameter definition and save them in the file"
    echo " $OUTFILE. This file is used by kvDataInputd to decide"
    echo " if a parameter is for the 'data' or for the 'text_data' table."
    echo ""
    echo " The program reads the configuration file $STINFOSYS"
    echo " and ~/.pgpass. On success the program generate the file"
    echo " $OUTFILE."
    echo ""
    echo " The program writes to the logfile $LOG_FILE."
    echo " In case of errors consult the log file to get a clue "
    echo " on what the problem may be."
    echo ""
    echo " The exit status to the program i 0 on success an 1 on failure ie."
    echo " it could not create or update the $OUTFILE"
    echo " for some reason."
    echo ""
    echo "  -n Do not ask for the passord to connect to 'stinfosys' database."
    echo "     For this to work all necessary connection information must"
    echo "     be given in the configuration file $STINFOSYS"
    echo "     and the password must be set in ~/.pgpass. Remeber that"
    echo "     the file .pgpass must have the permission 0600 otherwise"
    echo "     it will be ignored by postgres." 
    echo "  -d Run i debug mode."
    echo "  -P pgport, override PGPORT in $STINFOSYS."
    echo "  -H pghost, override PGHOST in $STINFOSYS."
    echo "  -U pguser, override PGUSER in $STINFOSYS." 
    echo "  -h Write this help screen and exit!"
    echo " "
    echo " The format to the configuration file '$STINFOSYS"
    echo " is key=val separated by new lines. Remeber no space"
    echo " between the = character and the 'key' and 'val'."
    echo " Valid keys is PGPORT, PGHOST and PGUSER. Lookup in the"
    echo " postgres docummentation if you don't now what they means (man psql)."
    echo ""
    exit 1
}


debug=false
no_password=""
pgport=""
pghost=""
pguser=""
listopt=0
OPTERR=0
getopts "hndP:U:H:" opt
optret=$?

while [ $optret -eq 0 ]; do
    case $opt in
        P) pgport=$OPTARG;;
        H) pghost=$OPTARG;;
        U) pguser=$OPTARG;;
        d) debug=true;;
        n) no_password="--no-password";;
        h) use;;
        \?) echo "Invalid option: $OPTARG"; use;;
        *) echo "Unexpected option $opt!"; use;;
    esac
    getopts "hndP:U:H:" opt
    optret=$?
done


if [ $debug="false" ]; then
    rm -f $TMP_OUTFILE
    rm -f $ERROR_OUTFILE
    rm -f ${LOG_FILE}_trunc
fi

touch $LOG_FILE

if [ -f $LOG_FILE ]; then
    #Truncate the logfile
    tail -n 50 $LOG_FILE > ${LOG_FILE}_trunc
    mv ${LOG_FILE}_trunc ${LOG_FILE}
fi

log()
{
    local t=""
    t=`date +'%Y-%m-%d %H:%M:%S'`
    echo "$t - $@" >> $LOG_FILE
}

if [ ! -f ${STINFOSYS} ]; then
    log "No '$STINFOSYS' file!"
    exit 1
fi

#Set PGHOST, PGPORT and PGUSER from the conf file.
. ${STINFOSYS}


if [ -n "$pgport" ]; then
    PGPORT=$pgport
fi

if [ -n "$pghost" ]; then
    PGHOST=$pghost
fi

if [ -n "$pguser" ]; then
    PGUSER=$pguser
fi

if [ $debug = true ]; then
    echo "PGHOST: $PGHOST"
    echo "PGPORT: $PGPORT"
    echo "PGUSER: $PGUSER"
    echo "no_password: $no_password" 
fi

export PGHOST
export PGPORT
export PGUSER


(psql $no_password stinfosys <<EOF
\set ON_ERROR_STOP
\copy (select paramid,name,scalar from param order by paramid) to '${TMP_OUTFILE}' delimiter as ',';
EOF
)> ${ERROR_OUTFILE} 2>&1 

if [ $? -ne 0 -o ! -f ${TMP_OUTFILE} ]; then 
    if [ -z "$no_password" ]; then
        cat ${ERROR_OUTFILE}
    fi
    log "Failed to get the parameter definition from stinfosys."
    log "The error message was."
    log "---------------------------"
    cat ${ERROR_OUTFILE} >> $LOG_FILE
    log "---------------------------"
    log "The file '$OUTFILE' was NOT changed."
    exit 1
fi

if [ ! -f $OUTFILE ]; then
    mv $TMP_OUTFILE $OUTFILE
    log "The $OUTFILE was created!"
    
    exit 0
fi

crcOldFile=`md5sum $OUTFILE | cut -f1 -d' '`
crcNewFile=`md5sum $TMP_OUTFILE | cut -f1 -d' '`

if [ $debug = true ]; then
    echo "crcOldFile: $crcOldFile"
    echo "crcNewFile: $crcNewFile"
fi

if [ "$crcOldFile" = "$crcNewFile" ]; then
    #Use touch to update the modification time så 
    #we can use this time to check when we last 
    #checked stinfosys for parameter information.
    touch $OUTFILE
    log "No change!"
else 
    mv $TMP_OUTFILE $OUTFILE
    log "New parameters defined. $OUTFILE was changed."
fi

if [ $debug = false ]; then
    rm -f $TMP_OUTFILE
    rm -f $ERROR_OUTFILE
    rm -f ${LOG_FILE}_trunc
fi

exit 0
