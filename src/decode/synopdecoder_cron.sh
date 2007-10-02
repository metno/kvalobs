#! /bin/sh

#skript for å komprimere logfiler etter synopdecoderen.
#Sletter også filer som er for gamle!

if [ -f "$HOME/.kvalobs" ]; then
	. "$HOME/.kvalobs"
fi

DIR="$KVALOBS/var/log/decoders/SynopDecoder"

if ! cd $DIR ; then
	echo "Kan ikke skifte katalog til $DIR!"
	exit 0
fi
	
files="*.raw"

todayfile=$(date +%Y-%m-%d).raw

for file in $files ; do
	echo "file: $file"
	if [ $file != $todayfile ]; then
		echo "gzip $file"
		gzip $file
	fi
done
	
#slett logfiler som er eldre enn 10 dager
find . -name '*.raw.gz' -mtime +10 -exec rm -f {} \;
