#! /bin/sh

SQL=sqlite3
SQLDIR=`KVCONFIG --datadir`/kvalobs/db
LOGDIR=`KVCONFIG --localstatedir`/kvalobs/log
DBFILE=`KVCONFIG --localstatedir`/kvalobs/kvsynopd.sqlite

DAY=`date '+%d'`
LOG=$LOGDIR/kvsynopdb-$DAY.log

echo -n "Start: " > $LOG
date >> $LOG
echo "--------------------------------------------------" >> $LOG 
$SQL $DBFILE  < $SQLDIR/cleansynopdb.sql >> $LOG  2>&1
echo "--------------------------------------------------" >> $LOG 
echo -n "Stop: " >> $LOG
date >> $LOG
