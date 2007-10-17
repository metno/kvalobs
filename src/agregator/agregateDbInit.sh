#!/bin/sh

cd $KVALOBS/var

if [ ! $? ]; then
    echo "Cant change directory to $KVALOBS/var!"
    echo "Solve the problem and try again!"
    exit 1
fi


pwd
mkdir -p agregate
cd agregate

if [ ! $? ]; then
    echo "Cant change directory to $KVALOBS/var/agregate!"
    echo "Solve the problem and try again!"
    exit 1
fi

pwd

sqlite database.sqlite "CREATE TABLE data (stationid INTEGER NOT NULL,obstime TIMESTAMP NOT NULL,original FLOAT NOT NULL,paramid INTEGER NOT NULL,tbtime TIMESTAMP NOT NULL,typeid INTEGER NOT NULL,sensor CHAR(1) DEFAULT '0',level INTEGER DEFAULT 0,corrected FLOAT NOT NULL,controlinfo CHAR(16) DEFAULT '0000000000000000',useinfo CHAR(16) DEFAULT '0000000000000000',cfailed TEXT DEFAULT NULL,UNIQUE ( stationid, obstime, paramid, level, sensor, typeid ));"
