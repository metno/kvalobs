#! /bin/sh

set -a
#set -e

ETCDIR=$(kvconfig --sysconfdir)/kvalobs
VARDIR=$(kvconfig --localstatedir)/kvalobs

if [ -f "$ETCDIR/kv-env.conf" ]; then
    . $ETCDIR/kv-env.conf
fi


NMONTH=4

#PGPORT=5434
#PGUSER=kvalobs
#PGHOST=localhost
#PGPASSWORD=`grep dbpass $KVALOBS/.kvpasswd | sed -e 's/ *dbpass *//'`

backupdir=$VARDIR/klima_backup
logdir=$VARDIR/log/klima_backup

mkdir -p $logdir
mkdir -p $backupdir

end_date=$(date -d "-$(date +%d) days" +'%Y-%m-%d')
start_date=$(date -d "$end_date" +'%Y-%m-01')

i=0
while [ $i -lt "$NMONTH" ]; do
	let i=i+1
	end_date=$(date -d "$start_date -1 days" +'%Y-%m-%d')
	start_date=$(date -d "$end_date" +'%Y-%m-01')
done

start_date=$(date -d "$start_date" +'%Y-%m-01 00:00:00')
end_date=$(date -d "$end_date" +'%Y-%m-%d 23:59:59')

backupname=data-$(date -d "$start_date" +'%Y%m')
tmpdir="$backupname"
sql="$tmpdir/save.sql"
lsql="$tmpdir/load.sql"
tarfile=$backupname.tar
logfile="$logdir/$(date -d "$start_date" +'%Y%m').log"

if ! cd $backupdir ; then
	echo "Cant change directory to: $backupdir" >> $logfile
	exit 1
fi


echo "----------------------------------------------------------" >> $logfile
echo "Start of backup: $(date +'%Y-%m-%d %H:%M:%S')" >> $logfile
echo "Backup data: $start_date - $end_date " >> $logfile

if [ -s "$tarfile.bz2" ]; then
	echo "A backup already exist!">>$logfile
	exit 0
fi

rm -rf $tmpdir

if ! mkdir -p "$tmpdir" ; then
	echo "Cant create temporary directory: $tmpdir" >> $logfile
	exit 1
fi

endname="$(date -d "$start_date" +'%Y%m').dat"
fdata="$tmpdir/data-$endname"
ftextdata="$tmpdir/text_data-$endname"
fmodeldata="$tmpdir/model_data-$endname"


#Opprett sqlfilen for aa ta data ut av databasen
echo "CREATE TEMP TABLE data_temp AS SELECT * FROM data WHERE obstime BETWEEN '$start_date' AND '$end_date';" > $sql 
echo "CREATE TEMP TABLE text_data_temp AS SELECT * FROM text_data WHERE obstime BETWEEN '$start_date' AND '$end_date';" >> $sql
echo "CREATE TEMP TABLE model_data_temp AS SELECT * FROM model_data WHERE obstime BETWEEN '$start_date' AND '$end_date';" >> $sql
echo "\\copy data_temp TO $fdata WITH DELIMITER AS '|'"  >> $sql
echo "\\copy text_data_temp TO $ftextdata WITH DELIMITER AS '|'" >> $sql
echo "\\copy model_data_temp TO $fmodeldata WITH DELIMITER AS '|'" >> $sql

#Opprett en sqlfil for aa laste data inn i databasen.
echo "DELETE FROM data WHERE obstime BETWEEN '$start_date' AND '$end_date';" > $lsql
echo "DELETE FROM text_data WHERE obstime BETWEEN '$start_date' AND '$end_date';" >> $lsql
echo "DELETE FROM model_data WHERE obstime BETWEEN '$start_date' AND '$end_date';" >> $lsql
echo "\\copy data FROM $fdata WITH DELIMITER AS '|'"  >> $lsql
echo "\\copy text_data FROM $ftextdata WITH DELIMITER AS '|'" >> $lsql
echo "\\copy model_data FROM $fmodeldata WITH DELIMITER AS '|'" >> $lsql

psql -U kvalobs kvalobs -f $sql

if [ "$?" -ne 0  ]; then
	echo "Failed to retrieve data from the database!" >> $logfile
	rm -rf $tmpdir
	exit 1
fi

echo "Data retrieved from the database!" >> $logfile
if ! tar cpf "$tarfile" "$backupname/" ; then
	echo "Cant run tar :   tar cpf $tarfile $backupname/" >> $logfile
	rm -rf $tmpdir
	exit 1
fi

bzip2 "$tarfile"

if [ "$?" -ne 0 ]; then
	echo "WARNING: Can't compress $tarfile!" >> $logfile
fi

rm -rf $tmpdir

scp "$tarfile.bz2" kvalobs@poriaz:var/klima_backup

if [ "$?" -ne 0 ]; then
	echo "WARNING: Cant copy the $tarfile.bz2 to kvalobs@poriaz:var/klima_backup" >> $logfile
fi

echo "End of backup: $(date +'%Y-%m-%d %H:%M:%S')" >> $logfile

