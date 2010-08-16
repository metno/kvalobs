#! /bin/sh

##QABASELOGDIR=/metno/kvalobs/var/log/html
QABASELOGDIR=$(kvconfig --localstatedir)/log/kvalobs/html/

#find $QABASELOGDIR -name '*.html' -type f -mtime +0 -exec rm -f {} \;
find $QABASELOGDIR -xdev -name '*.html' -type f -mtime +3 -exec rm -f {} \;
#find $QABASELOGDIR -name '*.html' -type f  -exec rm -f {} \;
find $QABASELOGDIR -xdev -type d -ctime +1 -exec rmdir --ignore-fail-on-non-empty {} \;

# create nice map of existing log-files (output: html)
####find $QABASELOGDIR -name '*.html' -type f | /metno/kvalobs/bin/log_map > $QABASELOGDIR/log_map.html
