#! /bin/sh

##QABASELOGDIR=/metno/kvalobs/var/log/html
QABASELOGDIR=$(kvconfig --localstatedir)/log/kvalobs/checks

find $QABASELOGDIR -xdev -type d -ctime +7 -empty -exec rmdir --ignore-fail-on-non-empty {} \;
find $QABASELOGDIR -xdev -name '*.log.gz' -type f -mtime +7 -exec rm -f {} \;
find $QABASELOGDIR -xdev -name '*.log' -type f -mtime +0 -exec gzip {} \;

# create nice map of existing log-files (output: html)
####find $QABASELOGDIR -name '*.html' -type f | /metno/kvalobs/bin/log_map > $QABASELOGDIR/log_map.html
