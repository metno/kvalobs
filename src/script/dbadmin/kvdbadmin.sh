#! /bin/sh
                                                                               
if [ -f $HOME/.kvalobs ]; then
   source $HOME/.kvalobs
else
   echo "Missing file: $HOME/.kvalobs"
   exit 1
fi
                                                                               
if [ ! -f $HOME/.kvpasswd ]; then
   echo "Missing file: $HOME/.kvpasswd"
   exit 1
fi

if [ -f $KVALOBS/etc/pgclean_locale.sql ]; then
    SQLCLEAN=$KVALOBS/etc/pgclean_locale.sql
else
    SQLCLEAN=$KVALOBS/share/kvdbadmin/pgclean.sql
fi

PSQL=psql
LOGDIR=$KVALOBS/var/log/kvdbadmin

export PGPASSWORD=`grep dbpass $HOME/.kvpasswd | sed -e 's/ *dbpass *//'`

mkdir -p $LOGDIR

DAY=`date '+%d'`
LOG=$LOGDIR/kvdbadmin-$DAY.log
echo -n "Start: " > $LOG
date >> $LOG
echo "Using clean script: $SQLCLEAN" >> $LOG 
$PSQL -U kvalobs kvalobs < $SQLCLEAN >> $LOG  2>&1
vacuumdb -U kvalobs -v -z kvalobs >> $LOG 2>&1
echo -n "Stop: " >> $LOG
date >> $LOG

rm -f $LOG.gz
gzip $LOG
