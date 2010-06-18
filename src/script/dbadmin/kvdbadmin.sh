#! /bin/sh

SQLDIR=`KVCONFIG --datadir`/kvalobs/db
ETCDIR=`KVCONFIG --sysconfdir`/kvalobs

if [ -f $ETCDIR/pgclean-local.sql ]; then
    SQLCLEAN=$ETCDIR/pgclean-local.sql
else
    SQLCLEAN=$SQLDIR/pgclean.sql
fi

PSQL=psql
LOGDIR=`KVCONFIG --localstatedir`/log/kvalobs

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
