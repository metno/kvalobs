#! /bin/sh

#skript for å komprimere logfiler etter synopdecoderen.
#Sletter også filer som er for gamle!

DIR="$(kvconfig --localstatedir)/log/kvalobs/decoders/SynopDecoder"

if ! cd $DIR ; then
	echo "Kan ikke skifte katalog til $DIR!"
	exit 0
fi
	

#Komprimer filer eldre enn 2 dager.
find . -name '*.raw' -mtime +2 -exec gzip {} \;
	
#slett logfiler som er eldre enn 10 dager
find . -name '*.raw.gz' -mtime +10 -exec rm -f {} \;
