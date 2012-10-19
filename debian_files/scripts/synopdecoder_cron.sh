#! /bin/sh

#skript for å komprimere logfiler etter synopdecoderen.
#Sletter også filer som er for gamle!

DIR="$(kvconfig --localstatedir)/log/kvalobs/decoders/SynopDecoder"
LIBDIR=$(kvconfig --pkglibdir)

if [ ! -f "$LIBDIR/tool_funcs.sh" ]; then
	echo "Cant load: $LIBDIR/tool_funcs.sh"
	exit 1
fi

. $LIBDIR/tool_funcs.sh

#Exit if the machines do NOT hold the ipalias or is an test machine.
ipalias_status > /dev/null || exit 0 

if ! cd $DIR ; then
	echo "Kan ikke skifte katalog til $DIR!"
	exit 0
fi
	

#Komprimer filer eldre enn 2 dager.
find . -name '*.raw' -mtime +2 -exec gzip {} \;
	
#slett logfiler som er eldre enn 10 dager
find . -name '*.raw.gz' -mtime +10 -exec rm -f {} \;
