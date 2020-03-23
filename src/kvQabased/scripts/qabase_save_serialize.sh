#! /bin/bash            

set -e

dir="$(kvconfig --logdir)/qabase"                   
zipdir="$dir/zip"
mkdir -p "$zipdir"
ts=$(date -u +'%FT%H')
zipf="$zipdir/ok_$ts.zip"
LOCK="${zipdir}/ZIP_LOCK"

if [ -f "$LOCK" ]; then
#   echo "Locked '$LOCK' ($zipf)"
   exit 0
fi

#echo "dir: $dir"
#echo "LOCK: $LOCK"



touch $LOCK
trap "{ rm -f $LOCK; }" EXIT



#Add log files to zip archive and delete the files.
find $dir -name '*.ok' -mmin +2 -execdir zip -q -m -j $zipf {} \;
#find /var/log/kvalobs/qabase/ -name '*.ok' -mmin +2 -execdir ls -l {} \;

#Keep 7 days with logs.
find $zipdir  -name 'ok_*zip' -mtime +7 -exec rm -f {} \;


