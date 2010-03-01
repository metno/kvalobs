#! /bin/sh
                                                                               
VARDIR=$(kvconfig --localstatedir)/kvalobs                                                                       DATADIR=$(kvconfig --datadir)/kvalobs        

SQL=sqlite3

LOGDIR=$VARDIR/log/kvsynopdbadmin
DBFILE=$VARDIR/kvsynop/kvsynopd.sqlite
mkdir -p $LOGDIR

DAY=`date '+%d'`
LOG=$LOGDIR/kvsynopdb-$DAY.log

echo -n "Start: " > $LOG
date >> $LOG
echo "--------------------------------------------------" >> $LOG 
$SQL $DBFILE  < $DATADIR/db/cleansynopdb.sql >> $LOG  2>&1
echo "--------------------------------------------------" >> $LOG 
echo -n "Stop: " >> $LOG
date >> $LOG
