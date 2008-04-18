#! /bin/sh

SQL=sqlite
SQLDIR=`kvconfig --datadir`/kvalobs/db
LOGDIR=`kvconfig --localstatedir`/kvalobs/log
DBFILE=`kvconfig --localstatedir`/kvalobs/kvsynopd.sqlite

DAY=`date '+%d'`
LOG=$LOGDIR/kvsynopdb-$DAY.log

echo -n "Start: " > $LOG
date >> $LOG
echo "--------------------------------------------------" >> $LOG 
$SQL $DBFILE  < $SQLDIR/cleansynopdb.sql >> $LOG  2>&1
echo "--------------------------------------------------" >> $LOG 
echo -n "Stop: " >> $LOG
date >> $LOG
