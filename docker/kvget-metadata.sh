#! /bin/bash

## Script to update metadata tables in Kvalobs.

set -a  # export variables to the environment of subsequent commands
set -e  # Exit if a simple shell command fails
#set -x  # for debugging, remove later

# METASRC="http://repo.met.no/data/metadata/obs/kvalobs/kvmeta.tar.bz2"
# METASRC_UTF8="http://repo.met.no/data/metadata/obs/kvalobs/kvmeta_UTF8.tar.bz2"
: ${METASRC:="http://157.249.168.104/kvalobs/kvmeta.tar.bz2"}
: ${METASRC_UTF8:="http://157.249.168.104/kvalobs/kvmeta_UTF8.tar.bz2"}

DATADIR=/var/lib
METAGET=$DATADIR/kvalobs/metaget
META_LOCAL_SRC=""
PGDATABASE=kvalobs
: ${PGUSER:=$USER}
: ${PGPORT:=5432}
: ${PGHOST:=localhost}
: ${USE_USERS_PGPASS_FILE:=true}
: ${CHECK_IS_PGREADY:=true}
: ${MAILTO:="terjeer@met.no"}
OKDB="false"

PGPORT=
PGHOST=

PSQL=psql
only_down_load=false
# force=false

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
#echo "PGPASSFILE: $PGPASSFILE"

if [ $USE_USERS_PGPASS_FILE = true ]; then
    if [ -f $HOME/.pgpass ]; then
        echo "The file $HOME/.pgpass does exist"
    else
        echo "The file $HOME/.pgpass does not exist"
        echo "The file $HOME/.pgpass does not exist" | mail -s "kvget_metadata: The file $HOME/.pgpass does not exist" $MAILTO
        exit 1
    fi

    for ll in `cat $HOME/.pgpass | grep 'kvalobs:kvalobs'| cut -f1 -d:`
    do
        echo $ll 
        PGHOST=$ll
        if cat $HOME/.pgpass | grep $ll | grep 'kvalobs:kvalobs' 1>&3
        then
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
                echo "Database connection problem : $?" | mail -s "kvget_metadata: Database connection problem " $MAILTO
                exit 1
            fi
        else
            echo "Noe galt med passordtilordning" | mail -s "kvget_metadata: Noe galt med passordtilordning" $MAILTO
            exit 1
        fi
    done
elif [ $CHECK_IS_PGREADY == true ]; then 
    if pg_isready; then
        VARn=`$PSQL --quiet --tuples-only -c 'select pg_is_in_recovery()'`
        VAR=`echo $VARn | tr -d '\n'`
        # echo -e "length(VAR)==$(echo -ne "${VAR}" | wc -m)"
        #if [ "z$VAR" != "z" ] && [ $VAR == "f" ]; then
        if [ "z$VAR" = "zf" ]; then
            # echo "selected database is $ll"
            OKDB="true"
        fi
    else 
        if [ -n $MAILTO ]; then
            echo "Database connection problem : $?" | mail -s "kvget_metadata: Database connection problem " $MAILTO
        else
            echo "Database connection problem : $?"
        fi
        exit 1
    fi
else
    OKDB=true 
fi


if [ "z$OKDB" != "ztrue" ]; then
        exit 1
        echo "No database that is not in recovery is found" | mail -s "kvget_metadata: No database that is not in recovery is found" $MAILTO
fi

echo "Database OK"

echo "host: $PGHOST:$PGPORT" 
# exit 0

CLIENT_ENCODING=`$PSQL -tc "SHOW CLIENT_ENCODING"| tr -d ' '`
SERVER_ENCODING=`$PSQL -tc "SHOW SERVER_ENCODING"| tr -d ' '`
echo "CLIENT_ENCODING=${CLIENT_ENCODING}"
echo "SERVER_ENCODING=${SERVER_ENCODING}"

if [[ ( $CLIENT_ENCODING = LATIN1  &&  $SERVER_ENCODING = LATIN1  ) ]] ; then
   export LANG=latin1
fi

#if [ "z$PGHOST" = "z" ]; then
#	PGHOST=localhost
#fi

## ** Subroutines **
assert_table_not_empty() {
	NUM_ROWS=`$PSQL -t -c "select count(*) from $1"`
	if [ $NUM_ROWS -eq 0 ]; then
		echo "ERROR: tabellen $1 er tom."
		exit 1
	fi
}


#if hash kvconfig 2>/dev/null; then
#    METAGET=$(kvconfig --datadir)/kvalobs/metaget
#fi

### METAGET=$(kvconfig --datadir)/kvalobs/metaget

mkdir -p -m700 "$METAGET"
cd $METAGET
KVMETA=kvmeta

if [ -n "$META_LOCAL_SRC" ]; then
   echo "METASRC: $META_LOCAL_SRC"
   tar xvvjf $META_LOCAL_SRC
else
   if [[ ( $CLIENT_ENCODING = LATIN1  &&  $SERVER_ENCODING = LATIN1  ) ]] ; then
       echo "METASRC: $METASRC"
       wget -O - $METASRC | tar xvvjf -
   else
       echo "METASRC: $METASRC_UTF8"
       wget -O - $METASRC_UTF8 | tar xvvjf -
   fi
fi

if [ "$only_down_load" = "true" ]; then
    echo "Metadata downloaded to '$METAGET'"
    exit 0
fi

## ** MAIN **

# if [ "$force" != true ]; then
#    read -p "Update kvalobs metadata tables (y/n): " ANS
#    if [ "$ANS" != "y" ]; then
#        echo "Terminates the metadata update!"
#        exit 0
#    fi
# fi

echo "Starting metadata update."

echo "Sletter tabellene metadatatype og station_metadata"
$PSQL -a -c "TRUNCATE metadatatype CASCADE"

echo "Oppdaterer tabellene metadatatype station_metadata";
for TABLE in metadatatype station_metadata
do
    $PSQL -c "\copy $TABLE from $METAGET/$KVMETA/$TABLE.out DELIMITER '|'"
    assert_table_not_empty $TABLE
done

echo "Sletter tabellen station der static=true";
$PSQL -a -c "delete from station where static=true"

echo "Oppdaterer tabellen station";
for TABLE in station
do
    $PSQL -c "\copy $TABLE from $METAGET/$KVMETA/$TABLE.out DELIMITER '|'"
    assert_table_not_empty $TABLE
done


echo "Oppdaterer tabellene algorithms checks station_param types param obs_pgm model qcx_info operator qc2_interpolation_best_neighbors";
for TABLE in algorithms checks station_param types param obs_pgm model qcx_info operator qc2_interpolation_best_neighbors
do
    $PSQL -a -c "truncate table $TABLE"
    $PSQL -c "\copy $TABLE from $METAGET/$KVMETA/$TABLE.out DELIMITER '|'"
    assert_table_not_empty $TABLE
done

if [ -f $DATADIR/kvalobs/utesperring/utesperring.sql ]; then
    echo "psql -f $DATADIR/kvalobs/utesperring/utesperring.sql"
    psql -f $DATADIR/kvalobs/utesperring/utesperring.sql
fi

if [ -f $HOME/insert.source ]; then
    source $HOME/insert.source
fi
