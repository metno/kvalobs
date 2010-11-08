#! /bin/sh

muser="kvdrift@met.no borgem@met.no nils.langgard@met.no"

set -a
#set -e

if [ ! $KVALOBS ]; then
   #KVALOBS=/disk1/kvruntime
  	KVALOBS=$HOME
fi

PGPORT=5434
PGUSER=kvalobs
PGHOST=localhost
# PGPASSWORD=`grep dbpass $KVALOBS/.kvpasswd | sed -e 's/ *dbpass *//'`

backupdir=$KVALOBS/var/klima_backup
logdir=$KVALOBS/var/log
logfile=$KVALOBS/var/log/klima_backup.log
tlogfile=$KVALOBS/var/log/last_klima_backup.log

loadlog=$KVALOBS/var/log/kl-load.log	
nowdate=$(date +"%Y-%m-01")
day=$(date +"%d")

rm -f "$tlogfile"

echo "Execution log to the script: load_last_month." >> $tlogfile
echo "The script loads backup data from kvalobs to kl-kvalobs." >> $tlogfile
echo "----------------------------------------------------------" >> $tlogfile
echo "" >> $tlogfile


debug=false

if [ $# -gt 0 ]; then
	debug=true
fi

function sendmail()
{
	$(Mail -s "Kvalobs: Kl-backup $@" $muser < $tlogfile)
}

function log() 
{
	if [ "$debug" = "true" ]; then
    	echo "$@"
   else
   	echo "$(date +"%Y-%m-%d %H:%M:%S"): $@" >> $logfile
   	echo "$(date +"%Y-%m-%d %H:%M:%S"): $@" >> $tlogfile
   fi
}

mkdir -p $logdir
mkdir -p $backupdir
mkdir -p $backupdir/backup

if ! cd $backupdir ; then
	log "Cant change directory to: $backupdir" 
	sendmail "failed!"
	exit 1
fi

if ! mkdir -p tmp ; then
	log "Cant create the directory: tmp" 
	sendmail "failed!"
	exit 1
fi


if [ -s "$loadlog" ]; then
	date=$(tail -1 $loadlog | cut -d' ' -f1 -)

	if [ "$nowdate" = "$date" ]; then
		log "Already backed up!"
		exit 0
	fi
fi 

files=$(ls -1 data-??????.tar.bz2  2>/dev/null)

if [ -z "$files" ]; then
	log "No new data file(s)."
	
	if [ $day -gt 3 ]; then
		log "No datafile has been received on kvalobs@histkvalobs from kvalobs@kvalobs!"
		log "Try to run the script save_last_month on kvalobs@kvalobs by hand,"
		log "and look out for any error message."
		log "The script is executed by cron on the 1th at kl. 6:25 each month."
		sendmail "failed!"
	fi 
	exit 0
fi

log "New file(s): $files"

if ! cd tmp ; then
	log "Cant change directory to: tmp" 
	sendmail "failed!"
	exit 1
fi

N=0

for file in $files ; do
	rm -rf *
	tar jxpf ../$file
	
	dir=$(ls -1 -d data-??????)
	
	log "datadir: $dir"
	
	if [ !  -d "$dir" ]; then
		log "No directory with expected name!" 
		continue
	fi
	
	if [ ! -f "$dir/load.sql" ]; then
		log "Missing file $load/load.sql" 
		continue
	fi
	
	let N=N+1
	
	log "Start to load the database." 
		
	if [ "$debug" = "true" ]; then
		psql --echo-all kvalobs -f $dir/load.sql 
	else
		psql --echo-all kvalobs -f $dir/load.sql >> $tlogfile 2>&1
	fi
			
	if [ "$?" -ne 0 ]; then
		log "Failed to load the database from $file."
		echo "$nowdate false $file" >> $loadlog
		sendmail "failed!"
	else
		log "Loaded the database from: $file"
		echo "$nowdate true $file" >> $loadlog
		
		rm -rf $dir
	   mv ../$file ../backup
	   sendmail "success!"
	fi
done

if [ $N -eq 0 ]; then
	log "Hmm, no valid data files!"
	sendmail "failed!"
fi