#! /bin/sh

INCOMMING="/var/lib/kvalobs/synopreports_tmp"
DEST="/var/lib/kvalobs/synopreports"
LOGDIR="/var/log/kvalobs/rcvs_synopreports"
LOGFILE="$LOGDIR/lastlog.log"

mkdir -p $DEST
mkdir -p $LOGDIR

if [ ! -d "$LOGDIR" ]; then 
	echo "FATAL: Logdir '$LOGDIR' do not exists."
	exit 1
fi 

if [ ! -d "$DEST" ]; then 
	echo "FATAL: Destdir '$DEST' do not exists."
	exit 1
fi 


if [ ! -d "$INCOMMING" ]; then 
	echo "FATAL: Incommingdir '$INCOMMING' do not exists."
	exit 1
fi 

start_time=`date +'%Y-%m-%d %H:%M:%S'`
echo "Start: $start_time" > $LOGFILE

handshakelist=`ls -1 $INCOMMING/handshake.* 2> /dev/null`

#echo "handshakelist: $handshakelist"

for handshake in $handshakelist; do
	handshake=`echo $handshake` #trim 
	
	if [ "${handshake}z" = "z" ]; then
 		continue
 	fi

	touch ${handshake}_new
done

sleep 5

handshakelist=`ls -1 $INCOMMING/handshake.*_new 2> /dev/null`

for handshake in $handshakelist; do
	handshake=`echo $handshake` #trim 
	
	if [ "${handshake}z" = "z" ]; then
 		continue
 	fi
	
	fname=`echo $handshake | sed s/\_new//`
	
#	echo "CMP: $handshake -nt $fname"
	
	if [ $fname -nt $handshake ]; then
		rm -f $handshake
	    continue
	fi
	
	rm -f $handshake
	
	echo "New synopreports: $fname" >> $LOGFILE
	
	for filein in `cat $fname`; do
		file=`echo $filein | cut -d. -f1`
		file=`basename $file`
		echo "MV: $INCOMMING/$filein $DEST/$file" >> $LOGFILE 
		mv $INCOMMING/$filein $DEST/$file
	done
	rm -f $fname
done

stop_time=`date +'%Y-%m-%d %H:%M:%S'`
echo "Completed: $stop_time" >> $LOGFILE

