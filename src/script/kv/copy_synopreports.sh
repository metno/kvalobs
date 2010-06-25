#! /bin/sh

SOURCE=/dnmi/norcom/data/kvalobs
DEST="kvalobs@dev-vm101:/var/lib/kvalobs/synopreports_tmp"
LOGDIR=/var/log/kvalobs/copy_synopreports

mkdir -p $LOGDIR

if [ ! -d "$LOGDIR" ]; then 
	echo "FATAL: Logdir '$LOGDIR' do not exists."
	exit 1
fi 

ctlfilenew="$LOGDIR/copyctl.new"
ctlfileold="$LOGDIR/copyctl.old"


if [ ! -f "$ctlfileold" ]; then
	mv $ctlfilenew $ctlfileold
	echo "Start copy: $timestamp" > $LOGDIR/lastcopy.log
	echo "Restart!"  >> $LOGDIR/lastcopy.log
fi

touch $ctlfilenew
timestamp=`date +'%Y%m%d%H%M%S'`
handshake="$LOGDIR/handshake.$timestamp"
files=`find "$SOURCE" -name 'data*' -newer "$ctlfileold"`

start_time=`date +'%Y-%m-%d %H:%M:%S'`
echo "Start copy: $start_time" > $LOGDIR/lastcopy.log

for file in $files; do
	file=`echo $file`
	if [ "${file}z"!="z" ]; then
		fname=`basename $file`
    	echo "Local copy: $file -> $LOGDIR/$fname.$timestamp" >> $LOGDIR/lastcopy.log
		cp $file $LOGDIR/$fname.$timestamp
	fi
done

#Sleep 10 seconds to allow the file to be completly written.
sleep 10

files=`ls -1 $LOGDIR/*.$timestamp`
for file in $files; do
	file=`echo $file`
	if [ "${file}z"!="z" ]; then
		fname=`basename $file`
		fname=`echo $fname | sed s/\.$timestamp//`
	 	
#	 	echo "CMP: $file -nt $SOURCE/$fname"
	 	if [ "$file" -nt "$SOURCE/$fname" ]; then
	 		echo "Copy: $file -> $DEST" >> $LOGDIR/lastcopy.log
			if scp $file $DEST > /dev/null 2>&1; then
				cpyfile=`basename $file`
				echo "$cpyfile" >> $handshake
			fi
		else 
			echo "Changed after local copy: $SOURCE/$fname" >> $LOGDIR/lastcopy.log
	 	fi
	 	
#	 	echo "RM rm -f $file"
	 	rm -f $file
	fi
done

if [ -f "$handshake" ]; then 
	fname=`basename $handshake`
	scp $handshake $DEST/$fname > /dev/null 2>&1
	rm $handshake
fi

mv $ctlfilenew $ctlfileold

stop_time=`date +'%Y-%m-%d %H:%M:%S'`
echo "Completed copy: $stop_time" >> $LOGDIR/lastcopy.log

