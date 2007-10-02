#! /bin/sh
                                                                               
if [ -f $HOME/.kvalobs ]; then
   source $HOME/.kvalobs
else
   echo "Missing file: $HOME/.kvalobs"
   exit 1
fi
                                                                               

SQL=sqlite
LOGDIR=$KVALOBS/var/log/kvsynopdbadmin
DBFILE=$KVALOBS/var/kvsynop/kvsynopd.sqlite
mkdir -p $LOGDIR

DAY=`date '+%d'`
LOG=$LOGDIR/kvsynopdb-$DAY.log

echo -n "Start: " > $LOG
date >> $LOG
echo "--------------------------------------------------" >> $LOG 
$SQL $DBFILE  < $KVALOBS/share/kvsynop/cleansynopdb.sql >> $LOG  2>&1
echo "--------------------------------------------------" >> $LOG 
echo -n "Stop: " >> $LOG
date >> $LOG
