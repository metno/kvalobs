    #! /bin/sh

SQLDIR=`KVCONFIG --datadir`/kvalobs/db
ETCDIR=`KVCONFIG --sysconfdir`/kvalobs
LIBDIR=$(kvconfig --pkglibdir)

LOGDIR=`KVCONFIG --localstatedir`/log/kvalobs/kvdbadmin

if [ ! -d "$LOGDIR" ]; then
    mkdir -p $LOGDIR || (echo "Failed create directory: $LOGDIR"; exit 1)
fi

DAY=`date '+%d'`
LOG=$LOGDIR/kvdbadmin-$DAY.log

echo "Start: $(date)" >> $LOG
#date >> $LOG

if [ ! -f "$LIBDIR/tool_funcs.sh" ]; then
	echo "Cant load: $LIBDIR/tool_funcs.sh" | tee -a $LOG
	exit 1
fi

#Set logfile to LOG. This is used by the functions
#in tool_funcs to write log tsatments.
logfile=$LOG

. $LIBDIR/tool_funcs.sh

#Exit if the machines do NOT hold the ipalias or is an test machine.
ipalias_status > /dev/null || exit 0 


if [ -f $ETCDIR/pgclean-local.sql ]; then
    SQLCLEAN=$ETCDIR/pgclean-local.sql
else
    SQLCLEAN=$SQLDIR/pgclean.sql
fi

PSQL=psql

echo "Using clean script: $SQLCLEAN" >> $LOG 
$PSQL -U kvalobs kvalobs < $SQLCLEAN >> $LOG  2>&1
echo -n "Stop: " >> $LOG
date >> $LOG

rm -f $LOG.gz
gzip $LOG
