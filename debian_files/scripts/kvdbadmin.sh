#! /bin/sh
                                                                               
ETCDIR=$(kvconfig --sysconfdir)/kvalobs
DATADIR=$(kvconfig --datadir)/kvalobs

if [ -f "$ETCDIR/kv-env.conf" ]; then
    . $ETCDIR/kv-env.conf
fi

if [ -f $ETCDIR/pgclean_locale.sql ]; then
    SQLCLEAN=$ETCDIR/pgclean_locale.sql
else
    SQLCLEAN=$DATADIR/db/pgclean.sql
fi

PSQL=psql
LOGDIR=$(kvconfig --localstatedir)/kvalobs/log/kvdbadmin

mkdir -p $LOGDIR

DAY=`date '+%d'`
LOG=$LOGDIR/kvdbadmin-$DAY.log
echo -n "Start: " > $LOG
date >> $LOG
## PS 2008-03-25 Stopped deleting in database contemporarily
##echo "Using clean script: $SQLCLEAN" >> $LOG 
##$PSQL -U kvalobs kvalobs < $SQLCLEAN >> $LOG  2>&1
## PS 2008-12-12 Added. Remove when lines above are uncommented
SQLCLEAN_MINI=$DATADIR/db/pgclean_mini.sql
echo "Using (mini) clean script: $SQLCLEAN_MINI" >> $LOG 
$PSQL -U kvalobs kvalobs < $SQLCLEAN_MINI >> $LOG  2>&1

#In postgresql >= 8.0 we dont need to run vacuumdb 
#vacuumdb -U kvalobs -v -z kvalobs >> $LOG 2>&1
echo -n "Stop: " >> $LOG
date >> $LOG

rm -f $LOG.gz
gzip $LOG
