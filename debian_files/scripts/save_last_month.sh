#! /bin/bash

set -a
#set -e

ETCDIR=$(kvconfig --sysconfdir)/kvalobs
LIBDIR=$(kvconfig --pkglibdir)

if [ ! -f "$LIBDIR/tool_funcs.sh" ]; then
	echo "Cant load: $LIBDIR/tool_funcs.sh"
	exit 1
fi

. $LIBDIR/tool_funcs.sh

#Exit if the machines do NOT hold the ipalias.
res=0
has_ip_alias=`ipalias_status` || res=$? 

#Only run if this machine has the ip alias for kvalobs.
[ "$has_ip_alias" = "true" ] || exit 0 


if [ -f "$ETCDIR/kv-env.conf" ]; then
    . $ETCDIR/kv-env.conf
fi

if [ -f "$ETCDIR/save_last_month.conf" ]; then
    . $ETCDIR/save_last_month.conf
fi

MAILTO="terjeer@met.no"
OKDB="false"

PSQL=psql
PGDATABASE=kvalobs
PGUSER=kvalobs
: ${PGUSER:=$USER}
: ${PGPORT:=5432}
: ${PGHOST:=localhost}

# echo "mypghost $PGHOST:$PGPORT:$PGDATABASE:$PGUSER:$PGPASSWORD"

# Create a .pgpass file to use so we do not need to give the
# password for each call to psql
# It works by creating a temporary file. Get a filehandle (3) to the file
# and delete it. Set the PGPASSFILE environment variable to /proc/PID/fd/3
# write the credentials to the file. The file is automaticly removed on exit.

rm -f "$HOME/.pgpass.kvget.*"
PGPASSFILE=$(mktemp $HOME/.pgpass.kvget.XXXXXXX)
chmod 0600 $PGPASSFILE
exec 3>$PGPASSFILE
rm $PGPASSFILE
PGPASSFILE="/proc/$$/fd/3"
# echo "PGPASSFILE: $PGPASSFILE"

if [ -f $HOME/.pgpass ]; then
    echo "The file $HOME/.pgpass does exist"
else
    echo "The file $HOME/.pgpass does not exist"
    echo "The file $HOME/.pgpass does not exist" | mail -s "kvget_utesperring: The file $HOME/.pgpass does not exist" $MAILTO
    exit 1
fi


for ll in `cat $HOME/.pgpass | grep 'kvalobs:kvalobs'| cut -f1 -d:`
do
       echo $ll	
       PGHOST=$ll
       # echo "HEI000"
       if cat $HOME/.pgpass | grep $ll | grep 'kvalobs:kvalobs' 1>&3
       then
          # echo "HEI01"
	  # PGHOST="brumle"
	  PGHOST=$ll 
	  if pg_isready
	  then
              VARn=`$PSQL --quiet --tuples-only -c 'select pg_is_in_recovery()'`
              VAR=`echo $VARn | tr -d '\n'`
              # echo -e "length(VAR)==$(echo -ne "${VAR}" | wc -m)"
              #if [ "z$VAR" != "z" ] && [ $VAR == "f" ]; then
              if [ "z$VAR" = "zf" ]; then
	          # echo "selected database is $ll"
		  OKDB="true"
	          break 
              fi
	  else
	      echo "Database connection problem : $?"
	      # echo "Database connection problem : $?" | mail -s "save_last_month: Database connection problem " $MAILTO
	      # exit 1
	  fi
       else
	   echo "Noe galt med passordtilordning" | mail -s "save_last_month: Noe galt med passordtilordning" $MAILTO
	   exit 1
       fi
	  
done


if [ "z$OKDB" != "ztrue" ]; then
	echo "No database that is not in recovery is found" | mail -s "save_last_month: No database that is not in recovery is found" $MAILTO
        exit 1
fi

echo "Database OK"

# $PSQL -c "select * from param limit 10"
# exit 0


#VARDIR=$(kvconfig --localstatedir)/lib/kvalobs
VARDIR=$(kvconfig --localstatedir)/log/kvalobs
logdir=$(kvconfig --localstatedir)/log/kvalobs/klima_backup
backupdir=$VARDIR/klima_backup

#echo $nmonth
if [ "${nmonth}z" = "z" ]; then
	NMONTH=2
else
    NMONTH=$nmonth
fi

echo $NMONTH

if [ "${histkvalobs}z" = "z" ]; then
	histkvalobs="kvalobs@histkvalobs:var/klima_backup"
else
	histkvalobs="$histkvalobs"
fi

if [ "${enable}z" = "z" ]; then
	enable="false"
fi

if [ "$enable" = "false" ]; then
	echo "Save last month disabled!"
	exit 0
fi 


#PGPORT=5434
#PGUSER=kvalobs
#PGHOST=localhost
#PGPASSWORD=`grep dbpass $KVALOBS/.kvpasswd | sed -e 's/ *dbpass *//'`

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
echo "\\copy (SELECT * FROM data WHERE obstime BETWEEN '$start_date' AND '$end_date') TO $fdata WITH DELIMITER AS '|'"  > $sql
echo "\\copy (SELECT * FROM text_data WHERE obstime BETWEEN '$start_date' AND '$end_date') TO $ftextdata WITH DELIMITER AS '|'" >> $sql
echo "\\copy (SELECT * FROM model_data WHERE obstime BETWEEN '$start_date' AND '$end_date') TO $fmodeldata WITH DELIMITER AS '|'" >> $sql

#Opprett en sqlfil for aa laste data inn i databasen.
echo "\set ON_ERROR_STOP" > $lsql
echo "BEGIN;" >> $lsql
echo "DELETE FROM data WHERE obstime BETWEEN '$start_date' AND '$end_date';" >> $lsql
echo "DELETE FROM text_data WHERE obstime BETWEEN '$start_date' AND '$end_date';" >> $lsql
echo "DELETE FROM model_data WHERE obstime BETWEEN '$start_date' AND '$end_date';" >> $lsql
echo "\\copy data FROM $fdata WITH DELIMITER AS '|'"  >> $lsql
echo "\\copy text_data FROM $ftextdata WITH DELIMITER AS '|'" >> $lsql
echo "\\copy model_data FROM $fmodeldata WITH DELIMITER AS '|'" >> $lsql
echo "END" >> $lsql

$PSQL -f $sql

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

scp "$tarfile.bz2" "$histkvalobs"

if [ "$?" -ne 0 ]; then
	echo "WARNING: Cant copy the $tarfile.bz2 to $histkvalobs" >> $logfile
fi

echo "End of backup: $(date +'%Y-%m-%d %H:%M:%S')" >> $logfile

