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

rm -f $TMP_OUTFILE
rm -f $ERROR_OUTFILE
rm -f ${LOG_FILE}_trunc

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

#echo "PGHOST: $PGHOST"
#echo "PGPORT: $PGPORT"
#echo "PGUSER: $PGUSER"

export PGHOST
export PGPORT
export PGUSER

(psql --no-password stinfosys <<EOF
\set ON_ERROR_STOP
\copy (select paramid,name,scalar from param order by paramid) to '${TMP_OUTFILE}' delimiter as ',';
EOF
)> ${ERROR_OUTFILE} 2>&1 

if [ $? -ne 0 -o ! -f ${TMP_OUTFILE} ]; then 
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

#echo "crcOldFile: --$crcOldFile-- "
#echo "crcNewFile: --$crcNewFile-- "

if [ "$crcOldFile" = "$crcNewFile" ]; then
    log "No change!"
else 
    mv $TMP_OUTFILE $OUTFILE
    log "New parameters defined. $OUTFILE was changed."
fi

rm -f $TMP_OUTFILE
rm -f $ERROR_OUTFILE
rm -f ${LOG_FILE}_trunc

exit 0
