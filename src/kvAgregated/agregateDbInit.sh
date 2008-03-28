#!/bin/sh

VARDIR=$1
if [ ! -d $VARDIR ]; then
	mkdir $VARDIR
fi  

mkdir -p $VARDIR/agregate
DATABASE=$VARDIR/agregate/database.sqlite
if [ -f $DATABASE ]; then
	rm $DATABASE
fi

sqlite $DATABASE "CREATE TABLE data (stationid INTEGER NOT NULL,obstime TIMESTAMP NOT NULL,original FLOAT NOT NULL,paramid INTEGER NOT NULL,tbtime TIMESTAMP NOT NULL,typeid INTEGER NOT NULL,sensor CHAR(1) DEFAULT '0',level INTEGER DEFAULT 0,corrected FLOAT NOT NULL,controlinfo CHAR(16) DEFAULT '0000000000000000',useinfo CHAR(16) DEFAULT '0000000000000000',cfailed TEXT DEFAULT NULL,UNIQUE ( stationid, obstime, paramid, level, sensor, typeid ));"
