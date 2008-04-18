#! /bin/sh

SQLDIR=`KVCONFIG --datadir`/kvalobs/db

if [ -f $SQLDIR/pgclean_locale.sql ]; then
    SQLCLEAN=$SQLDIR/pgclean_locale.sql
else
    SQLCLEAN=$SQLDIR/pgclean.sql
fi

PSQL=psql
LOGDIR=`KVCONFIG --localstatedir`/kvalobs/log

DAY=`date '+%d'`
LOG=$LOGDIR/kvdbadmin-$DAY.log
echo -n "Start: " > $LOG
date >> $LOG
echo "Using clean script: $SQLCLEAN" >> $LOG 
$PSQL -U kvalobs kvalobs < $SQLCLEAN >> $LOG  2>&1
echo -n "Stop: " >> $LOG
date >> $LOG

rm -f $LOG.gz
gzip $LOG
