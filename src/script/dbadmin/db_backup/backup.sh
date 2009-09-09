#!/bin/sh
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

( /usr/local/sbin/alias_list | /bin/grep -q kvalobs ) || exit 0

PGPORT=`grep PGPORT $HOME/.bashrc|cut -f2 -d=`
#echo $PGPORT
#source $HOME/.bashrc

KVALOBS=$HOME
DIR=$KVALOBS/db_backup

if [ -d $DIR ]; then
    # echo "OK $DIR"
    cd $DIR
else
   echo "Missing directory: $DIR"
   exit 1
fi

/usr/lib/postgresql/8.3/bin/pg_dump -h localhost -p $PGPORT -U kvalobs kvalobs | gzip -9 > "kvalobs.`date +'%Y-%m-%d'`.gz"

#-----------------------------------
# DAYS means to delete files older than this number of days
DAYS=+64
#-----------------------------------
find $DIR -name 'kvalobs.*.gz' -type f -mtime +7 -exec rm -f {} \;
$KVALOBS/bin/db_backup/error_backup.pl    $DIR

