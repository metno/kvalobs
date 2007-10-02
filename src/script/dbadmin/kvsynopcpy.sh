#! /bin/sh
                                                                               
if [ -f $HOME/.kvalobs ]; then
   source $HOME/.kvalobs
else
   echo "Missing file: $HOME/.kvalobs"
   exit 1
fi
                                                                               

SQL=sqlite
BACKUPFILE=$KVALOBS/var/kvsynop/kvsynopd.sqlite.bak
DBFILE=$KVALOBS/var/kvsynop/kvsynopd.sqlite

echo 
echo "Creating backup file!!"
echo "$BACKUPFILE"
echo

if [ -f $BACKUPFILE ]; then
    echo "Backup file allready exist."
    echo "Remove it!!!"
    exit 1
fi

cp $DBFILE $BACKUPFILE

if [ ! $? ]; then
    echo "ERROR: Cant create the backupfile!!"
    echo "FILE: <$BACKUPFILE>"
    exit 1
fi

echo
echo "Backup file created!"
echo

DAY=`date '+%d'`

echo -n "Start: "
date 
echo "-----------------------------------------------------------------------" 
echo -n "Deleting kvsynopd.sqlite .... "
rm -f $DBFILE

if [ ! $? ]; then
    echo "Failed."
    echo "ERROR: Cant delete file!!"
    echo "FILE: <$DBFILE>"
    exit 1
fi

echo "Completed!"

echo -n "Recreating kvsynopd.sqlite .... "

$KVALOBS/bin/kvsynopd_initdb > /dev/null 2>&1

if [ ! $? ]; then
    echo "Failed."
    echo "ERROR: Cant recreate kvsynopd.sqlite file!!"
    echo "FILE: <$DBFILE>"
    exit 1
fi

echo "Completed!"
echo -n "Inserting data from backupfile  .... "

cat $KVALOBS/share/kvsynop/kvsynopcpy.sql | $SQL $BACKUPFILE  | $SQL $DBFILE

if [! $? ]; then
    echo "Failed."
    echo "ERROR: inserting data from backup file failed!!"
    exit 1
fi

echo "Completed!"
echo "Removing backupfile"
rm -f $BACKUPFILE
echo "-----------------------------------------------------------------------" 
echo -n "Stop: " 
date
