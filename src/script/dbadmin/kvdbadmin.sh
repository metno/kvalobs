#! /bin/sh

SQLDIR=`KVCONFIG --datadir`/kvalobs/db
ETCDIR=`KVCONFIG --sysconfdir`/kvalobs
LIBDIR=$(kvconfig --pkglibdir)

if [ ! -f "$LIBDIR/tool_funcs.sh" ]; then
	echo "Cant load: $LIBDIR/tool_funcs.sh"
	exit 1
fi

. $LIBDIR/tool_funcs.sh

#Exit if the machines do NOT hold the ipalias or is an test machine.
ipalias_status > /dev/null || exit 0 


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
